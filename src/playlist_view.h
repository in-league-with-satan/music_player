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

#ifndef PLAYLIST_VIEW_H
#define PLAYLIST_VIEW_H

#include <QWidget>
#include <QTableView>
#include <QStandardItemModel>

#include "track_metadata.h"

class QListView;
class QStringListModel;
class QMenu;

class PlaylistView : public QWidget
{
    Q_OBJECT

public:
    PlaylistView(QWidget *parent=0);
    ~PlaylistView();

    QString currentFile();

    QByteArray saveState() const;
    bool restoreState(const QByteArray &state);

    QVariantList savePlaylist();
    void restorePlaylist(const QVariantList &pl);

    int currentIndex() const;
    void setIndex(int index);

    struct Mark {
        enum T {
            Error,
            Play,
            Pause,
            Void
        };
    };

public slots:
    void updatePlaylist(const QStringList &list, bool drop_prev);
    void next();
    void prev();

    void mark(PlaylistView::Mark::T type);
    void markError();
    void markPlay();
    void markPause();
    void markVoid();

    void setCursorFollowsPlayback(bool state);

private slots:
    void startPlaying(const QModelIndex &index);
    void menuRequested(const QPoint &pos);

    void removeRows();

private:
    void makePlayRequest();

    void markRow(int index, bool state);

    void appendRow(TrackMetadata &tmd);

    enum {
        RoleFilepath=Qt::UserRole + 1,
        RoleMetadata,
        RoleMarker
    };

    QTableView *table;
    QStandardItemModel *model;

    QMenu *context_menu;

    int current_index;

    bool cursor_follows_playback;

signals:
    void playRequest(const QString &filename);
    void currentIndexRemoved();

    void nowPlaying(TrackMetadata tmd);
};

#endif // PLAYLIST_VIEW_H
