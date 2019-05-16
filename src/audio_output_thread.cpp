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

#include <QDebug>
#include <qcoreapplication.h>

#include "audio_output.h"

#include "audio_output_thread.h"

AudioOutputThread::AudioOutputThread(QObject *parent)
    : QThread(parent)
{
    thread_running=false;

    start(QThread::NormalPriority);

    while(!thread_running) {
        msleep(10);
    }
}

AudioOutputThread::~AudioOutputThread()
{
    quit();

    while(thread_running) {
        msleep(10);
    }
}

bool AudioOutputThread::isActive() const
{
    return state_active;
}

bool AudioOutputThread::isOpen() const
{
    return state_open;
}

void AudioOutputThread::run()
{
    state_active=false;
    state_open=false;

    AudioOutput *audio_output=new AudioOutput();

    audio_output->moveToThread(this);

    connect(this, SIGNAL(setFile(QString)), audio_output, SLOT(setFile(QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(play()),           audio_output, SLOT(play()), Qt::QueuedConnection);
    connect(this, SIGNAL(stop()),           audio_output, SLOT(stop()), Qt::QueuedConnection);
    connect(this, SIGNAL(pause()),          audio_output, SLOT(pause()), Qt::QueuedConnection);
    connect(this, SIGNAL(seek(qint64)),     audio_output, SLOT(seek(qint64)), Qt::QueuedConnection);
    connect(this, SIGNAL(setVolume(int)),   audio_output, SLOT(setVolume(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(setDevice(bool, QString)),
                                            audio_output, SLOT(setDevice(bool, QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(skipSilence(bool)),audio_output, SLOT(skipSilence(bool)), Qt::QueuedConnection);

    connect(audio_output, SIGNAL(posChanged(qint64)),       SIGNAL(posChanged(qint64)), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(durationChanged(qint64)),  SIGNAL(durationChanged(qint64)), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(playtimeChanged(qint64)),  SIGNAL(playtimeChanged(qint64)), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(statsChanged(FFDecStats)), SIGNAL(statsChanged(FFDecStats)), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(streamEnded()),            SIGNAL(streamEnded()), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(openError()),              SIGNAL(openError()), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(openOk()),                 SIGNAL(openOk()), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(startPlaying()),           SIGNAL(startPlaying()), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(paused()),                 SIGNAL(paused()), Qt::QueuedConnection);
    connect(audio_output, SIGNAL(stopped()),                SIGNAL(stopped()), Qt::QueuedConnection);

    connect(audio_output, &AudioOutput::openError,      [this](){ state_open=false; state_active=false; });
    connect(audio_output, &AudioOutput::openOk,         [this](){ state_open=true; });
    connect(audio_output, &AudioOutput::startPlaying,   [this](){ state_active=true; });
    connect(audio_output, &AudioOutput::paused,         [this](){ state_active=false; });
    connect(audio_output, &AudioOutput::stopped,        [this](){ state_active=false; state_open=false; });

    thread_running=true;

    exec();

    thread_running=false;
}

