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

#ifndef LASTFM_H
#define LASTFM_H

#include <QObject>
#include <QList>

#include "track_metadata.h"

class QNetworkAccessManager;

class Lastfm : public QObject
{
    Q_OBJECT

public:
    Lastfm(QObject *parent=nullptr);
    ~Lastfm();

    void setup(const QString &login, const QString &password, const QString &client_id, const QString &client_version="1.0");

    bool nowPlaying(const TrackMetadata &md);

    bool submit(const TrackMetadataList &list);

    int lastError() const;

    enum {
        ERROR_OK,
        ERROR_BANNED,
        ERROR_BADAUTH,
        ERROR_BADTIME,
        ERROR_BADSESSION,
        ERROR_FAILED,
        ERROR_OTHER
    };

private:
    bool auth();
    void readError(QString body);

    QNetworkAccessManager *network_access_manager;

    QString login;
    QString password;
    QString client_id;
    QString client_version;

    QString session_id;
    QString url_now_playing;
    QString url_submission;

    int last_error;
};

#endif // LASTFM_H
