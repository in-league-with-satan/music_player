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

#ifndef NET_CTRL_H
#define NET_CTRL_H

#include <QObject>

class QUdpSocket;

#define net_ctrl NetCtrl::instance()

class NetCtrl : public QObject
{
    Q_OBJECT

public:
    static NetCtrl *createInstance(QObject *parent=0);
    static NetCtrl *instance();

    bool listen() const;

    struct KeyCode {
        enum T {
            PlayPause,
            Next,
            Prev
        };
    };

    void send(KeyCode::T code);

private slots:
    void read();

private:
    NetCtrl(QObject *parent=0);

    static NetCtrl *_instance;

    QUdpSocket *socket;

    bool bind_state;

signals:
    void playPause();
    void pause();
    void next();
    void prev();
};

#endif // NET_CTRL_H
