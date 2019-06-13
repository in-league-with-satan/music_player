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

#ifndef MENU_BAR_H
#define MENU_BAR_H

#include <QMenuBar>

class QAction;

class MenuBar : public QMenuBar
{
    Q_OBJECT

public:
    MenuBar(QWidget *parent=0);
    ~MenuBar();

private:
    QAction *a_lastfm_online;
    QAction *a_lastfm_cache_size;

public slots:
    void lastfmBadauth();
    void setLastfmCacheSize(qint64 size);

private slots:
    void showLibraryChanged(bool state);
    void showCoverChanged(bool state);

    void cursorFollowsPlaybackChanged(bool state);

    void lastfmEnabledChanged(bool state);
    void lastfmOnlineChanged(bool state);

signals:
    void showSettings();
    void showEqualizer();
    void showLibrary(bool state);
    void showCover(bool state);
    void cursorFollowsPlayback(bool state);

    void lastfmEnabled(bool state);
    void lastfmOnline(bool state);
};

#endif // MENU_BAR_H
