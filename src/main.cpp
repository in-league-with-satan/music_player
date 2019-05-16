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

#include <QApplication>

#include "ff_tools.h"
#include "net_ctrl.h"
#include "settings.h"
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    avLogToQDebug();
    avLogSetEnabled(false);


    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);

    QApplication application(argc, argv);

    net_ctrl->createInstance();

    if(!net_ctrl->listen()) {
        if(application.arguments().contains(QStringLiteral("--play-pause")))
            net_ctrl->send(NetCtrl::KeyCode::PlayPause);

        else if(application.arguments().contains(QStringLiteral("--pause")))
            net_ctrl->send(NetCtrl::KeyCode::PlayPause);

        else if(application.arguments().contains(QStringLiteral("--next")))
            net_ctrl->send(NetCtrl::KeyCode::Next);

        else if(application.arguments().contains(QStringLiteral("--prev")))
            net_ctrl->send(NetCtrl::KeyCode::Prev);

        exit(0);
    }

    settings->createInstance();

    if(!settings->load())
        settings->save();

    // a.setStyle("cleanlooks");

    MainWindow w;
    w.show();
    w.restoreWindow();

    if(application.arguments().contains(QStringLiteral("--play-pause")) || application.arguments().contains(QStringLiteral("--pause")))
        w.playPause();

    return application.exec();
}
