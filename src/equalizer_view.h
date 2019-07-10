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

#ifndef EQUALIZER_VIEW_H
#define EQUALIZER_VIEW_H

#include <QDialog>

#include "ff_equalizer.h"

class QSlider;
class QComboBox;
class QCheckBox;
class QLineEdit;

class EqualizerView : public QDialog
{
    Q_OBJECT

public:
    explicit EqualizerView(QWidget *parent=nullptr);
    ~EqualizerView();

    EQParams params(bool full=true) const;
    double volume() const;

    QVariantMap getPresets() const;
    void setPresets(const QVariantMap &map_root);

public slots:
    void reset();

private slots:
    bool presetAdd();
    void presetRename();
    void presetRemove();

    void onOk();
    void onCancel();

    void setParams(const EQParams &params);

    void onPresetIndexChanged(int index);

    void onAutoVolStateChanged(bool state);

    void calcVol();

private:
    EQParams paramsBase(bool full) const;

    const QVector <int> freq;
    QVector <QSlider*> slider;

    QComboBox *cb_preset;

    QCheckBox *cb_enabled;

    QCheckBox *cb_volume_auto;

    QLineEdit *le_volume;

    QVariantMap map_preset;

    double volume_manual=0;
};

#endif // EQUALIZER_VIEW_H
