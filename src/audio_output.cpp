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
#include <QAudioOutput>
#include <qcoreapplication.h>


#include "audio_output.h"

AudioOutput::AudioOutput(QObject *parent)
    : QObject(parent)
    , audio_output(nullptr)
    , open_state(false)
{
    decoder=new FFDecoder();

    connect(decoder, SIGNAL(streamEnded()), SIGNAL(streamEnded()));
    connect(decoder, SIGNAL(statsChanged(FFDecStats)), SIGNAL(statsChanged(FFDecStats)));
    connect(decoder, SIGNAL(openError()), SIGNAL(openError()));
    connect(decoder, SIGNAL(openOk()), SIGNAL(openOk()));

    dev_audio_output.open(AudioIODevice::ReadWrite);
    dev_audio_output.setDecoder(decoder);

    connect(&dev_audio_output, SIGNAL(posChanged(qint64)), SIGNAL(posChanged(qint64)));
    connect(&dev_audio_output, SIGNAL(playtimeChanged(qint64)), SIGNAL(playtimeChanged(qint64)));

    audio_format.setSampleRate(48000);
    audio_format.setChannelCount(2);
    audio_format.setSampleSize(16);
    audio_format.setCodec("audio/pcm");
    audio_format.setByteOrder(QAudioFormat::LittleEndian);
    audio_format.setSampleType(QAudioFormat::SignedInt);


    init();
}

AudioOutput::~AudioOutput()
{
}

void AudioOutput::setFile(const QString &filename)
{
    open_state=decoder->open(filename);

    emit durationChanged(decoder->duration());
}

void AudioOutput::play()
{
    init();

    audio_output->start(&dev_audio_output);
    emit startPlaying();
}

void AudioOutput::stop()
{
    audio_output->stop();
    audio_output->reset();
    decoder->close();
    dev_audio_output.clear();
    open_state=false;

    emit stopped();
}

void AudioOutput::pause()
{
    audio_output->stop();

    emit paused();
}

void AudioOutput::seek(qint64 pos)
{
    decoder->seek(pos);
}

void AudioOutput::setVolume(int volume)
{
    audio_output->setVolume(volume*.001);
}

void AudioOutput::setDevice(bool use_default, QString dev_name)
{
    dev_param.use_default=use_default;
    dev_param.name=dev_name;

    init();
}

bool AudioOutput::isActive() const
{
    return audio_output->state()==QAudio::ActiveState;
}

bool AudioOutput::isOpen() const
{
    return open_state;
}

void AudioOutput::init()
{
    QAudioDeviceInfo di_tmp;

    if(dev_param.use_default) {
        di_tmp=QAudioDeviceInfo::defaultOutputDevice();

    } else {
        foreach(const QAudioDeviceInfo &di, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
            if(di.deviceName()==dev_param.name) {
                di_tmp=di;
                break;
            }
        }

        if(di_tmp.isNull())
            di_tmp=QAudioDeviceInfo::defaultOutputDevice();
    }

    if(dev_param.di!=di_tmp) {
        dev_param.di=di_tmp;

        if(audio_output) {
            audio_output->stop();
            audio_output->deleteLater();
        }

        audio_output=new QAudioOutput(dev_param.di, audio_format);

        //

        if(open_state) {
            audio_output->start(&dev_audio_output);
            emit startPlaying();
        }
    }
}
