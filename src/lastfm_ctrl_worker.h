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

#ifndef LASTFM_CTRL_WORKER_H
#define LASTFM_CTRL_WORKER_H

#include <QObject>

#include "track_metadata.h"

class Lastfm;
class LastfmDb;

class LastfmCtrlWorker : public QObject
{
    Q_OBJECT

public:
    LastfmCtrlWorker(QObject *parent=nullptr);

private:
    bool enabled;
    bool online;

    Lastfm *lfm;
    LastfmDb *db;

    TrackMetadata current_track_md;

public slots:
    void setup(QString login, QString password);

    void setEnabled(bool state);
    void setOnline(bool state);

    void nowPlaying(TrackMetadata md);
    void playtimeChanged(qint64 playtime);

private slots:
    void checkCache();

signals:
    void badauth();
    void cacheSize(qint64);
};

#endif // LASTFM_CTRL_WORKER_H
