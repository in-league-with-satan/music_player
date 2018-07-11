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

#ifndef FF_TOOLS_H
#define FF_TOOLS_H

#include <QString>

extern "C" {
#include <libavformat/avformat.h>
#include <libavutil/opt.h>
#include <libavutil/time.h>
#include <libswresample/swresample.h>
}

void avLogToQDebug();
void avLogSetEnabled(bool state);

QString ffErrorString(int code);

QString timeToStringSec(int64_t t);
QString timeToStringMSec(int64_t t);

class TagsReader
{
public:
    struct Tags {
        QString artist;
        QString album;
        QString title;
        QString tracknumber;
        QString date;
        QString duration;
    };

    static Tags get(const QString &filename);
};

#endif // FF_TOOLS_H
