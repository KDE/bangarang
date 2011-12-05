/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
* <http://gitorious.org/bangarang>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef INFOITEMVIEW_H
#define INFOITEMVIEW_H

#include <QListView>

class InfoItemDelegate;
class InfoItemModel;

class InfoItemView : public QListView
{
    Q_OBJECT
public:
    explicit InfoItemView(QWidget *parent = 0);
    void resizeEvent(QResizeEvent *e);
    void fixHeightToContents();
    void enableTouch();
    void suppressEditing(bool suppress);

signals:
    void updateSizeHints(QModelIndex index);

public slots:
    void infoDataChangedSlot(const QModelIndex &topleft, const QModelIndex &bottomright);
    void infoChanged(bool changed);

private:
    InfoItemModel *m_infoItemModel;
    InfoItemDelegate *m_infoItemDelegate;

};

#endif // INFOITEMVIEW_H
