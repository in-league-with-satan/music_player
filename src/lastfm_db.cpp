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
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <qcoreapplication.h>

#include "lastfm_db.h"

LastfmDb::LastfmDb(QObject *parent)
    : QObject(parent)
{
    init();
}

void LastfmDb::init()
{
    db_name=QString::number(qintptr(this));

    QSqlDatabase db=QSqlDatabase::addDatabase("QSQLITE", db_name);

    db.setDatabaseName(qApp->applicationDirPath() + "/music_player.sqlite");

    if(!db.open()) {
        qCritical() << "LastfmDb::init:" << db.lastError().text();
        return;
    }


    QSqlQuery query=QSqlQuery(db);

    //

    if(!query.exec(QString("CREATE TABLE IF NOT EXISTS lastfm_cache("
                           "start_playing_time INTEGER PRIMARY KEY AUTOINCREMENT,"
                           "title TEXT,"
                           "artist TEXT,"
                           "album TEXT,"
                           "track_number TEXT,"
                           "track_length INTEGER)"))) {
        qCritical() << query.lastError().text() << query.lastQuery();
    }
}

bool LastfmDb::addTrack(const TrackMetadata &tmd)
{
    if(!QSqlDatabase::database(db_name).isOpen()) {
        qCritical() << "db is not opened";
        init();
        return false;
    }

    QSqlQuery query=QSqlQuery(QSqlDatabase::database(db_name));

    query.prepare(QStringLiteral("INSERT INTO lastfm_cache VALUES(?, ?, ?, ?, ?, ?)"));
    query.addBindValue(tmd.start_playing_time);
    query.addBindValue(tmd.title);
    query.addBindValue(tmd.artist);
    query.addBindValue(tmd.album);
    query.addBindValue(tmd.track_number);
    query.addBindValue(tmd.track_length);

    if(!query.exec()) {
        qCritical() << "LastfmDb::addTrack:" << query.lastError().text();
        return false;
    }

    return true;
}

bool LastfmDb::getTracks(TrackMetadataList *list)
{
    list->clear();

    if(!QSqlDatabase::database(db_name).isOpen()) {
        qCritical() << "db is not opened";
        return false;
    }

    QSqlQuery query=QSqlQuery(QSqlDatabase::database(db_name));

    query.prepare(QStringLiteral("SELECT * FROM lastfm_cache LIMIT 50"));

    if(!query.exec()) {
        qCritical() << "LastfmDb::getTracks:" << query.lastError().text();
        return false;
    }

    TrackMetadata tmd;

    while(query.next()) {
        tmd.start_playing_time=query.value(QStringLiteral("start_playing_time")).toLongLong();
        tmd.title=query.value(QStringLiteral("title")).toString();
        tmd.artist=query.value(QStringLiteral("artist")).toString();
        tmd.album=query.value(QStringLiteral("album")).toString();
        tmd.track_number=query.value(QStringLiteral("track_number")).toString();
        tmd.track_length=query.value(QStringLiteral("track_length")).toLongLong();

        list->append(tmd);
    }

    return true;
}

bool LastfmDb::removeTrack(QList <qint64> start_playing_time)
{
    if(start_playing_time.isEmpty())
        return true;

    if(!QSqlDatabase::database(db_name).isOpen()) {
        qCritical() << "db is not opened";
        return false;
    }

    QSqlQuery query=QSqlQuery(QSqlDatabase::database(db_name));

    QString ids;

    foreach(const qint64 &id, start_playing_time) {
        ids+=QString("%1,").arg(id);
    }

    ids.resize(ids.size() - 1);

    if(!query.exec(QString("DELETE FROM lastfm_cache WHERE start_playing_time IN(%1)").arg(ids))) {
        qCritical() << "LastfmDb::removeTrack:" << query.lastError().text();
        return false;
    }

    return true;
}

qint64 LastfmDb::cacheSize()
{
    if(!QSqlDatabase::database(db_name).isOpen()) {
        qCritical() << "db is not opened";
        return 0;
    }

    QSqlQuery query=QSqlQuery(QSqlDatabase::database(db_name));

    query.exec("SELECT COUNT(*) FROM lastfm_cache");

    if(!query.next())
        return 0;

    return query.value(0).toLongLong();
}
