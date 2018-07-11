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
#include <QFileSystemModel>
#include <QDirIterator>

#include <functional>

#include "file_proxy_model.h"

FileProxyModel::FileProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
    , enabled(true)
{
}

void FileProxyModel::setEnabled(bool enabled)
{
    // qDebug() << "FileProxyModel::setEnabled:" << enabled;

    this->enabled=enabled;

    invalidateFilter();
}

bool FileProxyModel::filterAcceptsRow(int source_row, const QModelIndex &source_parent) const
{
    if(!enabled)
        return true;

    QFileSystemModel *model=static_cast<QFileSystemModel*>(sourceModel());

    QModelIndex index=model->index(source_row, 0, source_parent);

    if(!model->isDir(index))
        return true;

    static std::function<bool(const QString&, const QStringList&)>dirHasFiles=
            [](const QString &dir_path, const QStringList &filter)->bool {
        // qDebug() << "dirHasFiles:" << dir_path;

        QDir dir(dir_path);

        if(!dir.entryList(filter).isEmpty())
            return true;

        foreach(QString dir_name, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
            if(dirHasFiles(dir_path + "/" + dir_name, filter))
                return true;
        }

        return false;
    };

    return dirHasFiles(index.data(QFileSystemModel::FilePathRole).toString(), model->nameFilters());
}
