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
#include <QToolTip>
#include <QCursor>
#include <QTime>
#include <QEvent>
#include <QMouseEvent>
#include <QStyle>
#include <QProxyStyle>

#include "ff_tools.h"

#include "progress.h"

Progress::Progress(bool slider_mode, QWidget *parent)
    : QWidget(parent)
    , pressed(false)
    , slider_mode(slider_mode)
{
    class SliderStyle: public QProxyStyle {
    public:
        virtual int styleHint(StyleHint hint, const QStyleOption *option=0, const QWidget *widget=0, QStyleHintReturn *return_data=0) const {
            if(hint==QStyle::SH_Slider_AbsoluteSetButtons) {
                return Qt::LeftButton;

            } else {
                return QProxyStyle::styleHint(hint, option, widget, return_data);
            }
        }
    };


    slider=new QSlider(Qt::Horizontal);
    slider->setStyle(new SliderStyle());

    connect(slider, SIGNAL(sliderPressed()), SLOT(sliderPressed()));
    connect(slider, SIGNAL(sliderReleased()), SLOT(sliderReleased()));
    connect(slider, SIGNAL(sliderMoved(int)), SLOT(valueChanged(int)));

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(slider);
    la_main->setSpacing(0);
    la_main->setMargin(0);

    setLayout(la_main);

    setRange(0, 0);
}

Progress::~Progress()
{
}

int64_t Progress::value() const
{
    if(slider_mode)
        return slider->value();

    return slider->value()*1000;
}

void Progress::setValue(int value)
{
    if(pressed)
        return;

    if(slider_mode)
        slider->setValue(value);

    else
        slider->setValue(value*.001);
}

void Progress::setRange(int min, int max)
{
    if(slider_mode)
        slider->setRange(min, max);

    else
        slider->setRange(min*.001, max*.001);

    slider->setEnabled(min!=max);
}

void Progress::sliderPressed()
{
    pressed=true;

    drag_value=slider->value();

    if(slider_mode)
        return;

    QToolTip::showText(QPoint(QCursor::pos().x(), slider->mapToGlobal(slider->pos()).y() ), timeToStringSec(slider->value()));
}

void Progress::sliderReleased()
{
    pressed=false;

    if(slider_mode)
        emit posChanged32(drag_value);

    else
        emit posChanged(drag_value*1000);
}

void Progress::valueChanged(int value)
{
    if(!pressed)
        return;

    drag_value=value;

    if(slider_mode) {
        emit posChanged32(drag_value);
        return;
    }

    QToolTip::hideText();
    QToolTip::showText(QPoint(QCursor::pos().x(), slider->mapToGlobal(slider->pos()).y()), timeToStringSec(value));
}

