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

#ifndef LASTFM_CTRL_H
#define LASTFM_CTRL_H

#include <QThread>

#include "track_metadata.h"

class LastfmCtrl : public QThread
{
    Q_OBJECT

public:
    LastfmCtrl(QObject *parent=nullptr);

protected:
    void run();

signals:
    void setEnabled(bool);
    void setOnline(bool);
    void setup(QString, QString);
    void nowPlaying(TrackMetadata);
    void playtimeChanged(qint64);
    void cacheSize(qint64);

    //

    void badauth();
};

#endif // LASTFM_CTRL_H
