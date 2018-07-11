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
#include <QFile>
#include <QApplication>

#include "ff_decoder.h"

#include "audio_io_device.h"

AudioIODevice::AudioIODevice(QObject *parent)
    : QIODevice(parent)
    , decoder(nullptr)
{
    QIODevice::open(QIODevice::ReadWrite);
}

bool AudioIODevice::open(QIODevice::OpenMode mode)
{
    Q_UNUSED(mode)

    // qWarning() << "AudioIODevice::open";

    return true;
}

void AudioIODevice::close()
{
    qWarning() << "AudioIODevice::close";
}

qint64 AudioIODevice::pos() const
{
    return 0;
}

qint64 AudioIODevice::size() const
{
    return 1;
}

bool AudioIODevice::seek(qint64 pos)
{
    Q_UNUSED(pos)

    return false;
}

bool AudioIODevice::atEnd() const
{
    return false;
}

bool AudioIODevice::reset()
{
    qWarning() << "AudioIODevice::reset";

    return true;
}

bool AudioIODevice::isSequential() const
{
    return true;
}

qint64 AudioIODevice::bytesAvailable() const
{
    return 1;
    return 0;
}

void AudioIODevice::clear()
{
    ba_data.clear();
}

bool AudioIODevice::canReadLine() const
{
    qWarning() << "AudioIODevice::canReadLine";

    return false;
}

void AudioIODevice::setDecoder(FFDecoder *decoder)
{
    this->decoder=decoder;
}

qint64 AudioIODevice::readData(char *data, qint64 maxlen)
{
    if(!decoder)
        return 0;

    QByteArray ba_readed;

    while(true) {
        if(ba_data.size()>=maxlen)
            break;

        ba_readed=decoder->read();

        if(ba_readed.isEmpty())
            break;

        ba_data+=ba_readed;
    }

    if(maxlen>ba_data.size())
        maxlen=ba_data.size();

    if(maxlen>0) {
        memcpy(data, ba_data.constData(), maxlen);

        ba_data.remove(0, maxlen);
    }

    qint64 pos=decoder->pos();

    if(last_audio_pos!=pos) {
        last_audio_pos=pos;

        emit posChanged(pos);
    }

    return maxlen;
}

qint64 AudioIODevice::writeData(const char *data, qint64 len)
{
    return 0;
}

