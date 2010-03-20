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

#include "infomanager.h"
#include "infoitemdelegate.h"
#include "infocategorydelegate.h"
#include "platform/utilities.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "platform/mediaitemmodel.h"
#include "platform/infoitemmodel.h"
#include "platform/infocategorymodel.h"
#include "platform/playlist.h"
#include "mediaitemdelegate.h"
#include <KUrlRequester>
#include <KLineEdit>
#include <KGlobalSettings>
#include <KDebug>
#include <QDateEdit>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <Nepomuk/ResourceManager>
#include <nepomuk/variant.h>
#include <QComboBox>
#include <QSpinBox>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

InfoManager::InfoManager(MainWindow * parent) : QObject(parent)
{
    m_parent = parent;
    ui = m_parent->ui;

    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true;
    } else {
        m_nepomukInited = false;
    }
    
    m_infoItemModel = new InfoItemModel(this);
    m_infoItemModel->setSourceModel(m_parent->m_mediaItemModel);
    ui->infoItemView->setModel(m_infoItemModel);
    InfoItemDelegate * infoItemDelegate = new InfoItemDelegate(m_parent);
    infoItemDelegate->setView(ui->infoItemView);
    ui->infoItemView->setItemDelegate(infoItemDelegate);
    ui->infoSaveHolder->setVisible(false);
    ui->infoCategoryRecentlyPlayedTitle->setFont(KGlobalSettings::smallestReadableFont());
    ui->infoCategoryHighestRatedTitle->setFont(KGlobalSettings::smallestReadableFont());
    ui->infoCategoryFrequentlyPlayedTitle->setFont(KGlobalSettings::smallestReadableFont());
    
    //Set up Category View
    m_infoCategoryModel = new InfoCategoryModel(this);
    m_infoCategoryDelegate = new InfoCategoryDelegate(m_parent);
    ui->infoCategoryView->setModel(m_infoCategoryModel);
    m_infoCategoryDelegate->setView(ui->infoCategoryView);
    ui->infoCategoryView->setItemDelegate(m_infoCategoryDelegate);
    
    //Set up Recently Played Info box
    ui->infoCategoryRecentlyPlayed->setMainWindow(m_parent);
    m_recentlyPlayedModel = (MediaItemModel *)ui->infoCategoryRecentlyPlayed->model();
    ui->infoCategoryRecentlyPlayed->setMode(MediaView::MiniPlaybackTimeMode);

    //Set up Recently Played Info box
    ui->infoCategoryHighestRated->setMainWindow(m_parent);
    m_highestRatedModel = (MediaItemModel *)ui->infoCategoryHighestRated->model();
    ui->infoCategoryHighestRated->setMode(MediaView::MiniRatingMode);

    //Set up Frequently Played Info box
    ui->infoCategoryFrequentlyPlayed->setMainWindow(m_parent);
    m_frequentlyPlayedModel = (MediaItemModel *)ui->infoCategoryFrequentlyPlayed->model();
    ui->infoCategoryFrequentlyPlayed->setMode(MediaView::MiniPlayCountMode);

    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(toggleInfoView()));
    connect(m_infoItemModel, SIGNAL(infoChanged(bool)), ui->infoSaveHolder, SLOT(setVisible(bool)));
    connect(ui->infoItemCancelEdit, SIGNAL(clicked()), this, SLOT(cancelItemEdit()));
    connect(ui->infoItemSave, SIGNAL(clicked()), this, SLOT(saveItemInfo()));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_infoItemModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)), this, SLOT(infoDataChangedSlot(const QModelIndex, const QModelIndex)));
    connect(m_infoCategoryModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)), this, SLOT(infoDataChangedSlot(const QModelIndex, const QModelIndex)));
    connect(m_infoCategoryModel, SIGNAL(modelDataChanged()), this, SLOT(updateViewsLayout()));
}

InfoManager::~InfoManager()
{
}


//---------------------
//-- UI Widget Slots --
//---------------------
void InfoManager::toggleInfoView()
{
    bool makeVisible = !ui->semanticsHolder->isVisible();
    ui->semanticsHolder->setVisible(makeVisible);
    
    if (makeVisible) {
        loadSelectedInfo();
        ui->showInfo->setToolTip(i18n("<b>Showing Information</b><br>Click to hide information."));
        ui->showInfo->setIcon(Utilities::turnIconOff(KIcon("help-about"), QSize(22, 22)));
    } else {
        ui->showInfo->setToolTip(i18n("Show Information"));
        ui->showInfo->setIcon(KIcon("help-about"));
    } 
}

void InfoManager::showInfoView()
{
    ui->semanticsHolder->setVisible(true);
    loadSelectedInfo();
}

void InfoManager::hideInfoView()
{
    ui->semanticsHolder->setVisible(false);
}

void InfoManager::mediaSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected )
{
    //Conditionally show the showInfo button
    //NOTE: This will eventually be whittled down to nothing as we add more info views
    //to the semantics stack.
    if (selected.indexes().count() > 0) {
        int row = selected.indexes().at(0).row();
        MediaItem firstItem = m_parent->m_mediaItemModel->mediaItemAt(row);
        QString listItemType = firstItem.type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
            if (!firstItem.url.startsWith("DVDTRACK") && !firstItem.url.startsWith("CDTRACK")) {
                ui->showInfo->setVisible(true);
            } else {
                ui->showInfo->setVisible(false);
            }
        } else if (listItemType == "Category") {
            if (firstItem.fields["categoryType"].toString() == "Artist" ||
                firstItem.fields["categoryType"].toString() == "Album" ||
                firstItem.fields["categoryType"].toString() == "MusicGenre" ||
                firstItem.fields["categoryType"].toString() == "AudioTag" ||
                firstItem.fields["categoryType"].toString() == "TVShow" ||
                firstItem.fields["categoryType"].toString() == "VideoGenre") {
                ui->showInfo->setVisible(true);
            } else {
                ui->showInfo->setVisible(false);
            }
        } else {
            ui->showInfo->setVisible(false);
        }
    } else {
        ui->showInfo->setVisible(false);
    }
    
    //Load info
    if (ui->semanticsHolder->isVisible()) {
        loadSelectedInfo();
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void InfoManager::saveItemInfo()
{
    //Save changed info in model
    m_infoItemModel->saveChanges();
    updateViewsLayout();
    
    //Update Now Playing and Playlist views
    m_parent->playlist()->nowPlayingModel()->updateMediaItems(m_infoItemModel->mediaList());
    m_parent->playlist()->playlistModel()->updateMediaItems(m_infoItemModel->mediaList());
    
    //Now that data is saved hide Save/Cancel controls
    ui->infoSaveHolder->setVisible(false);
}

void InfoManager::cancelItemEdit()
{
    m_infoItemModel->cancelChanges();
    ui->infoSaveHolder->setVisible(false);
    updateViewsLayout();
}

void InfoManager::removeSelectedItemsInfo()
{
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        MediaItem mediaItem = m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row());
        if (mediaItem.type == "Audio" || mediaItem.type == "Video" || mediaItem.type == "Image") {
            mediaList.append(mediaItem);
        }
    }
    if (mediaList.count() > 0) {
        m_parent->m_mediaItemModel->removeSourceInfo(mediaList);
    }
            
}

void InfoManager::showInfoViewForMediaItem(const MediaItem &mediaItem)
{
    ui->stackedWidget->setCurrentIndex(0);
    if (!ui->semanticsHolder->isVisible()) {
        ui->semanticsHolder->setVisible(true);
        ui->showInfo->setToolTip(i18n("<b>Showing Information</b><br>Click to hide information."));
        ui->showInfo->setIcon(Utilities::turnIconOff(KIcon("help-about"), QSize(22, 22)));
    }
    ui->semanticsStack->setCurrentIndex(1);
    
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    if (mediaList.count() == 0) {
        return;
    }
    m_infoItemModel->loadInfo(mediaList);
    updateViewsLayout();
}



//----------------------
//-- Helper functions --
//----------------------
void InfoManager::loadSelectedInfo()
{
    //Show the Info Item page
    //NOTE:For each selection type a page will be added to the semantics stack
    //ui->semanticsStack->setCurrentIndex(1);
    
    //Get selected items
    QList<MediaItem> mediaList;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    for (int i = 0 ; i < selectedRows.count() ; ++i) {
        mediaList.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
    }
    if (mediaList.count() == 0) {
        return;
    }
    
    //Load selected items into info model
    if (mediaList.at(0).type == "Audio" || mediaList.at(0).type == "Video") {
        m_infoItemModel->loadInfo(mediaList);
        ui->semanticsStack->setCurrentIndex(1);
        updateViewsLayout();
    } else if (mediaList.at(0).type == "Category") {
        m_infoCategoryModel->setSourceModel(m_parent->m_mediaItemModel);
        
        if (mediaList.at(0).fields["categoryType"].toString() == "Artist") {
            m_infoCategoryModel->setMode(InfoCategoryModel::ArtistMode);
            m_currentCategory = "Artist";
            m_infoCategoryModel->loadInfo(mediaList);
            ui->semanticsStack->setCurrentIndex(0);
            
            //NOTE:This following is present here for development and debugging purposes only.
            // The expected workflow is to display info contained in the nepomuk and
            // allow use of downloadInfo to populate nepomuk. We could provide
            // automatic display of downloaded info as an option...
            m_infoCategoryModel->downloadInfo();
            
            QString recentlyPlayedLRI = QString("semantics://recent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "artist", "=");
            m_recentlyPlayedModel->loadLRI(recentlyPlayedLRI);
            
            QString highestRatedLRI = QString("semantics://highest?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "artist", "=");
            m_highestRatedModel->loadLRI(highestRatedLRI);

            QString frequentlyPlayedLRI = QString("semantics://frequent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "artist", "=");
            m_frequentlyPlayedModel->loadLRI(frequentlyPlayedLRI);
            
            updateViewsLayout();
        } else if (mediaList.at(0).fields["categoryType"].toString() == "Album") {
            m_infoCategoryModel->setMode(InfoCategoryModel::AlbumMode);
            m_currentCategory = "Album";
            m_infoCategoryModel->loadInfo(mediaList);
            ui->semanticsStack->setCurrentIndex(0);
            
            QString recentlyPlayedLRI = QString("semantics://recent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "album", "=");
            m_recentlyPlayedModel->loadLRI(recentlyPlayedLRI);
            
            QString highestRatedLRI = QString("semantics://highest?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "album", "=");
            m_highestRatedModel->loadLRI(highestRatedLRI);

            QString frequentlyPlayedLRI = QString("semantics://frequent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "album", "=");
            m_frequentlyPlayedModel->loadLRI(frequentlyPlayedLRI);
            
            updateViewsLayout();
        } else if (mediaList.at(0).fields["categoryType"].toString() == "MusicGenre") {
            m_infoCategoryModel->setMode(InfoCategoryModel::MusicGenreMode);
            m_currentCategory = "MusicGenre";
            m_infoCategoryModel->loadInfo(mediaList);
            ui->semanticsStack->setCurrentIndex(0);
            
            QString recentlyPlayedLRI = QString("semantics://recent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "genre", "=");
            m_recentlyPlayedModel->loadLRI(recentlyPlayedLRI);
            
            QString highestRatedLRI = QString("semantics://highest?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "genre", "=");
            m_highestRatedModel->loadLRI(highestRatedLRI);
            
            QString frequentlyPlayedLRI = QString("semantics://frequent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "genre", "=");
            m_frequentlyPlayedModel->loadLRI(frequentlyPlayedLRI);
            
            updateViewsLayout();
        } else if (mediaList.at(0).fields["categoryType"].toString() == "AudioTag") {
            m_infoCategoryModel->setMode(InfoCategoryModel::AudioTagMode);
            m_currentCategory = "AudioTag";
            m_infoCategoryModel->loadInfo(mediaList);
            ui->semanticsStack->setCurrentIndex(0);
            
            QString recentlyPlayedLRI = QString("semantics://recent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "tag", "=");
            m_recentlyPlayedModel->loadLRI(recentlyPlayedLRI);
            
            QString highestRatedLRI = QString("semantics://highest?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "tag", "=");
            m_highestRatedModel->loadLRI(highestRatedLRI);
            
            QString frequentlyPlayedLRI = QString("semantics://frequent?audio||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "tag", "=");
            m_frequentlyPlayedModel->loadLRI(frequentlyPlayedLRI);
            
            updateViewsLayout();
        } else if (mediaList.at(0).fields["categoryType"].toString() == "TVShow") {
            m_infoCategoryModel->setMode(InfoCategoryModel::TVShowMode);
            m_currentCategory = "TVShow";
            m_infoCategoryModel->loadInfo(mediaList);
            ui->semanticsStack->setCurrentIndex(0);
            
            QString recentlyPlayedLRI = QString("semantics://recent?video||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "seriesName", "=");
            m_recentlyPlayedModel->loadLRI(recentlyPlayedLRI);
            
            QString highestRatedLRI = QString("semantics://highest?video||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "seriesName", "=");
            m_highestRatedModel->loadLRI(highestRatedLRI);
            
            QString frequentlyPlayedLRI = QString("semantics://frequent?video||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "seriesName", "=");
            m_frequentlyPlayedModel->loadLRI(frequentlyPlayedLRI);
            
            updateViewsLayout();
        } else if (mediaList.at(0).fields["categoryType"].toString() == "VideoGenre") {
            m_infoCategoryModel->setMode(InfoCategoryModel::VideoGenreMode);
            m_currentCategory = "VideoGenre";
            m_infoCategoryModel->loadInfo(mediaList);
            ui->semanticsStack->setCurrentIndex(0);
            
            QString recentlyPlayedLRI = QString("semantics://recent?video||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "genre", "=");
            m_recentlyPlayedModel->loadLRI(recentlyPlayedLRI);
            
            QString highestRatedLRI = QString("semantics://highest?video||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "genre", "=");
            m_highestRatedModel->loadLRI(highestRatedLRI);
            
            QString frequentlyPlayedLRI = QString("semantics://frequent?video||limit=5") + Utilities::lriFilterFromMediaListField(mediaList, "title", "genre", "=");
            m_frequentlyPlayedModel->loadLRI(frequentlyPlayedLRI);
            
            updateViewsLayout();
        }
    }
}

void InfoManager::updateViewsLayout()
{
    //Update infoItemView
    for (int row = 0; row < m_infoItemModel->rowCount(); row++) {
        QString field = m_infoItemModel->item(row)->data(InfoItemModel::FieldRole).toString();
        if (field == "artwork" || field == "title") {
            ui->infoItemView->setSpan(row,0,1,2);
        }
    }
    ui->infoItemView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->infoItemView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    
    //Update infoCategoryView
    ui->infoCategoryView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->infoCategoryView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
    ui->infoCategoryView->setMinimumHeight(m_infoCategoryDelegate->heightForAllRows());
    ui->infoCategoryView->setMaximumHeight(m_infoCategoryDelegate->heightForAllRows());
    
}

void InfoManager::infoDataChangedSlot(const QModelIndex &topleft, const QModelIndex &bottomright)
{
    updateViewsLayout();
    Q_UNUSED(topleft);
    Q_UNUSED(bottomright);
}
