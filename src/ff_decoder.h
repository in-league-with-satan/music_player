/******************************************************************************

Copyright © 2018-2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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

#ifndef FF_DECODER_H
#define FF_DECODER_H

#include <QObject>

class FFDecoderContext;

struct FFDecStats {
    QString format;
    int64_t bitrate;
    int64_t pos;
    int64_t duration;
    int64_t samplerate;
    int64_t last_update_time=0;
};


class FFDecoder : public QObject
{
    Q_OBJECT

public:
    FFDecoder(QObject *parent=0);
    ~FFDecoder();

    bool open(const QString &filename);

    void close();

    void skipSilence(bool enabled);

    int64_t pos() const;
    int64_t duration() const;
    int64_t playtime() const;

    QByteArray read();

    void seek(int64_t pos);

private:
    FFDecoderContext *context;
    FFDecStats stats;

signals:
    void streamEnded();
    void openError();
    void openOk();
    void statsChanged(FFDecStats stats);
};

#endif // FF_DECODER_H
