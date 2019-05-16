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

#include <QDebug>
#include <QLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QCheckBox>
#include <QComboBox>
#include <QFileDialog>
#include <QGroupBox>
#include <QAudioDeviceInfo>
#include <qcoreapplication.h>

#include "settings.h"

#include "settings_view.h"

SettingsView::SettingsView(QWidget *parent)
 : QDialog(parent)
{
    setWindowTitle("settings");

    setModal(true);

    cb_device_default=new QCheckBox("use default device");
    cb_device_name=new QComboBox();

    QPushButton *b_reload_devices=new QPushButton("reload");
    connect(b_reload_devices, SIGNAL(clicked(bool)), SLOT(reloadOutputDevices()));

    QPushButton *b_select_dir=new QPushButton("select dir");
    connect(b_select_dir, SIGNAL(clicked(bool)), SLOT(selectLibraryPath()));

    QLabel *l_library_path=new QLabel("library path:");

    QLabel *l_file_filter=new QLabel("file filter:");

    le_library_path=new QLineEdit();
    le_file_filter=new QLineEdit();

    cb_filter_empty_dirs=new QCheckBox("filter empty dirs");

    cb_skip_silence=new QCheckBox("skip silence");

    int row=0;

    QGridLayout *la_device=new QGridLayout();
    la_device->addWidget(cb_device_default, row++, 0);
    la_device->addWidget(cb_device_name, row, 0);
    la_device->addWidget(b_reload_devices, row, 1);
    la_device->setColumnStretch(0, 10);
    la_device->setColumnStretch(1, 0);

    QGroupBox *gb_device=new QGroupBox("output device");
    gb_device->setLayout(la_device);

    //

    QGridLayout *la_basic_settings=new QGridLayout();

    row=0;

    la_basic_settings->addWidget(l_library_path, row, 0);
    la_basic_settings->addWidget(le_library_path, row, 1);
    la_basic_settings->addWidget(b_select_dir, row, 2);

    row++;

    la_basic_settings->addWidget(l_file_filter, row, 0);
    la_basic_settings->addWidget(le_file_filter, row, 1);

    row++;

    la_basic_settings->addWidget(cb_filter_empty_dirs, row, 1);

    row++;

    la_basic_settings->addWidget(cb_skip_silence, row, 1);


    QGroupBox *gb_basic_settings=new QGroupBox("basic settings");
    gb_basic_settings->setLayout(la_basic_settings);

    //


    le_lastfm_login=new QLineEdit();
    le_lastfm_password=new QLineEdit();

    le_lastfm_password->setEchoMode(QLineEdit::PasswordEchoOnEdit);

    QLabel *l_lastfm_login=new QLabel("login:");
    QLabel *l_lastfm_password=new QLabel("password:");

    QGridLayout *la_lastfm=new QGridLayout();

    row=0;

    la_lastfm->addWidget(l_lastfm_login, row, 0);
    la_lastfm->addWidget(le_lastfm_login, row, 1);

    row++;

    la_lastfm->addWidget(l_lastfm_password, row, 0);
    la_lastfm->addWidget(le_lastfm_password, row, 1);

    QGroupBox *gb_lastfm=new QGroupBox("last.fm");
    gb_lastfm->setLayout(la_lastfm);


    //

    QPushButton *b_ok=new QPushButton("ok");
    connect(b_ok, SIGNAL(clicked(bool)), SLOT(accept()));

    QPushButton *b_cancel=new QPushButton("cancel");
    connect(b_cancel, SIGNAL(clicked(bool)), SLOT(reject()));

    QHBoxLayout *la_buttons=new QHBoxLayout();
    la_buttons->addStretch(1);
    la_buttons->addWidget(b_ok);
    la_buttons->addWidget(b_cancel);
    la_buttons->addStretch(1);

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(gb_device);
    la_main->addWidget(gb_basic_settings);
    la_main->addWidget(gb_lastfm);
    la_main->addLayout(la_buttons);

    setLayout(la_main);

    connect(this, SIGNAL(accepted()), SLOT(updateSettings()));

    reloadOutputDevices();
}

int SettingsView::exec()
{
    le_library_path->setText(settings->main.library_path);
    le_file_filter->setText(settings->main.file_filter);
    cb_filter_empty_dirs->setChecked(settings->main.filter_empty_dirs);
    cb_skip_silence->setChecked(settings->main.skip_silence);

    le_lastfm_login->setText(settings->lastfm.login);
    le_lastfm_password->setText(settings->lastfm.password);

    cb_device_default->setChecked(settings->output_device.use_default);


    for(int i=0; i<cb_device_name->count(); ++i) {
        if(cb_device_name->itemText(i)==settings->output_device.dev_name) {
            cb_device_name->setCurrentIndex(i);
            break;
        }
    }


    return QDialog::exec();
}

void SettingsView::lastfmBadauth()
{
    settings->lastfm.password.clear();
}

void SettingsView::updateSettings()
{
    settings->main.library_path=le_library_path->text().simplified();
    settings->main.file_filter=le_file_filter->text().simplified();
    settings->main.filter_empty_dirs=cb_filter_empty_dirs->isChecked();
    settings->main.skip_silence=cb_skip_silence->isChecked();

    settings->lastfm.login=le_lastfm_login->text().simplified();
    settings->lastfm.password=le_lastfm_password->text().simplified();

    settings->output_device.use_default=cb_device_default->isChecked();
    settings->output_device.dev_name=cb_device_name->currentText();

    emit settings->updated();
}

void SettingsView::selectLibraryPath()
{
    QString path=QFileDialog::getExistingDirectory(this, "select library directory", le_library_path->text().simplified());

    if(path.isEmpty())
        return;

    le_library_path->setText(path);
}

void SettingsView::reloadOutputDevices()
{
    cb_device_name->clear();

    foreach(QAudioDeviceInfo dev, QAudioDeviceInfo::availableDevices(QAudio::AudioOutput)) {
        cb_device_name->addItem(dev.deviceName());

        if(dev.deviceName()==settings->output_device.dev_name)
            cb_device_name->setCurrentIndex(cb_device_name->count() - 1);
    }
}
