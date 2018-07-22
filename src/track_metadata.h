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

#ifndef TRACK_METADATA_H
#define TRACK_METADATA_H

#include <QString>
#include <QList>

struct TrackMetadata
{
    QString title;
    QString artist;
    QString album;
    QString track_number;
    QString date;
    QString filepath;
    qint64 track_length=-1;
    qint64 start_playing_time=-1;

    inline QVariant toVariant() {
        return QVariant::fromValue(*this);
    }

    inline TrackMetadata fromVariant(const QVariant &var) {
        return (*this)=var.value<TrackMetadata>();
    }
};

typedef QList <TrackMetadata> TrackMetadataList;

Q_DECLARE_METATYPE(TrackMetadata);
Q_DECLARE_METATYPE(TrackMetadataList);

#endif // TRACK_METADATA_H
