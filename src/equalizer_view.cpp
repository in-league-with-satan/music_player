/******************************************************************************

Copyright © 2019 Andrey Cheprasov <ae.cheprasov@gmail.com>

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
#include <QInputDialog>
#include <QLabel>
#include <QSlider>
#include <QPushButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLineEdit>

#include "equalizer_view.h"

EqualizerView::EqualizerView(QWidget *parent)
    : QDialog(parent)
    , freq(QVector<int>() << 20 << 25 << 32 << 40 << 50 << 63 << 80 << 125 << 160
                          << 200 << 250 << 315 << 400 << 500 << 630 << 800 << 1000 << 1250 << 1600
                          << 2500 << 3150 << 4000 << 5000 << 6300 << 8000 << 10000 << 12500 << 16000
                          << 20000)
{
    setMinimumSize(1024, 360);

    setWindowTitle("graphic equalizer");

    //

    cb_preset=new QComboBox();
    connect(cb_preset, SIGNAL(currentIndexChanged(int)), SLOT(onPresetIndexChanged(int)));

    QPushButton *b_preset_add=new QPushButton("add");
    connect(b_preset_add, SIGNAL(clicked(bool)), SLOT(presetAdd()));

    QPushButton *b_preset_rename=new QPushButton("rename");
    connect(b_preset_rename, SIGNAL(clicked(bool)), SLOT(presetRename()));

    QPushButton *b_preset_remove=new QPushButton("remove");
    connect(b_preset_remove, SIGNAL(clicked(bool)), SLOT(presetRemove()));


    QHBoxLayout *la_preset=new QHBoxLayout();
    la_preset->addWidget(new QLabel("preset:"));
    la_preset->addWidget(cb_preset, 10);
    la_preset->addWidget(b_preset_add);
    la_preset->addWidget(b_preset_rename);
    la_preset->addWidget(b_preset_remove);

    //

    const QVector <int> gain=QVector<int>() << 12 << 8 << 4 << 0 << -4 << -8 << -12;

    QVBoxLayout *la_head_gain=new QVBoxLayout();

    la_head_gain->setMargin(0);

    for(int value:gain)
        la_head_gain->addWidget(new QLabel(QString::number(value)), 0, Qt::AlignRight);

    QVBoxLayout *la_head_gain_2=new QVBoxLayout();

    la_head_gain_2->addLayout(la_head_gain, 10);
    la_head_gain_2->addWidget(new QLabel());

    QHBoxLayout *la_band=new QHBoxLayout();
    la_band->addLayout(la_head_gain_2, 0);

    QSlider *sl;
    QLabel *label;

    for(int freq:freq) {
        QVBoxLayout *la_item=new QVBoxLayout();

        sl=new QSlider();
        sl->setRange(-120, 120);

        connect(sl, SIGNAL(valueChanged(int)), SLOT(calcVol()));

        label=new QLabel(freq<1000 ? QString::number(freq) : QString("%1k").arg(freq*.001));
        label->setAlignment(Qt::AlignCenter);

        la_item->addWidget(sl, 0, Qt::AlignHCenter);
        la_item->addWidget(label);

        la_band->addLayout(la_item);

        slider << sl;
    }

    le_volume=new QLineEdit();

    cb_volume_auto=new QCheckBox("auto");
    connect(cb_volume_auto, SIGNAL(toggled(bool)), SLOT(onAutoVolStateChanged(bool)));

    QHBoxLayout *la_volume=new QHBoxLayout();
    la_volume->addWidget(new QLabel("volume:"));
    la_volume->addWidget(cb_volume_auto);
    la_volume->addWidget(le_volume);

    QPushButton *b_reset=new QPushButton("reset");
    connect(b_reset, SIGNAL(clicked(bool)), SLOT(reset()));

    QPushButton *b_ok=new QPushButton("ok");
    connect(b_ok, SIGNAL(clicked(bool)), SLOT(onOk()));

    QPushButton *b_cancel=new QPushButton("cancel");
    connect(b_cancel, SIGNAL(clicked(bool)), SLOT(onCancel()));

    cb_enabled=new QCheckBox("enabled");

    QHBoxLayout *la_buttons=new QHBoxLayout();
    la_buttons->addWidget(b_reset);
    la_buttons->addStretch(1);
    la_buttons->addWidget(b_ok);
    la_buttons->addWidget(b_cancel);
    la_buttons->addStretch(1);
    la_buttons->addWidget(cb_enabled);

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addLayout(la_preset);
    la_main->addLayout(la_band);
    la_main->addLayout(la_volume);
    la_main->addLayout(la_buttons);

    setLayout(la_main);
}

EqualizerView::~EqualizerView()
{
}

EQParams EqualizerView::params(bool full) const
{
    if(!cb_enabled->isChecked())
        return EQParams();

    return paramsBase(full);
}

double EqualizerView::volume() const
{
    return le_volume->text().toDouble();
}

EQParams EqualizerView::paramsBase(bool full) const
{
    EQParams params;
    EQBand band;

    for(int i=0; i<slider.size(); ++i) {
        if(!full && slider[i]->value()==0)
            continue;

        band.frequency=freq[i];

        band.width=freq[i]*1.2;

        band.gain=slider[i]->value()*.1;

        params.bands.push_back(band);
    }

    params.volume_auto=cb_volume_auto->isChecked();
    params.volume_manual=volume_manual;
    params.volume_value=volume();

    return params;
}

QVariantMap EqualizerView::getPresets() const
{
    QVariantMap map_root;
    QVariantMap map_values;
    QVariantMap map_preset;

    for(int i_preset=0; i_preset<cb_preset->count(); ++i_preset) {
        QVariantList list_gain;

        EQParams params=cb_preset->itemData(i_preset).value<EQParams>();

        for(int i_band=0; i_band<params.bands.size(); ++i_band) {
            list_gain << int(params.bands[i_band].gain*10);
        }

        map_values.insert("gain", list_gain);
        map_values.insert("volume_manual", params.volume_manual);
        map_values.insert("volume_auto", params.volume_auto);

        map_preset.insert(cb_preset->itemData(i_preset, Qt::DisplayRole).toString(), map_values);
    }

    map_root.insert("preset", map_preset);
    map_root.insert("index", cb_preset->currentIndex());
    map_root.insert("enabled", cb_enabled->isChecked());

    return map_root;
}

void EqualizerView::setPresets(const QVariantMap &map_root)
{
    QVariantMap map_preset=map_root.value("preset").toMap();

    QList <double> vol_man;
    QList <bool> vol_auto;

    for(int i_preset=0; i_preset<map_preset.size(); ++i_preset) {
        EQParams params;
        EQBand band;

        QVariantMap map_values=map_preset.values()[i_preset].toMap();
        QVariantList list_gain=map_values.value("gain").toList();

        params.volume_auto=map_values.value("volume_auto").toBool();
        params.volume_manual=map_values.value("volume_manual").toDouble();

        vol_auto << params.volume_auto;
        vol_man << params.volume_manual;

        for(int i_band=0; i_band<std::min(list_gain.size(), freq.size()); ++i_band) {
            band.frequency=freq[i_band];
            band.width=freq[i_band]*1.2;
            band.gain=list_gain[i_band].toDouble()*.1;

            params.bands.push_back(band);
        }

        cb_preset->addItem(map_preset.keys()[i_preset].simplified(), QVariant::fromValue(params));
    }

    cb_preset->setCurrentIndex(map_root.value("index").toInt());

    if(cb_preset->currentIndex()<0)
        cb_preset->setCurrentIndex(0);

    cb_enabled->setChecked(map_root.value("enabled").toBool());

    if(vol_man.size()>cb_preset->currentIndex()) {
        volume_manual=vol_man[cb_preset->currentIndex()];
        cb_volume_auto->setChecked(vol_auto[cb_preset->currentIndex()]);
    }

    calcVol();
}

void EqualizerView::reset()
{
    for(QSlider *sl:slider) {
        sl->setValue(0);
    }

    calcVol();
}

bool EqualizerView::presetAdd()
{
    const QString str=QInputDialog::getText(this, "", "preset name:").simplified();

    if(str.isEmpty())
        return false;

    cb_preset->addItem(str);
    cb_preset->setCurrentIndex(cb_preset->count() - 1);

    cb_preset->setItemData(cb_preset->count() - 1, QVariant::fromValue(paramsBase(true)));

    return true;
}

void EqualizerView::presetRename()
{
    if(cb_preset->currentIndex()<0)
        return;

    const QString str=QInputDialog::getText(this, "", "preset name:", QLineEdit::Normal, cb_preset->currentText()).simplified();

    if(str.isEmpty())
        return;

    cb_preset->setItemData(cb_preset->currentIndex(), str, Qt::DisplayRole);
}

void EqualizerView::presetRemove()
{
    if(cb_preset->currentIndex()<0)
        return;

    cb_preset->removeItem(cb_preset->currentIndex());
}

void EqualizerView::onOk()
{
    if(cb_preset->currentIndex()<0) {
        if(!presetAdd())
            return;
    }

    if(!cb_volume_auto->isChecked()) {
         volume_manual=le_volume->text().toDouble();
         le_volume->setText(QString::number(volume_manual));
    }

    cb_preset->setItemData(cb_preset->currentIndex(), QVariant::fromValue(paramsBase(true)));

    accept();
}

void EqualizerView::onCancel()
{
    setParams(cb_preset->currentData().value<EQParams>());

    reject();
}

void EqualizerView::setParams(const EQParams &params)
{
    for(int i=0; i<slider.size(); ++i) {
        if(params.bands.size()<=i)
            return;

        slider[i]->setValue(params.bands[i].gain*10);
    }

    cb_volume_auto->setChecked(params.volume_auto);
    volume_manual=params.volume_manual;

    calcVol();
}

void EqualizerView::onPresetIndexChanged(int index)
{
    setParams(cb_preset->itemData(index).value<EQParams>());
}

void EqualizerView::onAutoVolStateChanged(bool state)
{
    if(state) {
        calcVol();

    } else {
        le_volume->setText(QString::number(volume_manual));
    }

    le_volume->setReadOnly(state);
}

void EqualizerView::calcVol()
{
    if(!cb_volume_auto->isChecked()) {
        le_volume->setText(QString::number(volume_manual));
        return;
    }

    double max=0.;

    for(int i=0; i<slider.size(); ++i) {
        max=std::max(max, slider[i]->value()/10.);
    }

    le_volume->setText(QString::number(-max));
}

