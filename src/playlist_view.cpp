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
#include <QScroller>
#include <qcoreapplication.h>

#include "ff_tools.h"
#include "settings.h"

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
    , cursor_follows_playback(true)
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
    table->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    table->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    // QScroller::grabGesture(table, QScroller::LeftMouseButtonGesture);
    QScroller::grabGesture(table, QScroller::TouchGesture);

    QPalette p=table->palette();
    p.setColor(QPalette::Inactive, QPalette::Highlight, p.color(QPalette::Active, QPalette::Highlight));
    p.setColor(QPalette::Inactive, QPalette::HighlightedText, p.color(QPalette::Active, QPalette::HighlightedText));
    table->setPalette(p);

    connect(table, SIGNAL(customContextMenuRequested(QPoint)), SLOT(menuRequested(QPoint)));
    connect(table, SIGNAL(activated(QModelIndex)), SLOT(startPlaying(QModelIndex)));

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

    TrackMetadata md;

    for(int row=0; row<model->rowCount(); ++row) {
        md.fromVariant(model->item(row, col_num_track)->data(RoleMetadata));

        item.insert("filepath", md.filepath);
        item.insert("title", md.title);
        item.insert("track_number", md.track_number);
        item.insert("date", md.date);
        item.insert("artist", md.artist);
        item.insert("album", md.album);
        item.insert("title", md.title);
        item.insert("duration", md.track_length);

        list.append(item);
    }

    return list;
}

void PlaylistView::appendRow(TrackMetadata &md)
{
    const int row_num=model->rowCount();

    model->appendRow(nullptr);

    QStandardItem *item_tracknumber=new QStandardItem(md.track_number);
    item_tracknumber->setFlags(item_tracknumber->flags()^Qt::ItemIsEditable);
    item_tracknumber->setTextAlignment(Qt::AlignCenter);
    item_tracknumber->setData(md.filepath, Qt::ToolTipRole);
    item_tracknumber->setData(md.toVariant(), RoleMetadata);

    QStandardItem *item_date=new QStandardItem(md.date);
    item_date->setFlags(item_date->flags()^Qt::ItemIsEditable);
    item_date->setTextAlignment(Qt::AlignCenter);

    QStandardItem *item_artist=new QStandardItem(md.artist);
    item_artist->setFlags(item_artist->flags()^Qt::ItemIsEditable);
    item_artist->setData(md.artist, Qt::ToolTipRole);

    QStandardItem *item_album=new QStandardItem(md.album);
    item_album->setFlags(item_album->flags()^Qt::ItemIsEditable);
    item_album->setData(md.album, Qt::ToolTipRole);

    QStandardItem *item_title=new QStandardItem(md.title);
    item_title->setFlags(item_title->flags()^Qt::ItemIsEditable);
    item_title->setData(md.filepath, QFileSystemModel::FilePathRole);
    item_title->setData(md.title, Qt::ToolTipRole);

    QStandardItem *item_duration=new QStandardItem(timeToStringSec(md.track_length));
    item_duration->setFlags(item_duration->flags()^Qt::ItemIsEditable);
    item_duration->setTextAlignment(Qt::AlignCenter);

    QStandardItem *item_state=new QStandardItem();
    item_state->setFlags(item_state->flags()^Qt::ItemIsEditable);
    item_state->setTextAlignment(Qt::AlignCenter);
    item_state->setData(settings->main.font_ctrl, Qt::FontRole);

    model->setItem(row_num, col_num_track, item_tracknumber);
    model->setItem(row_num, col_num_date, item_date);
    model->setItem(row_num, col_num_artist, item_artist);
    model->setItem(row_num, col_num_album, item_album);
    model->setItem(row_num, col_num_title, item_title);
    model->setItem(row_num, col_num_duration, item_duration);
    model->setItem(row_num, col_num_state, item_state);
}

void PlaylistView::restorePlaylist(const QVariantList &pl)
{
    model->removeRows(0, model->rowCount());

    TrackMetadata md;

    foreach(const QVariant &var, pl) {
        const QVariantMap &item=var.toMap();

        md.title=item.value("title").toString();
        md.artist=item.value("artist").toString();
        md.album=item.value("album").toString();
        md.track_number=item.value("track_number").toString();
        md.date=item.value("date").toString();
        md.filepath=item.value("filepath").toString();
        md.track_length=item.value("duration").toLongLong();
        md.start_playing_time=-1;

        appendRow(md);
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

    if(cursor_follows_playback) {
        table->selectRow(index);
        table->scrollTo(model_index, QTableView::PositionAtCenter);
        table->scrollTo(model_index, QTableView::PositionAtCenter); // dirty hack? X_x
    }

    if(table->selectionModel()->selectedRows().isEmpty())
        table->selectRow(index);
}

void PlaylistView::updatePlaylist(const QStringList &list, bool drop_prev)
{
    if(drop_prev) {
        model->removeRows(0, model->rowCount());
        current_index=0;
    }

    TrackMetadata md;

    foreach(const QString &filepath, list) {
        QFileInfo fi(filepath);

        md=readMetadata(filepath);

        if(md.title.isEmpty())
            md.title=fi.fileName();

        appendRow(md);
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

            if(cursor_follows_playback) {
                table->selectRow(i);
                table->scrollTo(item->index());
            }

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
    // qDebug() << "PlaylistView::markPlay";

    mark(Mark::Play);

    QModelIndex index=model->index(current_index, col_num_track);

    if(!index.isValid())
        return;

    emit nowPlaying(TrackMetadata().fromVariant(index.data(RoleMetadata)));
}

void PlaylistView::markPause()
{
    mark(Mark::Pause);
}

void PlaylistView::markVoid()
{
    mark(Mark::Void);
}

void PlaylistView::setCursorFollowsPlayback(bool state)
{
    cursor_follows_playback=state;
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
