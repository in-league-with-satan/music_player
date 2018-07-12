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
#include <QUdpSocket>
#include <QNetworkDatagram>

#include "net_ctrl.h"

const int port=2432;

NetCtrl *NetCtrl::_instance=nullptr;

NetCtrl *NetCtrl::createInstance(QObject *parent)
{
    if(_instance==nullptr)
        _instance=new NetCtrl(parent);

    return _instance;
}

NetCtrl *NetCtrl::instance()
{
    return _instance;
}

NetCtrl::NetCtrl(QObject *parent) :
    QObject(parent)
{
    socket=new QUdpSocket();

    bind_state=socket->bind(QHostAddress::LocalHost, port);

    if(bind_state) {
        connect(socket, SIGNAL(readyRead()), SLOT(read()));
    }
}

bool NetCtrl::listen() const
{
    return bind_state;
}

void NetCtrl::send(NetCtrl::KeyCode::T code)
{
    socket->writeDatagram(QByteArray(1, (char)code), QHostAddress::LocalHost, port);
    socket->flush();
}

void NetCtrl::read()
{
    QByteArray ba;

    while(socket->hasPendingDatagrams()) {
        ba=socket->receiveDatagram().data();

        if(ba.size()!=1)
            continue;

        switch(ba[0]) {
        case KeyCode::T::PlayPause:
            emit playPause();
            break;

        case KeyCode::T::Next:
            emit next();
            break;

        case KeyCode::T::Prev:
            emit prev();
            break;

        default:
            break;
        }
    }
}

