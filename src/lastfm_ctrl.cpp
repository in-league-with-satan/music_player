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

#include "lastfm_ctrl_worker.h"

#include "lastfm_ctrl.h"

LastfmCtrl::LastfmCtrl(QObject *parent)
    : QThread(parent)
{
    start(QThread::NormalPriority);
}

void LastfmCtrl::run()
{
    LastfmCtrlWorker *w=new LastfmCtrlWorker();

    w->moveToThread(this);

    connect(this, SIGNAL(setEnabled(bool)), w, SLOT(setEnabled(bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(setOnline(bool)), w, SLOT(setOnline(bool)), Qt::QueuedConnection);
    connect(this, SIGNAL(setup(QString,QString)), w, SLOT(setup(QString,QString)), Qt::QueuedConnection);
    connect(this, SIGNAL(nowPlaying(TrackMetadata)), w, SLOT(nowPlaying(TrackMetadata)), Qt::QueuedConnection);
    connect(this, SIGNAL(playtimeChanged(qint64)), w, SLOT(playtimeChanged(qint64)), Qt::QueuedConnection);
    connect(w, SIGNAL(cacheSize(qint64)), SIGNAL(cacheSize(qint64)), Qt::QueuedConnection);
    connect(w, SIGNAL(badauth()), SIGNAL(badauth()), Qt::QueuedConnection);

    exec();
}
