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

#ifndef SETTINGS_H
#define SETTINGS_H

#include <QObject>
#include <QVariantMap>
#include <QFont>

#undef QT_NEEDS_QMAIN

#define settings Settings::instance()

class Settings : public QObject
{
    Q_OBJECT

public:
    static Settings *createInstance(QObject *parent=0);
    static Settings *instance();

    bool load();
    bool save();

    struct Main {
        QString library_path;
        QString file_filter;
        QByteArray geometry;
        QByteArray state_window;
        QByteArray state_table;
        QVariantList playlist;
        int playlist_index;
        bool filter_empty_dirs;
        bool show_library;
        bool show_cover;
        int volume_level;
        bool cursor_follows_playback;
        bool skip_silence;
        QFont font_ctrl;

    } main;

    struct Lastfm {
        bool enabled;
        bool online;
        QString login;
        QString password;

    } lastfm;

    struct OutputDevice {
        bool use_default;
        QString dev_name;

    } output_device;

private:
    Settings(QObject *parent=0);

    static Settings *_instance;

signals:
    void updated();
};

#endif // SETTINGS_H
