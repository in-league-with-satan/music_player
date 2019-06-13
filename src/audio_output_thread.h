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

#ifndef AUDIO_OUTPUT_THREAD_H
#define AUDIO_OUTPUT_THREAD_H

#include <QThread>

#include <atomic>

#include "ff_decoder.h"

class AudioOutputThread : public QThread
{
    Q_OBJECT

public:
    AudioOutputThread(QObject *parent=0);
    ~AudioOutputThread();

    bool isActive() const;
    bool isOpen() const;

protected:
    void run();

private:
    std::atomic <bool> state_active;
    std::atomic <bool> state_open;
    std::atomic <bool> thread_running;

signals:
    void posChanged(qint64 pos);
    void durationChanged(qint64);
    void playtimeChanged(qint64);
    void statsChanged(FFDecStats);

    void streamEnded();
    void openError();
    void openOk();
    void startPlaying();
    void paused();
    void stopped();

    // slots:
    void setFile(const QString &filename);
    void play();
    void stop();
    void pause();

    void seek(qint64 pos);

    void setVolume(int volume);

    void setDevice(bool use_default, QString dev_name);

    void skipSilence(bool enabled);

    void setupEq(EQParams params);
};

#endif // AUDIO_OUTPUT_THREAD_H
