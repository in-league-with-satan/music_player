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
#include <QSvgRenderer>
#include <QGuiApplication>
#include <QScreen>
#include <QPainter>
#include <QPixmap>
#include <QImage>

#include "img_src.h"

ImgSrc *ImgSrc::_instance=nullptr;

ImgSrc *ImgSrc::instance()
{
    if(_instance==nullptr)
        _instance=new ImgSrc();

    return _instance;
}

//

ImgSrc::ImgSrc(QObject *parent)
    : QObject(parent)
{
    const QSize size_scr=QGuiApplication::primaryScreen()->geometry().size();

    target_side_size=(size_scr.width() + size_scr.height())*.05;
}

QIcon ImgSrc::get(int code, bool wide)
{
    if(code<0 || code>=PmCode::size)
        return QIcon();

    if(!wide && cache.contains(code))
        return cache[code];

    QString res_name;

    switch(code) {
    case PmCode::next:
        res_name=":/images/next.svg";
        break;

    case PmCode::pause:
        res_name=":/images/pause.svg";
        break;

    case PmCode::play_pause:
        res_name=":/images/play-pause.svg";
        break;

    case PmCode::play:
        res_name=":/images/play.svg";
        break;

    case PmCode::prev:
        res_name=":/images/prev.svg";
        break;

    case PmCode::stop:
        res_name=":/images/stop.svg";
        break;
    }

    QSvgRenderer svg_render(res_name);

    if(!svg_render.isValid())
        return QIcon();

    svg_render.defaultSize();

    const int width=target_side_size;
    const int height=target_side_size/(svg_render.defaultSize().width()/svg_render.defaultSize().height());

    QImage image;

    if(wide && width==height) {
        image=QImage(width*2, height, QImage::Format_ARGB32);
        image.fill(qRgba(0, 0, 0, 0));

        QPainter painter(&image);
        svg_render.render(&painter, QRectF(width*.5, 0, width, height));
        painter.end();

    } else {
        image=QImage(width, height, QImage::Format_ARGB32);
        image.fill(qRgba(0, 0, 0, 0));

        QPainter painter(&image);
        svg_render.render(&painter);
        painter.end();

        cache[code]=QIcon(QPixmap::fromImage(image));

        return cache[code];
    }

    return QIcon(QPixmap::fromImage(image));
}
