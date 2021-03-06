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
#include <QLayout>
#include <QDockWidget>
#include <QPushButton>
#include <QStatusBar>
#include <QDir>
#include <QFile>
#include <QTime>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>
#include <QFileDialog>
#include <QJsonDocument>
#include <qcoreapplication.h>

#ifdef LIB_HOTKEY
#  include "qhotkey.h"
#endif

#include "filelist_view.h"
#include "playlist_view.h"
#include "ff_tools.h"
#include "audio_output_thread.h"
#include "progress.h"
#include "menu_bar.h"
#include "net_ctrl.h"
#include "settings.h"
#include "settings_view.h"
#include "lastfm_ctrl.h"
#include "cover_view.h"
#include "img_src.h"
#include "equalizer_view.h"

#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    qRegisterMetaType<FFDecStats>("FFDecStats");
    qRegisterMetaType<TrackMetadata>("TrackMetadata");
    qRegisterMetaType<EQParams>("EQParams");

    setAcceptDrops(true);

    connect(settings, SIGNAL(updated()), SLOT(applySettings()));

    equalizer_view=new EqualizerView();

    lastfm=new LastfmCtrl();

    MenuBar *menu_bar=new MenuBar();

    connect(menu_bar, SIGNAL(lastfmEnabled(bool)), lastfm, SIGNAL(setEnabled(bool)));
    connect(menu_bar, SIGNAL(lastfmOnline(bool)), lastfm, SIGNAL(setOnline(bool)));
    connect(lastfm, SIGNAL(cacheSize(qint64)), menu_bar, SLOT(setLastfmCacheSize(qint64)));
    connect(lastfm, SIGNAL(badauth()), menu_bar, SLOT(lastfmBadauth()));
    connect(menu_bar, SIGNAL(openPlaylist()), SLOT(dlgOpenPlaylist()));
    connect(menu_bar, SIGNAL(savePlaylist()), SLOT(dlgSavePlaylist()));
    connect(menu_bar, SIGNAL(showEqualizer()), SLOT(setupEqualizer()));

    setMenuBar(menu_bar);

    settings_view=new SettingsView();

    connect(menu_bar, SIGNAL(showSettings()), settings_view, SLOT(exec()));
    connect(lastfm, SIGNAL(badauth()), settings_view, SLOT(lastfmBadauth()));

    audio_output=new AudioOutputThread(this);
    connect(audio_output, SIGNAL(durationChanged(qint64)), SLOT(onDurationChanged(qint64)));
    connect(audio_output, SIGNAL(posChanged(qint64)), SLOT(onPosChanged(qint64)));
    connect(audio_output, SIGNAL(statsChanged(FFDecStats)), SLOT(updateStatus(FFDecStats)));
    connect(audio_output, SIGNAL(playtimeChanged(qint64)), lastfm, SIGNAL(playtimeChanged(qint64)));

    filelist_view=new FilelistView();

    playlist_view=new PlaylistView();
    connect(playlist_view, SIGNAL(playRequest(QString)), audio_output, SIGNAL(setFile(QString)));
    connect(playlist_view, SIGNAL(playRequest(QString)), SLOT(onPlayRequest(QString)));
    connect(playlist_view, SIGNAL(playRequest(QString)), SLOT(saveSettingsCommon()));
    connect(playlist_view, SIGNAL(listChanged()), SLOT(savePlaylist()));
    connect(playlist_view, SIGNAL(currentIndexRemoved()), SLOT(stop()));
    connect(playlist_view, SIGNAL(nowPlaying(TrackMetadata)), lastfm, SIGNAL(nowPlaying(TrackMetadata)));

    connect(menu_bar, SIGNAL(cursorFollowsPlayback(bool)), playlist_view, SLOT(setCursorFollowsPlayback(bool)));

    connect(audio_output, SIGNAL(streamEnded()), playlist_view, SLOT(next()));
    connect(audio_output, SIGNAL(openError()), playlist_view, SLOT(markError()));
    connect(audio_output, SIGNAL(openOk()), playlist_view, SLOT(markPlay()));
    connect(audio_output, SIGNAL(startPlaying()), playlist_view, SLOT(markPlay()));
    connect(audio_output, SIGNAL(paused()), playlist_view, SLOT(markPause()));
    connect(audio_output, SIGNAL(stopped()), playlist_view, SLOT(markVoid()));

    connect(filelist_view, SIGNAL(updatePlaylist(QStringList,bool)), playlist_view, SLOT(updatePlaylist(QStringList,bool)));

    connect(net_ctrl, SIGNAL(playPause()), SLOT(playPause()));
    connect(net_ctrl, SIGNAL(next()), playlist_view, SLOT(next()));
    connect(net_ctrl, SIGNAL(prev()), playlist_view, SLOT(prev()));

    cover_view=new CoverView();
    cover_view->setVisible2(settings->main.show_cover);

    connect(playlist_view, SIGNAL(playRequest(QString)), cover_view, SLOT(setPath(QString)));
    connect(menu_bar, SIGNAL(showCover(bool)), cover_view, SLOT(setVisible2(bool)));

    const QSize button_size(42, 24);
    const QSize icon_size=button_size*.7;

    QFont font=settings->main.font_ctrl;

    QPushButton *b_play=new QPushButton();
    b_play->setIcon(img->get(ImgSrc::PmCode::play_pause));
    b_play->setFont(font);
    b_play->setMinimumSize(button_size);
    b_play->setIconSize(icon_size);
    connect(b_play, SIGNAL(clicked(bool)), SLOT(playPause()));

    QPushButton *b_stop=new QPushButton();
    b_stop->setIcon(img->get(ImgSrc::PmCode::stop, true));
    b_stop->setFont(font);
    b_stop->setMinimumSize(button_size);
    b_stop->setIconSize(icon_size);
    connect(b_stop, SIGNAL(clicked(bool)), SLOT(stop()));

    QPushButton *b_prev=new QPushButton();
    b_prev->setIcon(img->get(ImgSrc::PmCode::prev));
    b_prev->setFont(font);
    b_prev->setMinimumSize(button_size);
    b_prev->setIconSize(icon_size);
    connect(b_prev, SIGNAL(clicked(bool)), playlist_view, SLOT(prev()));

    QPushButton *b_next=new QPushButton();
    b_next->setIcon(img->get(ImgSrc::PmCode::next));
    b_next->setFont(font);
    b_next->setMinimumSize(button_size);
    b_next->setIconSize(icon_size);
    connect(b_next, SIGNAL(clicked(bool)), playlist_view, SLOT(next()));

    volume_level=new Progress(true);
    volume_level->setRange(0, 1000);
    volume_level->setValue(settings->main.volume_level);
    connect(volume_level, SIGNAL(posChanged32(int)), audio_output, SIGNAL(setVolume(int)));

    audio_output->setVolume(settings->main.volume_level);

    progress=new Progress();
    connect(progress, SIGNAL(posChanged(qint64)), audio_output, SIGNAL(seek(qint64)));


    QGridLayout *la_filelist=new QGridLayout();
    la_filelist->addWidget(filelist_view, 0, 0);
    la_filelist->addWidget(cover_view, 1, 0);
    la_filelist->setMargin(0);

    QWidget *w_filelist=new QWidget();
    w_filelist->setLayout(la_filelist);

    setStatusBar(new QStatusBar());


    lists_widgets=new QMainWindow();

    QDockWidget *dock_filelist=new QDockWidget();
    dock_filelist->setObjectName("filelist");
    dock_filelist->setWidget(w_filelist);
    dock_filelist->setTitleBarWidget(new QWidget());
    dock_filelist->setFloating(true);

    connect(menu_bar, SIGNAL(showLibrary(bool)), dock_filelist, SLOT(setVisible(bool)));


    QDockWidget *dock_playlist=new QDockWidget();
    dock_playlist->setObjectName("playlist");
    dock_playlist->setWidget(playlist_view);
    dock_playlist->setTitleBarWidget(new QWidget());
    dock_playlist->setFloating(true);

    lists_widgets->addDockWidget(Qt::LeftDockWidgetArea, dock_filelist, Qt::Horizontal);
    lists_widgets->addDockWidget(Qt::LeftDockWidgetArea, dock_playlist, Qt::Horizontal);

    QHBoxLayout *la_controls=new QHBoxLayout();
    la_controls->addWidget(b_play);
    la_controls->addWidget(b_stop);
    la_controls->addWidget(b_prev);
    la_controls->addWidget(b_next);
    la_controls->addSpacing(5);
    la_controls->addWidget(volume_level, 1);
    la_controls->addSpacing(5);
    la_controls->addWidget(progress, 5);
    la_controls->setContentsMargins(5, 5, 5, 0);

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addLayout(la_controls);
    la_main->addWidget(lists_widgets);
    la_main->setMargin(1);

    QWidget *w_central=new QWidget();
    w_central->setLayout(la_main);

    setCentralWidget(w_central);

    if(settings->main.playlist.isEmpty())
        settings->main.playlist=qApp->applicationDirPath() + "/playlist.json";

    loadPlaylist();

    applySettings();

    initHotkeys();
}

MainWindow::~MainWindow()
{
    settings->main.geometry=saveGeometry();
    settings->main.state_window=lists_widgets->saveState();
    settings->main.state_table=playlist_view->saveState();
    settings->equalizer=equalizer_view->getPresets();

    saveSettingsCommon();

    savePlaylist();
}

void MainWindow::restoreWindow()
{
    if(qApp->arguments().contains(QStringLiteral("--frameless"))) {
        setWindowFlags(Qt::Widget | Qt::FramelessWindowHint);

        show();
        showMaximized();

        lists_widgets->restoreState(settings->main.state_window);
        playlist_view->restoreState(settings->main.state_table);

    } else if(settings->main.geometry.isEmpty()) {
        showMaximized();

    } else {
        restoreGeometry(settings->main.geometry);

        lists_widgets->restoreState(settings->main.state_window);
        playlist_view->restoreState(settings->main.state_table);
    }
}

void MainWindow::playPause()
{
    if(audio_output->isActive()) {
        audio_output->pause();

    } else {
        if(audio_output->isOpen()) {
            audio_output->play();

        } else {
            if(playlist_view->currentFile().isEmpty())
                return;

            cover_view->setPath(playlist_view->currentFile());

            audio_output->setFile(playlist_view->currentFile());
            audio_output->play();
        }
    }
}

void MainWindow::stop()
{
    audio_output->stop();
    statusBar()->showMessage("stopped");
    progress->setRange(0, 0);
}

void MainWindow::applySettings()
{
    filelist_view->setRootPath(settings->main.library_path);

    QStringList filter;

    foreach(QString str, settings->main.file_filter.split(";", QString::SkipEmptyParts)) {
        filter << QString("*.%1").arg(str.simplified());
    }

    filelist_view->setNameFilters(filter);

    filelist_view->filterEmptyDirs(settings->main.filter_empty_dirs);

    playlist_view->setCursorFollowsPlayback(settings->main.cursor_follows_playback);

    //

    audio_output->skipSilence(settings->main.skip_silence);

    //

    lastfm->setEnabled(settings->lastfm.enabled);
    lastfm->setOnline(settings->lastfm.online);
    lastfm->setup(settings->lastfm.login, settings->lastfm.password);

    //

    audio_output->setDevice(settings->output_device.use_default, settings->output_device.dev_name);

    //

    equalizer_view->setPresets(settings->equalizer);

    audio_output->setupEq(equalizer_view->params(false));
}

void MainWindow::dlgOpenPlaylist()
{
    QString dir=settings->main.playlist;

    if(dir.isEmpty())
        dir=qApp->applicationDirPath() + "/playlist.json";

    const QString filename=QFileDialog::getOpenFileName(this, "select playlist file", dir, "*.json");

    if(filename.isEmpty())
        return;

    if(QString::compare(filename, Settings::filename(), Qt::CaseInsensitive)==0)
        return;

    stop();

    settings->main.playlist=filename;
    settings->main.playlist_index=0;

    settings->save();

    loadPlaylist();
}

void MainWindow::dlgSavePlaylist()
{
    QString dir=settings->main.playlist;

    if(dir.isEmpty())
        dir=qApp->applicationDirPath() + "/playlist.json";

    const QString filename=QFileDialog::getSaveFileName(this, "select playlist file", dir, "*.json");

    if(filename.isEmpty())
        return;

    if(QString::compare(filename, Settings::filename(), Qt::CaseInsensitive)==0)
        return;

    settings->main.playlist=filename;

    settings->save();

    savePlaylist();
}

void MainWindow::loadPlaylist()
{
    QFile file(settings->main.playlist);

    if(!file.open(QFile::ReadOnly))
        return;

    playlist_view->restorePlaylist(QJsonDocument::fromJson(file.readAll()).toVariant().toList());
    playlist_view->setIndex(settings->main.playlist_index);

    file.close();
}

void MainWindow::savePlaylist()
{
    QFile file(settings->main.playlist);

    if(!file.open(QFile::WriteOnly | QFile::Truncate))
        return;

    file.write(QJsonDocument::fromVariant(playlist_view->savePlaylist()).toJson());

    file.close();
}

void MainWindow::saveSettingsCommon()
{
    settings->main.playlist_index=playlist_view->currentIndex();
    settings->main.volume_level=volume_level->value();

    settings->save();
}

void MainWindow::onDurationChanged(qint64 duration)
{
    progress->setRange(0, duration);
}

void MainWindow::onPosChanged(qint64 pos)
{
    progress->setValue(pos);
}

void MainWindow::onPlayRequest(QString)
{
    if(!audio_output->isActive())
        audio_output->play();
}

void MainWindow::updateStatus(FFDecStats stats)
{
    statusBar()->showMessage(QString("%1 | %2 kbps | %3 hz | %4/%5")
                             .arg(stats.format)
                             .arg(int(stats.bitrate*.001))
                             .arg(stats.samplerate)
                             .arg(timeToStringMSec(stats.pos))
                             .arg(timeToStringMSec(stats.duration))
                             );
}

void MainWindow::setupEqualizer()
{
    if(equalizer_view->exec()==QDialog::Accepted) {
        audio_output->setupEq(equalizer_view->params(false));
    }
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
        event->acceptProposedAction();
}

void MainWindow::dropEvent(QDropEvent *event)
{
    QStringList list;

    foreach(const QUrl &url, event->mimeData()->urls()) {
        list.append(url.toLocalFile());
    }

    filelist_view->tryAddUrls(list);
}

void MainWindow::initHotkeys()
{
#ifdef LIB_HOTKEY

    QHotkey *hotkey;

    hotkey=new QHotkey(Qt::Key_MediaPlay, Qt::NoModifier, true, this);
    connect(hotkey, &QHotkey::activated, [this](){ playPause(); });

    hotkey=new QHotkey(Qt::Key_MediaPause, Qt::NoModifier, true, this);
    connect(hotkey, &QHotkey::activated, [this](){ playPause(); });

    hotkey=new QHotkey(Qt::Key_MediaPrevious, Qt::NoModifier, true, this);
    connect(hotkey, &QHotkey::activated, [this](){ playlist_view->prev(); });

    hotkey=new QHotkey(Qt::Key_MediaNext, Qt::NoModifier, true, this);
    connect(hotkey, &QHotkey::activated, [this](){ playlist_view->next(); });

#endif
}

