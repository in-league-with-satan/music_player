/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.

******************************************************************************/

#include <QDebug>

#include "settings.h"

#include "menu_bar.h"

MenuBar::MenuBar(QWidget *parent)
    : QMenuBar(parent)
{
    QAction *action;

    QMenu *menu_file=new QMenu("file");

    action=menu_file->addAction("open playlist");
    connect(action, SIGNAL(triggered(bool)), SIGNAL(openPlaylist()));

    action=menu_file->addAction("save playlist");
    connect(action, SIGNAL(triggered(bool)), SIGNAL(savePlaylist()));

    QMenu *menu_edit=new QMenu("edit");

    action=menu_edit->addAction("settings");
    action->setShortcut(QKeySequence::fromString("ctrl+p"));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(showSettings()));

    action=menu_edit->addAction("equalizer");
    action->setShortcut(QKeySequence::fromString("ctrl+e"));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(showEqualizer()));

    QMenu *menu_view=new QMenu("view");

    action=menu_view->addAction("show library");
    action->setShortcut(QKeySequence::fromString("ctrl+l"));
    action->setCheckable(true);
    action->setChecked(settings->main.show_library);
    connect(action, SIGNAL(triggered(bool)), SLOT(showLibraryChanged(bool)));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(showLibrary(bool)));

    action=menu_view->addAction("show cover");
    action->setShortcut(QKeySequence::fromString("ctrl+c"));
    action->setCheckable(true);
    action->setChecked(settings->main.show_cover);
    connect(action, SIGNAL(triggered(bool)), SLOT(showCoverChanged(bool)));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(showCover(bool)));

    action=menu_view->addAction("cursor follows playback");
    // action->setShortcut(QKeySequence::fromString("ctrl+f"));
    action->setCheckable(true);
    action->setChecked(settings->main.cursor_follows_playback);
    connect(action, SIGNAL(triggered(bool)), SLOT(cursorFollowsPlaybackChanged(bool)));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(cursorFollowsPlayback(bool)));

    QMenu *menu_lastfm=new QMenu("last.fm");

    action=menu_lastfm->addAction("enabled");
    action->setCheckable(true);
    action->setChecked(settings->lastfm.enabled);
    connect(action, SIGNAL(triggered(bool)), SLOT(lastfmEnabledChanged(bool)));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(lastfmEnabled(bool)));

    a_lastfm_online=menu_lastfm->addAction("online");
    a_lastfm_online->setCheckable(true);
    a_lastfm_online->setChecked(settings->lastfm.online);
    connect(a_lastfm_online, SIGNAL(triggered(bool)), SLOT(lastfmOnlineChanged(bool)));

    a_lastfm_cache_size=menu_lastfm->addAction("cache size: 0");
    a_lastfm_cache_size->setEnabled(false);

    addMenu(menu_file);
    addMenu(menu_edit);
    addMenu(menu_view);
    addMenu(menu_lastfm);
}

MenuBar::~MenuBar()
{
}

void MenuBar::lastfmBadauth()
{
    a_lastfm_online->setChecked(settings->lastfm.online=false);
    emit lastfmOnline(false);
}

void MenuBar::setLastfmCacheSize(qint64 size)
{
    a_lastfm_cache_size->setText(QString("cache size: %1").arg(size));
}

void MenuBar::showLibraryChanged(bool state)
{
    settings->main.show_library=state;
}

void MenuBar::showCoverChanged(bool state)
{
    settings->main.show_cover=state;
}

void MenuBar::cursorFollowsPlaybackChanged(bool state)
{
    settings->main.cursor_follows_playback=state;
}

void MenuBar::lastfmEnabledChanged(bool state)
{
    settings->lastfm.enabled=state;
}

void MenuBar::lastfmOnlineChanged(bool state)
{
    if(state && (settings->lastfm.login.isEmpty() || settings->lastfm.password.isEmpty())) {
        a_lastfm_online->setChecked(settings->lastfm.online=false);
        emit lastfmOnline(false);
        return;
    }

    settings->lastfm.online=state;

    emit lastfmOnline(state);
}
