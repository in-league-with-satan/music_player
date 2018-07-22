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

#ifndef LASTFM_DB_H
#define LASTFM_DB_H

#include <QObject>

#include "track_metadata.h"

class LastfmDb : public QObject
{
    Q_OBJECT

public:
    LastfmDb(QObject *parent=nullptr);

    bool addTrack(const TrackMetadata &tmd);
    bool getTracks(TrackMetadataList *list);
    bool removeTrack(QList <qint64> start_playing_time);

    qint64 cacheSize();

private:
    void init();

    QString db_name;
};

#endif // LASTFM_DB_H
