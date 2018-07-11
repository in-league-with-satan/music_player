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

#ifndef FILELIST_VIEW_H
#define FILELIST_VIEW_H

#include <QWidget>

class QTreeView;
class QFileSystemModel;
class QMenu;

class FileProxyModel;

class FilelistView : public QWidget
{
    Q_OBJECT

public:

    FilelistView(QWidget *parent=0);
    ~FilelistView();

public slots:
    void setRootPath(const QString &dir);
    void setNameFilters(const QStringList &filters);

    void filterEmptyDirs(bool enabled);

    void tryAddUrls(const QStringList &urls);

private slots:
    void menuRequested(const QPoint &pos);
    void onDoubleClicked(const QModelIndex &index);
    void menuSendToPlaylist();
    void menuAddToPlaylist();

private:
    QStringList getSelected();
    void getDirEntryList(const QString &dir, QStringList *list);

    QTreeView *tree_files;
    QFileSystemModel *model_file_system;

    QMenu *context_menu;

    FileProxyModel *file_proxy_model;

signals:
    void updatePlaylist(const QStringList &list, bool drop_prev);
};

#endif // FILELIST_VIEW_H
