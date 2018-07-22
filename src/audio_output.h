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

#ifndef AUDIO_OUTPUT_H
#define AUDIO_OUTPUT_H

#include <QAudioFormat>

#include "audio_io_device.h"

#include "ff_decoder.h"

class QAudioOutput;

class AudioOutput : public QObject
{
    Q_OBJECT

public:
    AudioOutput(QObject *parent=0);
    ~AudioOutput();

    bool isActive() const;
    bool isOpen() const;

public slots:
    void setFile(const QString &filename);
    void play();
    void stop();
    void pause();

    void seek(qint64 pos);

    void setVolume(int volume); // max level=1000

private:
    QAudioOutput *audio_output;

    AudioIODevice dev_audio_output;

    QAudioFormat audio_format;

    FFDecoder *decoder;

    bool open_state;

signals:
    void posChanged(qint64 );
    void durationChanged(qint64);
    void playtimeChanged(qint64);
    void statsChanged(FFDecStats);

    void streamEnded();
    void openError();
    void openOk();
    void startPlaying();
    void paused();
    void stopped();
};

#endif // AUDIO_OUTPUT_H
