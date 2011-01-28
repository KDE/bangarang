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

#include "infoitemview.h"
#include "infoitemdelegate.h"
#include "platform/infoitemmodel.h"

InfoItemView::InfoItemView(QWidget *parent) :
    QListView(parent)
{
    m_infoItemModel = new InfoItemModel(this);
    setModel(m_infoItemModel);
    m_infoItemDelegate = new InfoItemDelegate(this);
    m_infoItemDelegate->setView(this);
    setItemDelegate(m_infoItemDelegate);
    connect(this, SIGNAL(updateSizeHints(QModelIndex)), m_infoItemDelegate, SIGNAL(sizeHintChanged(QModelIndex)));
    connect(m_infoItemModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)), this, SLOT(infoDataChangedSlot(const QModelIndex, const QModelIndex)));
    connect(m_infoItemModel, SIGNAL(infoChanged(bool)), this, SLOT(infoChanged(bool)));

}

void InfoItemView::fixHeightToContents()
{
    //Fix height to contents
    int infoItemViewHeight = m_infoItemDelegate->heightForAllRows();
    setMinimumHeight(infoItemViewHeight);
    setMaximumHeight(infoItemViewHeight);
}

void InfoItemView::resizeEvent(QResizeEvent *e)
{
    fixHeightToContents();
    for (int i = 0; i < model()->rowCount(); i++) {
        emit updateSizeHints(model()->index(i,0));
    }
    QListView::resizeEvent(e);
}

void InfoItemView::infoDataChangedSlot(const QModelIndex &topleft, const QModelIndex &bottomright)
{
    m_infoItemDelegate->resetEditMode();
    fixHeightToContents();
    Q_UNUSED(topleft);
    Q_UNUSED(bottomright);
}
void InfoItemView::infoChanged(bool changed)
{
    m_infoItemDelegate->resetEditMode();
    fixHeightToContents();
    Q_UNUSED(changed);
}

