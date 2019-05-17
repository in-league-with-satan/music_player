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
#include <QApplication>
#include <QLayout>
#include <QTreeView>
#include <QFileSystemModel>
#include <QDirIterator>
#include <QMenu>
#include <QTimer>

#include "file_proxy_model.h"

#include "filelist_view.h"

FilelistView::FilelistView(QWidget *parent)
    : QWidget(parent)
{
    context_menu=new QMenu();
    context_menu->addAction("send to playlist", this, SLOT(menuSendToPlaylist()));
    context_menu->addAction("add to playlist", this, SLOT(menuAddToPlaylist()));

    //

    timer_hold_check=new QTimer();
    timer_hold_check->setInterval(100);
    connect(timer_hold_check, SIGNAL(timeout()), SLOT(holdCheck()));

    //

    model_file_system=new QFileSystemModel();
    model_file_system->setReadOnly(true);
    model_file_system->setNameFilterDisables(false);

    file_proxy_model=new FileProxyModel();
    file_proxy_model->setSourceModel(model_file_system);

    tree_files=new QTreeView();
    tree_files->setModel(file_proxy_model);

    tree_files->setColumnHidden(1, true);
    tree_files->setColumnHidden(2, true);
    tree_files->setColumnHidden(3, true);
    tree_files->setHeaderHidden(true);
    tree_files->setSelectionMode(QAbstractItemView::ContiguousSelection);
    tree_files->setContextMenuPolicy(Qt::CustomContextMenu);

    connect(tree_files, SIGNAL(customContextMenuRequested(QPoint)), SLOT(menuRequested(QPoint)));
    connect(tree_files, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClicked(QModelIndex)));
    connect(tree_files, SIGNAL(pressed(QModelIndex)), SLOT(onPressed(QModelIndex)));

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(tree_files);
    la_main->setSpacing(0);
    la_main->setMargin(0);

    setLayout(la_main);
}

FilelistView::~FilelistView()
{
}

void FilelistView::setRootPath(const QString &dir)
{
    tree_files->setRootIndex(file_proxy_model->mapFromSource(model_file_system->setRootPath(dir)));
}

void FilelistView::setNameFilters(const QStringList &filters)
{
    model_file_system->setNameFilters(filters);
}

void FilelistView::filterEmptyDirs(bool enabled)
{
    file_proxy_model->setEnabled(enabled);
}

void FilelistView::tryAddUrls(const QStringList &urls)
{
    QStringList list;

    foreach(QString url, urls) {
        if(QFileInfo(url).isDir())
            getDirEntryList(url, &list);

        else
            list.append(url);
    }

    if(list.isEmpty())
        return;

    emit updatePlaylist(list, false);
}

void FilelistView::menuRequested(const QPoint &pos)
{
    context_menu->exec(mapToGlobal(pos));
}

void FilelistView::onDoubleClicked(const QModelIndex &index)
{
     if(model_file_system->isDir(file_proxy_model->mapToSource(index)))
         return;

     emit updatePlaylist(QStringList() << index.data(QFileSystemModel::FilePathRole).toString(), true);
}

void FilelistView::onPressed(const QModelIndex &index)
{
    Q_UNUSED(index);

    hold_counter=0;
    timer_hold_check->start();
}

void FilelistView::holdCheck()
{
    if((QApplication::mouseButtons()&Qt::LeftButton)==0) {
        timer_hold_check->stop();
        return;
    }

    if(hold_counter++>=10) {
        timer_hold_check->stop();
        menuRequested(mapFromGlobal(QCursor::pos()));
    }
}

void FilelistView::menuSendToPlaylist()
{
    emit updatePlaylist(getSelected(), true);
}

void FilelistView::menuAddToPlaylist()
{
    emit updatePlaylist(getSelected(), false);
}

QStringList FilelistView::getSelected()
{
    QStringList list;

    foreach(QModelIndex index, tree_files->selectionModel()->selectedRows()) {
        QString filepath=index.data(QFileSystemModel::FilePathRole).toString();

        if(model_file_system->isDir(file_proxy_model->mapToSource(index))) {
            getDirEntryList(filepath, &list);

        } else {
            if(!list.contains(filepath))
                list.append(filepath);
        }
    }

    return list;
}

void FilelistView::getDirEntryList(const QString &dir, QStringList *list)
{
    foreach(QString str, QDir(dir).entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        getDirEntryList(dir + "/" + str, list);
    }

    foreach(QString str, QDir(dir).entryList(model_file_system->nameFilters())) {
        const QString filepath=dir + "/" + str;

        if(!list->contains(filepath))
            list->append(filepath);
    }
}

