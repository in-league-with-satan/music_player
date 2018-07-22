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
#include <QNetworkAccessManager>
#include <QNetworkProxy>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDateTime>
#include <QCryptographicHash>
#include <QEventLoop>

#include "lastfm.h"

const QString scrobbler_url="http://post.audioscrobbler.com";

Lastfm::Lastfm(QObject *parent)
    : QObject(parent)
    , last_error(ERROR_OK)
{
    network_access_manager=new QNetworkAccessManager();
}

Lastfm::~Lastfm()
{
}

void Lastfm::setup(const QString &login, const QString &password, const QString &client_id, const QString &client_version)
{
    if(this->login!=login
            || this->password!=password
            || this->client_id!=client_id
            || this->client_version!=client_version) {
        this->login=login;
        this->password=password;
        this->client_id=client_id;
        this->client_version=client_version;

        session_id.clear();
    }
}

bool Lastfm::nowPlaying(const TrackMetadata &md)
{
    if(session_id.isEmpty()) {
        if(!auth())
            return false;
    }

    QUrlQuery url_query;

    url_query.addQueryItem("s", session_id);
    url_query.addQueryItem(QStringLiteral("a"), md.artist);
    url_query.addQueryItem(QStringLiteral("t"), md.title);
    url_query.addQueryItem(QStringLiteral("l"), QString::number(md.track_length));
    url_query.addQueryItem(QStringLiteral("b"), md.album);
    url_query.addQueryItem(QStringLiteral("n"), md.track_number);
    url_query.addQueryItem(QStringLiteral("m"), "");


    QNetworkRequest request(url_now_playing);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply=network_access_manager->post(request, url_query.toString(QUrl::FullyEncoded).toUtf8());


    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    const QString body=QString::fromUtf8(reply->readAll());
    const QVariant status_code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);


    reply->deleteLater();


    if(body.contains(QStringLiteral("OK"))) {
        last_error=ERROR_OK;
        return true;
    }

    readError(body);

    return false;
}

bool Lastfm::submit(const TrackMetadataList &list)
{
    if(session_id.isEmpty()) {
        if(!auth())
            return false;
    }

    if(list.size()>50)
        return false;

    QUrlQuery url_query;

    url_query.addQueryItem("s", session_id);

    for(int i=0; i<list.size(); ++i) {
        url_query.addQueryItem(QString("a[%1]").arg(i), list[i].artist);
        url_query.addQueryItem(QString("t[%1]").arg(i), list[i].title);
        url_query.addQueryItem(QString("i[%1]").arg(i), QString::number(list[i].start_playing_time));
        url_query.addQueryItem(QString("o[%1]").arg(i), "P");
        url_query.addQueryItem(QString("r[%1]").arg(i), "");
        url_query.addQueryItem(QString("l[%1]").arg(i), QString::number(list[i].track_length));
        url_query.addQueryItem(QString("b[%1]").arg(i), list[i].album);
        url_query.addQueryItem(QString("n[%1]").arg(i), list[i].track_number);
        url_query.addQueryItem(QString("m[%1]").arg(i), "");
    }

    QNetworkRequest request(url_submission);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");

    QNetworkReply *reply=network_access_manager->post(request, url_query.toString(QUrl::FullyEncoded).toUtf8());


    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    const QString body=QString::fromUtf8(reply->readAll());
    const QVariant status_code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);


    reply->deleteLater();


    if(body.contains(QStringLiteral("OK"))) {
        last_error=ERROR_OK;
        return true;
    }

    readError(body);

    return false;
}

int Lastfm::lastError() const
{
    return last_error;
}

bool Lastfm::auth()
{
    if(login.isEmpty() || password.isEmpty() || client_id.isEmpty()) {
        last_error=ERROR_BADAUTH;
        return false;
    }

    int64_t timestamp=QDateTime::currentSecsSinceEpoch();

    QCryptographicHash md5(QCryptographicHash::Md5);

    md5.addData(password.toLatin1());

    const QByteArray ba_md5_pass=md5.result();

    md5.reset();

    md5.addData(QString("%1%2").arg(QString(ba_md5_pass.toHex())).arg(timestamp).toLatin1());

    const QByteArray ba_md5_pass_timestamp=md5.result();

    // token=md5(md5(password) + timestamp)
    const QString token=QString(ba_md5_pass_timestamp.toHex());

    QUrlQuery url_query;
    url_query.addQueryItem("hs", "true");
    url_query.addQueryItem("p", "1.2.1");
    url_query.addQueryItem("c", client_id);
    url_query.addQueryItem("v", client_version);
    url_query.addQueryItem("u", login);
    url_query.addQueryItem("t", QString::number(timestamp));
    url_query.addQueryItem("a", token);

    QUrl url(scrobbler_url.endsWith("/") ? scrobbler_url : scrobbler_url + "/");
    url.setQuery(url_query);


    QNetworkReply *reply=network_access_manager->get(QNetworkRequest(url));


    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();


    const QString body=QString::fromUtf8(reply->readAll());
    const QVariant status_code=reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);


    reply->deleteLater();


    // if(status_code.toInt()==200) {
    if(body.contains(QStringLiteral("OK"))) {
        last_error=ERROR_OK;

        QStringList sl=body.split("\n", QString::SkipEmptyParts);

        if(sl.size()<4) {
            qCritical() << "last.fm result 200, but not enough data" << body.simplified();
            readError(body);
            return false;
        }

        session_id=sl[1];
        url_now_playing=sl[2];
        url_submission=sl[3];

        return true;

    } else {
        readError(body);
    }

    return false;
}

void Lastfm::readError(QString body)
{
    qCritical() << "last.fm error. response:" << body.simplified();

    last_error=ERROR_OTHER;

    if(body.contains(QStringLiteral("BANNED"))) {
        last_error=ERROR_BANNED;
        login.clear();
        password.clear();

    } else if(body.contains(QStringLiteral("BADAUTH"))) {
        last_error=ERROR_BADAUTH;
        login.clear();
        password.clear();

    } else if(body.contains(QStringLiteral("BADTIME"))) {
        last_error=ERROR_BADTIME;

    } else if(body.contains(QStringLiteral("BADSESSION"))) {
        last_error=ERROR_BADSESSION;
        session_id.clear();

    } else if(body.contains(QStringLiteral("FAILED"))) {
        last_error=ERROR_FAILED;
    }
}

