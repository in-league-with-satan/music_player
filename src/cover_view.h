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

#ifndef COVER_VIEW_H
#define COVER_VIEW_H

#include <QWidget>

class QLabel;

class CoverView : public QWidget
{
    Q_OBJECT

public:
    explicit CoverView(QWidget *parent=nullptr);

    virtual QSize sizeHint() const;

    void resizeEvent(QResizeEvent *event);

public slots:
    void setPath(const QString &path);

    void setVisible2(bool value);

private:
    QLabel *label;

    QString current_path;

    QPixmap current_pixmap;

    bool visible=true;
};

#endif // COVER_VIEW_H
