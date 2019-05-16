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

#ifndef SETTINGS_VIEW_H
#define SETTINGS_VIEW_H

#include <QDialog>

class QLineEdit;
class QCheckBox;
class QComboBox;

class SettingsView : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsView(QWidget *parent=0);

public slots:
    virtual int exec();

    void lastfmBadauth();

private slots:
    void updateSettings();

    void selectLibraryPath();

    void reloadOutputDevices();

private:
    QCheckBox *cb_device_default;
    QComboBox *cb_device_name;

    QLineEdit *le_library_path;
    QLineEdit *le_file_filter;

    QLineEdit *le_lastfm_login;
    QLineEdit *le_lastfm_password;

    QCheckBox *cb_filter_empty_dirs;
    QCheckBox *cb_skip_silence;
};

#endif // SETTINGS_VIEW_H
