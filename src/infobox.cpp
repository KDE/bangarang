/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)
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

#include "infobox.h"
#include "mainwindow.h"
#include "mediaview.h"
#include "mediaitemdelegate.h"
#include "infomanager.h"
#include "bangarangapplication.h"
#include "platform/mediaitemmodel.h"
#include <KIcon>
#include <KGlobalSettings>
#include <KDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QApplication>

InfoBox::InfoBox(QWidget * parent):QWidget (parent) 
{
    //Set up title bar
    m_titleBar = new QWidget;
    m_titleBar->setAutoFillBackground(true);
    m_titleBar->setPalette(QApplication::palette());
    connect(KGlobalSettings::self(), SIGNAL(kdisplayPaletteChanged()), this, SLOT(updateTitleColors())); 
    
    QHBoxLayout * titleLayout = new QHBoxLayout;
    titleLayout->setContentsMargins(0,0,0,0);
    titleLayout->setSpacing(0);
    
    m_icon = new QLabel;
    m_icon->setMaximumSize(QSize(20,20));
    m_icon->setAlignment(Qt::AlignCenter);
    m_title = new QLabel;
    m_title->setFont(KGlobalSettings::smallestReadableFont());
    m_title->setAlignment(Qt::AlignCenter);
    QLabel * spacer = new QLabel;
    spacer->setMaximumSize(QSize(20,20));
    titleLayout->addWidget(m_icon);
    titleLayout->addWidget(m_title);
    titleLayout->addWidget(spacer);
    m_titleBar->setLayout(titleLayout);
    m_titleBar->setMinimumHeight(20);
    
    //Set up media view
    QVBoxLayout * layout = new QVBoxLayout;
    layout->setContentsMargins(0,0,0,0);
    layout->setSpacing(0);
    m_mediaView = new MediaView();
    connect((MediaItemModel *)m_mediaView->sourceModel(), SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    layout->addWidget(m_titleBar);
    layout->addWidget(m_mediaView);
    
    //Complete setup
    setLayout(layout);
}

InfoBox::~InfoBox() 
{
}

QLabel * InfoBox::title()
{ 
    return m_title;
}

MediaView * InfoBox::mediaView() 
{
    return m_mediaView;
}

void InfoBox::setMainWindow(MainWindow * mainWindow)
{
    m_mainWindow = mainWindow;
    m_mediaView->setMainWindow(mainWindow);
    MediaItemDelegate * mediaItemDelegate = (MediaItemDelegate *)m_mediaView->itemDelegate();
    connect(mediaItemDelegate, SIGNAL(categoryActivated(QModelIndex)), this, SLOT(categoryActivated(QModelIndex)));
}

void InfoBox::setInfo(const QString &title, const QString & lri)
{
    m_title->setText(title);
    if (lri.startsWith("semantics://frequent")) {
        m_mediaView->setMode(MediaItemDelegate::MiniPlayCountMode);
    } else if (lri.startsWith("semantics://recent")) {
        m_mediaView->setMode(MediaItemDelegate::MiniPlaybackTimeMode);
    } else if (lri.startsWith("semantics://highest")) {
        m_mediaView->setMode(MediaItemDelegate::MiniRatingMode);
    }else if (lri.startsWith("music://albums")) {
        m_mediaView->setMode(MediaItemDelegate::MiniAlbumMode);
    } else {
        m_mediaView->setMode(MediaItemDelegate::MiniMode);
    }
    MediaItemModel * model = (MediaItemModel *)m_mediaView->sourceModel();
    hide();
    model->loadLRI(lri);
}

void InfoBox::setTitle(const QString &title)
{
    m_title->setText(title);
}

void InfoBox::updateTitleColors()
{
    m_titleBar->setPalette(QApplication::palette());
}

void InfoBox::mediaListChanged()
{
    //Set title bar icon based on content of mediaview
    MediaItemModel * model = (MediaItemModel *)m_mediaView->sourceModel();
    if (model->rowCount() > 0) {
        if (model->rowCount() == 1) {
            if (model->mediaItemAt(0).type == "Message" && m_mediaView->mode() != MediaItemDelegate::NormalMode) {
                hide();
                return;
            }
        }
        show();
        QPixmap pixmap = model->mediaItemAt(0).artwork.pixmap(16,16);
        m_icon->setPixmap(pixmap);
    }
    
    //Set widget height to contents
    MediaItemDelegate * mediaViewDelegate = (MediaItemDelegate *)m_mediaView->itemDelegate();
    int mediaViewHeight = mediaViewDelegate->heightForAllRows();
    setMaximumHeight(mediaViewHeight + m_titleBar->height());
    
}

void InfoBox::categoryActivated(QModelIndex index)
{
    MediaItemModel *model = (MediaItemModel *)m_mediaView->sourceModel();
    MediaItem categoryMediaItem = model->mediaItemAt(index.row());
    MediaListProperties mediaListProperties = MediaListProperties(categoryMediaItem.url);
    mediaListProperties.name = categoryMediaItem.title;
    mediaListProperties.category = categoryMediaItem;
    m_mainWindow->addListToHistory();
    BangarangApplication * application = (BangarangApplication *)KApplication::kApplication();
    application->browsingModel()->clearMediaListData();
    application->browsingModel()->setMediaListProperties(mediaListProperties);
    application->browsingModel()->load();
}
