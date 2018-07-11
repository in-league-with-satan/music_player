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

#ifndef PROGRESS_H
#define PROGRESS_H

#include <QSlider>

class Progress : public QWidget
{
    Q_OBJECT

public:
    Progress(bool slider_mode=false, QWidget *parent=0);
    ~Progress();

    int64_t value() const;

public slots:
    void setValue(int value);
    void setRange(int min, int max);

private slots:
    void sliderPressed();
    void sliderReleased();
    void valueChanged(int value);

private:
    QSlider *slider;

    bool pressed;
    int drag_value;
    bool slider_mode;

signals:
    void posChanged(qint64 value);
    void posChanged32(int value);
};

#endif // PROGRESS_H
