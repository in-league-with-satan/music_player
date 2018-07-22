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
#include <QDateTime>

#include "lastfm.h"
#include "lastfm_db.h"

#include "lastfm_ctrl_worker.h"

LastfmCtrlWorker::LastfmCtrlWorker(QObject *parent)
    : QObject(parent)
    , enabled(true)
    , online(true)
{
    lfm=new Lastfm(this);

    db=new LastfmDb(this);
}

void LastfmCtrlWorker::setup(QString login, QString password)
{
    lfm->setup(login, password, "test_player");

    checkCache();
}

void LastfmCtrlWorker::checkCache()
{
    if(!online)
        return;

    TrackMetadataList lst;

    db->getTracks(&lst);

    if(lst.isEmpty())
        return;

    if(lfm->submit(lst)) {
        QList <qint64> ids;

        foreach(TrackMetadata md, lst)
            ids << md.start_playing_time;

        db->removeTrack(ids);

        emit cacheSize(db->cacheSize());

    } else if(lfm->lastError()==Lastfm::ERROR_BADAUTH) {
        emit badauth();
    }
}

void LastfmCtrlWorker::setEnabled(bool state)
{
    enabled=state;

    emit cacheSize(db->cacheSize());
}

void LastfmCtrlWorker::setOnline(bool state)
{
    // qInfo() << "LastfmCtrlWorker::setOnline" << state;
    online=state;
}

void LastfmCtrlWorker::nowPlaying(TrackMetadata md)
{
    if(!enabled)
        return;

    if(current_track_md.filepath==md.filepath)
        return;

    if(md.album.isEmpty() || md.artist.isEmpty() || md.title.isEmpty() || md.track_length<2) {
        current_track_md=TrackMetadata();

    } else {
        current_track_md=md;
        current_track_md.start_playing_time=QDateTime::currentSecsSinceEpoch();

        checkCache();

        if(online) {
            lfm->nowPlaying(md);

            if(lfm->lastError()==Lastfm::ERROR_BADAUTH) {
                emit badauth();
            }
        }
    }
}

void LastfmCtrlWorker::playtimeChanged(qint64 playtime)
{
    if(!enabled)
        return;

    if(current_track_md.start_playing_time<=0) {
        return;
    }

    if(playtime>=240 || playtime>=current_track_md.track_length*.5) { // the track must have been played for a duration of at least 240 seconds or half the track's total length
        db->addTrack(current_track_md);

        emit cacheSize(db->cacheSize());

        current_track_md=TrackMetadata();
    }
}
