/******************************************************************************

Copyright © 2018 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
    QMenu *menu_edit=new QMenu("edit");

    QAction *action=menu_edit->addAction("settings");
    action->setShortcut(QKeySequence::fromString("ctrl+p"));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(showSettings()));

    QMenu *menu_view=new QMenu("view");

    action=menu_view->addAction("show library");
    action->setShortcut(QKeySequence::fromString("ctrl+l"));
    action->setCheckable(true);
    action->setChecked(settings->main.show_library);
    connect(action, SIGNAL(triggered(bool)), SLOT(showLibraryChanged(bool)));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(showLibrary(bool)));

    action=menu_view->addAction("cursor follows playback");
    // action->setShortcut(QKeySequence::fromString("ctrl+f"));
    action->setCheckable(true);
    action->setChecked(settings->main.cursor_follows_playback);
    connect(action, SIGNAL(triggered(bool)), SLOT(cursorFollowsPlaybackChanged(bool)));
    connect(action, SIGNAL(triggered(bool)), SIGNAL(cursorFollowsPlayback(bool)));


    addMenu(menu_edit);
    addMenu(menu_view);
}

MenuBar::~MenuBar()
{
}

void MenuBar::showLibraryChanged(bool state)
{
    settings->main.show_library=state;
}

void MenuBar::cursorFollowsPlaybackChanged(bool state)
{
    settings->main.cursor_follows_playback=state;
}
