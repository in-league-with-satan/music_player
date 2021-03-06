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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#include "settings_view.h"
#include "ff_decoder.h"

class FilelistView;
class PlaylistView;
class AudioOutputThread;
class Progress;
class LastfmCtrl;
class CoverView;
class EqualizerView;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent=0);
    ~MainWindow();

    enum {
        STATE_IDLE,
        STATE_PLAY,
        STATE_PAUSED
    };

    void restoreWindow();

public slots:
    void playPause();
    void stop();

    void applySettings();

private slots:
    void dlgOpenPlaylist();
    void dlgSavePlaylist();
    void loadPlaylist();
    void savePlaylist();
    void saveSettingsCommon();
    void onDurationChanged(qint64 duration);
    void onPosChanged(qint64 pos);
    void onPlayRequest(QString);
    void updateStatus(FFDecStats stats);
    void setupEqualizer();

protected:
    virtual void dragEnterEvent(QDragEnterEvent *event);
    virtual void dropEvent(QDropEvent *event);

private:
    void initHotkeys();

    QMainWindow *lists_widgets;
    FilelistView *filelist_view;
    PlaylistView *playlist_view;

    CoverView *cover_view;

    AudioOutputThread *audio_output;

    LastfmCtrl *lastfm;

    Progress *volume_level;
    Progress *progress;

    EqualizerView *equalizer_view;

    SettingsView *settings_view;
};

#endif // MAINWINDOW_H
