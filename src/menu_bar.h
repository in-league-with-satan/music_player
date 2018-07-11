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

#ifndef MENU_BAR_H
#define MENU_BAR_H

#include <QMenuBar>

class MenuBar : public QMenuBar
{
    Q_OBJECT

public:
    MenuBar(QWidget *parent=0);
    ~MenuBar();

private slots:
    void showLibraryChanged(bool state);

signals:
    void showSettings();
    void showLibrary(bool state);
};

#endif // MENU_BAR_H
