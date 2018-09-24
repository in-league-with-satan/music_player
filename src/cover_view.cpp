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
#include <QLayout>
#include <QLabel>
#include <QResizeEvent>
#include <QFileInfo>
#include <QDir>

#include "cover_view.h"

CoverView::CoverView(QWidget *parent)
    : QWidget(parent)
{
    label=new QLabel();
    label->setScaledContents(true);
    label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(label);
    la_main->setMargin(0);

    setLayout(la_main);

    hide();
}

QSize CoverView::sizeHint() const
{
    return QSize(width(), width());
}

void CoverView::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);

    updateGeometry();
}

void CoverView::setPath(const QString &path)
{
    QFileInfo fi(path);

    if(current_path==fi.dir().path())
        return;

    current_path=fi.dir().path();

    if(!visible) {
        if(isVisible())
            hide();

        return;
    }

    QDir dir=fi.dir();

    QStringList images=dir.entryList(QStringList() << "*.jpg" << "*.jpeg" << "*.png", QDir::Files);

    if(images.isEmpty()) {
        label->clear();

        hide();

        return;
    }

    const QPixmap pixmap(current_path + "/" + images.first());

    if(pixmap.isNull()) {
        hide();

    } else {
        label->setPixmap(pixmap);

        show();
    }
}

void CoverView::setVisible2(bool value)
{
    visible=value;

    if(visible) {
        const QString current_path_tmp=current_path + "/";

        current_path.clear();

        setPath(current_path_tmp);

    } else {
        label->clear();

        hide();
    }
}
