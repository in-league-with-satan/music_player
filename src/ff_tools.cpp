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
#include <QTime>

#include "ff_tools.h"

bool av_log_enabled=true;

void av_log_callback(void*, int level, const char *fmt, va_list vl)
{
    if(!av_log_enabled)
        return;

    // if(level>AV_LOG_ERROR)
    //     return;

    // if(level>AV_LOG_WARNING)
    //     return;

    if(level>AV_LOG_INFO)
        return;

    if(!fmt)
        return;

    qInfo().noquote() << "av:" << QString().vsprintf(fmt, vl).remove("\n");
}

void avLogToQDebug()
{
    av_log_set_callback(av_log_callback);
}

void avLogSetEnabled(bool state)
{
    av_log_enabled=state;
}

QString ffErrorString(int code)
{
    char buf[AV_ERROR_MAX_STRING_SIZE];

    av_strerror(code, buf, AV_ERROR_MAX_STRING_SIZE);

    return QString(buf);
}

QString timeToStringSec(int64_t t)
{
    QTime time=QTime(0, 0).addSecs(t);

    if(time.hour()>0)
        return time.toString("hh:mm:ss");

    return time.toString("mm:ss");
}

QString timeToStringMSec(int64_t t)
{
    return timeToStringSec(t*.001);
}

TagsReader::Tags TagsReader::get(const QString &filename)
{
    TagsReader::Tags tags;


    AVFormatContext *format_context=avformat_alloc_context();

    if(avformat_open_input(&format_context, filename.toUtf8().data(), 0, 0)<0) {
        qWarning() << "TagsReader::get: avformat_open_input err";
        return tags;
    }

    int ret=avformat_find_stream_info(format_context, nullptr);

    if(ret<0) {
        qWarning() << "TagsReader::get: avformat_find_stream_info err" << ffErrorString(ret);
    }


    static auto readTags=[](AVDictionary *metadata, TagsReader::Tags *tags) {
        AVDictionaryEntry *tag=nullptr;

        while((tag=av_dict_get(metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
            QString key=QString::fromUtf8(tag->key);

            if(QString::compare(key, "artist", Qt::CaseInsensitive)==0)
                tags->artist=QString(tag->value);

            if(QString::compare(key, "album", Qt::CaseInsensitive)==0)
                tags->album=QString(tag->value);

            if(QString::compare(key, "title", Qt::CaseInsensitive)==0)
                tags->title=QString(tag->value);

            if(QString::compare(key, "track", Qt::CaseInsensitive)==0)
                tags->tracknumber=QString(tag->value);

            if(QString::compare(key, "date", Qt::CaseInsensitive)==0)
                tags->date=QString(tag->value);
        }
    };

    static auto readDuration=[](int64_t duration, AVRational time_base)->QString {
        if(duration!=AV_NOPTS_VALUE)
            return timeToStringSec(duration*av_q2d(time_base));

        return QString();
    };


    readTags(format_context->metadata, &tags);


    AVDictionary *metadata=nullptr;

    for(unsigned int i=0; i<format_context->nb_streams; ++i) {
        if(format_context->streams[i]->codecpar->codec_type==AVMEDIA_TYPE_AUDIO) {
            metadata=format_context->streams[i]->metadata;

            if(format_context->streams[i]->duration!=AV_NOPTS_VALUE) {
                tags.duration=readDuration(format_context->streams[i]->duration, format_context->streams[i]->time_base);

            } else if(format_context->duration!=AV_NOPTS_VALUE) {
                tags.duration=readDuration(format_context->duration, AV_TIME_BASE_Q);

            } else {
                tags.duration="???";

                qWarning() << "TagsReader::get: wrong stream duration";
            }

            break;
        }
    }


    if(metadata)
        readTags(metadata, &tags);


    avformat_close_input(&format_context);

    avformat_free_context(format_context);


    return tags;
}
