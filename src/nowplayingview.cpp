/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Stefan Burnicki (stefan.burnicki@gmx.de)
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

#include "nowplayingview.h"
#include "bangarangapplication.h"
#include "mainwindow.h"
#include "actionsmanager.h"
#include "nowplayingdelegate.h"
#include <QMenu>
#include <QHeaderView>
#include "platform/playlist.h"
#include "platform/mediaitemmodel.h"

NowPlayingView::NowPlayingView(QWidget* parent): QTreeView(parent)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_nowPlayingModel = m_application->playlist()->nowPlayingModel();
    setModel( m_nowPlayingModel );
    header()->setVisible(false);
    m_nowPlayingDelegate = NULL;
    connect(m_nowPlayingModel, SIGNAL(mediaListChanged()), this, SLOT(tidyHeader()));
}

NowPlayingView::~NowPlayingView() {
}

void NowPlayingView::setMainWindow(MainWindow * mainWindow)
{
    m_nowPlayingDelegate = new NowPlayingDelegate(mainWindow);
    setItemDelegate(m_nowPlayingDelegate);
    m_nowPlayingDelegate->setView(this);
}

void NowPlayingView::contextMenuEvent(QContextMenuEvent * event)
{
    QMenu * menu = m_application->actionsManager()->nowPlayingContextMenu();
    menu->exec(event->globalPos());
}

void NowPlayingView::resizeEvent(QResizeEvent *event)
{
    if (m_nowPlayingDelegate) {
        m_nowPlayingDelegate->updateSizeHint();
    }
    QTreeView::resizeEvent(event);
}

void NowPlayingView::tidyHeader()
{
    header()->setStretchLastSection(false);
    header()->setResizeMode(0, QHeaderView::Stretch);
}

void NowPlayingView::showInfo()
{
    if (m_nowPlayingDelegate) {
        m_nowPlayingDelegate->setShowInfo(true);
    }
}

void NowPlayingView::hideInfo()
{
    if (m_nowPlayingDelegate) {
        m_nowPlayingDelegate->setShowInfo(false);
    }
}

