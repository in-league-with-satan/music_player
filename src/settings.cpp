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
#include <QFile>
#include <QJsonDocument>
#include <QDir>
#include <qcoreapplication.h>

#include "settings.h"

Settings *Settings::_instance=nullptr;

Settings *Settings::createInstance(QObject *parent)
{
    if(_instance==nullptr)
        _instance=new Settings(parent);

    return _instance;
}

Settings *Settings::instance()
{
    return _instance;
}

//

QString filename()
{
    return qApp->applicationDirPath() + "/music_player.json";
}

//

Settings::Settings(QObject *parent) :
    QObject(parent)
{
}

bool Settings::load()
{
    QFile f(filename());

    QByteArray ba;

    bool file_readed=true;

    if(f.open(QFile::ReadOnly)) {
        ba=f.readAll();
        f.close();

    } else {
        file_readed=false;
    }

    QVariantMap map_root=QJsonDocument::fromJson(ba).toVariant().toMap();

    //

    QVariantMap map_main=map_root.value(QStringLiteral("main")).toMap();

    main.library_path=map_main.value(QStringLiteral("library_path"), QDir::homePath() + "/Music").toString();
    main.file_filter=map_main.value(QStringLiteral("file_filter"), "opus;ogg;mp3;flac;ape;aac;").toString();
    main.filter_empty_dirs=map_main.value(QStringLiteral("filter_empty_dirs"), false).toBool();
    main.geometry=QByteArray::fromBase64(map_main.value(QStringLiteral("geometry")).toByteArray());
    main.state_window=QByteArray::fromBase64(map_main.value(QStringLiteral("state_window")).toByteArray());
    main.state_table=QByteArray::fromBase64(map_main.value(QStringLiteral("state_table")).toByteArray());
    main.playlist=map_main.value(QStringLiteral("playlist")).toList();
    main.playlist_index=map_main.value(QStringLiteral("playlist_index"), 0).toInt();
    main.show_library=map_main.value(QStringLiteral("show_library"), true).toBool();
    main.volume_level=map_main.value(QStringLiteral("volume_level"), 1000).toInt();

    return file_readed;
}

bool Settings::save()
{
    QVariantMap map_root;
    QVariantMap map_main;

    map_main.insert(QStringLiteral("library_path"), main.library_path);
    map_main.insert(QStringLiteral("file_filter"), main.file_filter);
    map_main.insert(QStringLiteral("filter_empty_dirs"), main.filter_empty_dirs);
    map_main.insert(QStringLiteral("geometry"), QString(main.geometry.toBase64()));
    map_main.insert(QStringLiteral("state_window"), QString(main.state_window.toBase64()));
    map_main.insert(QStringLiteral("state_table"), QString(main.state_table.toBase64()));
    map_main.insert(QStringLiteral("playlist"), main.playlist);
    map_main.insert(QStringLiteral("playlist_index"), main.playlist_index);
    map_main.insert(QStringLiteral("show_library"), main.show_library);
    map_main.insert(QStringLiteral("volume_level"), main.volume_level);

    map_root.insert(QStringLiteral("main"), map_main);

    QByteArray ba=QJsonDocument::fromVariant(map_root).toJson();

    //

    QFile f(filename());

    if(!f.open(QFile::ReadWrite | QFile::Truncate))
        return false;

    f.write(ba);

    f.close();

    return true;
}
