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
#include <QListView>
#include <QStringListModel>
#include <QFileInfo>
#include <QHeaderView>
#include <QDateTime>
#include <QFileSystemModel>
#include <QMenu>
#include <qcoreapplication.h>

#include "ff_tools.h"

#include "playlist_view.h"

enum {
    col_num_track,
    col_num_date,
    col_num_artist,
    col_num_album,
    col_num_title,
    col_num_duration,
    col_num_state,
    col_size
};

PlaylistView::PlaylistView(QWidget *parent)
    : QWidget(parent)
    , current_index(0)
{
    context_menu=new QMenu();

    QAction *action=context_menu->addAction("remove", this, SLOT(removeRows()));

    action->setShortcut(QKeySequence(Qt::Key_Delete));
    connect(action, SIGNAL(triggered(bool)), SLOT(removeRows()));

    addAction(action);

    //

    model=new QStandardItemModel();

    table=new QTableView();
    table->setModel(model);
    table->setAlternatingRowColors(true);
    table->setSelectionMode(QAbstractItemView::ExtendedSelection);
    table->setContextMenuPolicy(Qt::CustomContextMenu);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setFocusPolicy(Qt::StrongFocus);

    connect(table, SIGNAL(customContextMenuRequested(QPoint)), SLOT(menuRequested(QPoint)));
    connect(table, SIGNAL(activated(QModelIndex)), SLOT(startPlaying(QModelIndex)));


    QPalette p=table->palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    table->setPalette(p);

    connect(table, SIGNAL(doubleClicked(QModelIndex)), SLOT(onDoubleClicked(QModelIndex)));

    QVBoxLayout *la_main=new QVBoxLayout();
    la_main->addWidget(table);
    la_main->setSpacing(0);
    la_main->setMargin(0);

    setLayout(la_main);

    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    model->setHorizontalHeaderLabels(QStringList() << "track" << " date " << "artist" << "album" << "title" << "duration" << "state");

    table->horizontalHeader()->setSectionResizeMode(col_num_track, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(col_num_date, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(col_num_title, QHeaderView::Stretch);
    table->horizontalHeader()->setSectionResizeMode(col_num_duration, QHeaderView::ResizeToContents);
    table->horizontalHeader()->setSectionResizeMode(col_num_state, QHeaderView::ResizeToContents);

    table->horizontalHeader()->setSectionsMovable(true);
}

PlaylistView::~PlaylistView()
{
}

QString PlaylistView::currentFile()
{
    QModelIndex index=model->index(current_index, col_num_title);

    if(!index.isValid())
        return QString();

    return index.data(QFileSystemModel::FilePathRole).toString();
}

QByteArray PlaylistView::saveState() const
{
    return table->horizontalHeader()->saveState();
}

bool PlaylistView::restoreState(const QByteArray &state)
{
    return table->horizontalHeader()->restoreState(state);
}

QVariantList PlaylistView::savePlaylist()
{
    QVariantList list;

    QVariantMap item;

    for(int row=0; row<model->rowCount(); ++row) {
        item.insert("filepath", model->item(row, col_num_title)->data(QFileSystemModel::FilePathRole));
        item.insert("title", model->item(row, col_num_title)->data(Qt::DisplayRole));
        item.insert("tracknumber", model->item(row, col_num_track)->data(Qt::DisplayRole));
        item.insert("date", model->item(row, col_num_date)->data(Qt::DisplayRole));
        item.insert("artist", model->item(row, col_num_artist)->data(Qt::DisplayRole));
        item.insert("album", model->item(row, col_num_album)->data(Qt::DisplayRole));
        item.insert("title", model->item(row, col_num_title)->data(Qt::DisplayRole));
        item.insert("duration", model->item(row, col_num_duration)->data(Qt::DisplayRole));

        list.append(item);
    }

    return list;
}

void PlaylistView::restorePlaylist(const QVariantList &pl)
{
    model->removeRows(0, model->rowCount());

    foreach(const QVariant &var, pl) {
        const QVariantMap &item=var.toMap();

        const int row_num=model->rowCount();

        model->appendRow(0);

        QStandardItem *item_tracknumber=new QStandardItem(item.value("tracknumber").toString());
        item_tracknumber->setFlags(item_tracknumber->flags()^Qt::ItemIsEditable);
        item_tracknumber->setTextAlignment(Qt::AlignCenter);
        item_tracknumber->setData(item.value("filepath"), Qt::ToolTipRole);

        QStandardItem *item_date=new QStandardItem(item.value("date").toString());
        item_date->setFlags(item_date->flags()^Qt::ItemIsEditable);
        item_date->setTextAlignment(Qt::AlignCenter);

        QStandardItem *item_artist=new QStandardItem(item.value("artist").toString());
        item_artist->setFlags(item_artist->flags()^Qt::ItemIsEditable);
        item_artist->setData(item.value("artist"), Qt::ToolTipRole);

        QStandardItem *item_album=new QStandardItem(item.value("album").toString());
        item_album->setFlags(item_album->flags()^Qt::ItemIsEditable);
        item_album->setData(item.value("album"), Qt::ToolTipRole);

        QStandardItem *item_title=new QStandardItem(item.value("title").toString());
        item_title->setFlags(item_title->flags()^Qt::ItemIsEditable);
        item_title->setData(item.value("filepath"), QFileSystemModel::FilePathRole);
        item_title->setData(item.value("title"), Qt::ToolTipRole);

        QStandardItem *item_duration=new QStandardItem(item.value("duration").toString());
        item_duration->setFlags(item_duration->flags()^Qt::ItemIsEditable);
        item_duration->setTextAlignment(Qt::AlignCenter);

        QStandardItem *item_state=new QStandardItem();
        item_state->setFlags(item_state->flags()^Qt::ItemIsEditable);
        item_state->setTextAlignment(Qt::AlignCenter);

        model->setItem(row_num, col_num_track, item_tracknumber);
        model->setItem(row_num, col_num_date, item_date);
        model->setItem(row_num, col_num_artist, item_artist);
        model->setItem(row_num, col_num_album, item_album);
        model->setItem(row_num, col_num_title, item_title);
        model->setItem(row_num, col_num_duration, item_duration);
        model->setItem(row_num, col_num_state, item_state);

        for(int col=0; col<col_size; ++col) {
            model->item(row_num, col)->setData(model->item(row_num, col)->data(RoleOrigBGColor), Qt::BackgroundRole);
            model->item(row_num, col)->setData(model->item(row_num, col)->data(RoleOrigTextColor), Qt::TextColorRole);
        }
    }
}

int PlaylistView::currentIndex() const
{
    return current_index;
}

void PlaylistView::setIndex(int index)
{
    if(index<0 || index>model->rowCount())
        return;

    current_index=index;

    markRow(current_index, true);

    const QModelIndex model_index=model->index(current_index, col_num_track);

    table->scrollTo(model_index, QTableView::PositionAtCenter);
    table->scrollTo(model_index, QTableView::PositionAtCenter); // dirty hack? X_x

    if(table->selectionModel()->selectedRows().isEmpty())
        table->selectRow(index);
}

void PlaylistView::updatePlaylist(const QStringList &list, bool drop_prev)
{
    if(drop_prev) {
        model->removeRows(0, model->rowCount());
        current_index=0;
    }

    foreach(const QString &filepath, list) {
        const int row_num=model->rowCount();

        model->appendRow(0);

        QFileInfo fi(filepath);

        TagsReader::Tags t=TagsReader::get(filepath);

        if(t.title.isEmpty())
            t.title=fi.fileName();

        QStandardItem *item_tracknumber=new QStandardItem(t.tracknumber);
        item_tracknumber->setFlags(item_tracknumber->flags()^Qt::ItemIsEditable);
        item_tracknumber->setTextAlignment(Qt::AlignCenter);
        item_tracknumber->setData(filepath, Qt::ToolTipRole);

        QStandardItem *item_date=new QStandardItem(t.date);
        item_date->setFlags(item_date->flags()^Qt::ItemIsEditable);
        item_date->setTextAlignment(Qt::AlignCenter);

        QStandardItem *item_artist=new QStandardItem(t.artist);
        item_artist->setFlags(item_artist->flags()^Qt::ItemIsEditable);
        item_artist->setData(t.artist, Qt::ToolTipRole);

        QStandardItem *item_album=new QStandardItem(t.album);
        item_album->setFlags(item_album->flags()^Qt::ItemIsEditable);
        item_album->setData(t.album, Qt::ToolTipRole);

        QStandardItem *item_title=new QStandardItem(t.title);
        item_title->setFlags(item_title->flags()^Qt::ItemIsEditable);
        item_title->setData(filepath, QFileSystemModel::FilePathRole);
        item_title->setData(t.title, Qt::ToolTipRole);

        QStandardItem *item_duration=new QStandardItem(t.duration);
        item_duration->setFlags(item_duration->flags()^Qt::ItemIsEditable);
        item_duration->setTextAlignment(Qt::AlignCenter);

        QStandardItem *item_state=new QStandardItem();
        item_state->setFlags(item_state->flags()^Qt::ItemIsEditable);
        item_state->setTextAlignment(Qt::AlignCenter);

        model->setItem(row_num, col_num_track, item_tracknumber);
        model->setItem(row_num, col_num_date, item_date);
        model->setItem(row_num, col_num_artist, item_artist);
        model->setItem(row_num, col_num_album, item_album);
        model->setItem(row_num, col_num_title, item_title);
        model->setItem(row_num, col_num_duration, item_duration);
        model->setItem(row_num, col_num_state, item_state);

        for(int col=0; col<col_size; ++col) {
            model->item(row_num, col)->setData(model->item(row_num, col)->data(RoleOrigBGColor), Qt::BackgroundRole);
            model->item(row_num, col)->setData(model->item(row_num, col)->data(RoleOrigTextColor), Qt::TextColorRole);
        }
    }

    if(drop_prev)
        makePlayRequest();
}

void PlaylistView::startPlaying(const QModelIndex &index)
{
    current_index=index.row();

    makePlayRequest();
}

void PlaylistView::next()
{
    current_index++;

    if(current_index>=model->rowCount())
        current_index=0;

    makePlayRequest();
}

void PlaylistView::prev()
{
    current_index--;

    if(current_index<0)
        current_index=model->rowCount() - 1;

    if(current_index<0)
        current_index=0;

    makePlayRequest();
}

void PlaylistView::mark(PlaylistView::Mark::T type)
{
    QStandardItem *item=nullptr;

    if(type==Mark::Error) {
        item=model->item(current_index, col_num_state);

        if(!item)
            return;

        item->setData("err", Qt::DisplayRole);

        return;
    }

    QString symbol="";

    if(type==Mark::Play)
        symbol="▶";

    if(type==Mark::Pause)
        symbol="▮▮";

    for(int i=0; i<model->rowCount(); ++i) {
        item=model->item(i, col_num_state);

        if(!item)
            continue;

        if(i==current_index) {
            item->setData(symbol, Qt::DisplayRole);

            markRow(i, true);

            table->scrollTo(item->index());

        } else if(item->data(RoleMarker).toBool()) {
            if("err"==item->data(Qt::DisplayRole).toString())
                continue;

            item->setData("", Qt::DisplayRole);

            markRow(i, false);
        }
    }
}

void PlaylistView::markError()
{
    mark(Mark::Error);
}

void PlaylistView::markPlay()
{
    mark(Mark::Play);
}

void PlaylistView::markPause()
{
    mark(Mark::Pause);
}

void PlaylistView::markVoid()
{
    mark(Mark::Void);
}

void PlaylistView::onDoubleClicked(const QModelIndex &index)
{
    if(!index.isValid())
        return;

    current_index=index.row();

    makePlayRequest();
}

void PlaylistView::makePlayRequest()
{
    QModelIndex index=model->index(current_index, col_num_title);

    if(!index.isValid())
        return;

    emit playRequest(index.data(QFileSystemModel::FilePathRole).toString());
}

void PlaylistView::markRow(int index, bool state)
{
    if(index<0 || index>=model->rowCount())
        return;

    model->item(index, col_num_state)->setData(state, RoleMarker);

    if(state) {
        for(int col=0; col<col_size; ++col) {
            model->item(index, col)->setData(QVariant(QColor(Qt::black)), Qt::BackgroundRole);
            model->item(index, col)->setData(QVariant(QColor(Qt::white)), Qt::TextColorRole);
        }

    } else {
        for(int col=0; col<col_size; ++col) {
            model->item(index, col)->setData(model->item(index, col)->data(RoleOrigBGColor), Qt::BackgroundRole);
            model->item(index, col)->setData(model->item(index, col)->data(RoleOrigTextColor), Qt::TextColorRole);
        }
    }
}

void PlaylistView::menuRequested(const QPoint &pos)
{
    context_menu->exec(table->viewport()->mapToGlobal(pos));
}

void PlaylistView::removeRows()
{
    QList <QPersistentModelIndex> list;

    foreach(const QModelIndex &idx, table->selectionModel()->selectedRows())
        list << idx;

    bool current_index_removed=false;

    foreach(const QPersistentModelIndex &idx, list) {
        if(idx.row()<current_index)
            current_index--;

        else if(idx.row()==current_index)
            current_index_removed=true;

        model->removeRow(idx.row());
    }

    if(current_index_removed) {
        if(model->rowCount()==0 || current_index<0)
            current_index=0;

        else if(current_index>=model->rowCount())
            current_index=model->rowCount() - 1;

        emit currentIndexRemoved();
    }
}
