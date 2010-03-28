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
#include "infobox.h"
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
    
    //Set up Category View
    m_infoCategoryModel = new InfoCategoryModel(this);
    m_infoCategoryDelegate = new InfoCategoryDelegate(m_parent);
    ui->infoCategoryView->setModel(m_infoCategoryModel);
    m_infoCategoryDelegate->setView(ui->infoCategoryView);
    ui->infoCategoryView->setItemDelegate(m_infoCategoryDelegate);
    
    connect(ui->showInfo, SIGNAL(clicked()), this, SLOT(toggleInfoView()));
    connect(m_infoItemModel, SIGNAL(infoChanged(bool)), ui->infoSaveHolder, SLOT(setVisible(bool)));
    connect(ui->infoItemCancelEdit, SIGNAL(clicked()), this, SLOT(cancelItemEdit()));
    connect(ui->infoItemSave, SIGNAL(clicked()), this, SLOT(saveItemInfo()));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_infoItemModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)), this, SLOT(infoDataChangedSlot(const QModelIndex, const QModelIndex)));
    connect(m_infoCategoryModel, SIGNAL(dataChanged(const QModelIndex, const QModelIndex)), this, SLOT(infoDataChangedSlot(const QModelIndex, const QModelIndex)));
    connect(m_infoCategoryModel, SIGNAL(modelDataChanged()), this, SLOT(updateViewsLayout()));
    connect(m_parent->m_mediaItemModel, SIGNAL(mediaListChanged()), this, SLOT(showOrHideInfoButton()));
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
    showOrHideInfoButton();
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void InfoManager::showOrHideInfoButton()
{
    //Conditionally show the showInfo button
    //NOTE: This will eventually be whittled down to nothing as we add more info views
    //to the semantics stack.
    
    MediaItem firstItem;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    if (selectedRows.count() > 0) {
        //Look at first item in selection
        firstItem = m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(0).row());
    } else if (m_parent->m_mediaItemModel->rowCount() > 0) {
        //Look at current context category
        firstItem = m_parent->m_mediaItemModel->mediaListProperties().category;
    } else {
        //If nothing is selected and there's nothing in the media list then nothing to do
        ui->showInfo->setVisible(false);
        return;
    }
    
    QString listItemType = firstItem.type;
    if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Image")) {
        if (!firstItem.url.startsWith("DVDTRACK") && !firstItem.url.startsWith("CDTRACK")) {
            ui->showInfo->setVisible(true);
        } else {
            ui->showInfo->setVisible(false);
        }
    } else if (listItemType == "Category") {
        if (firstItem.fields["contextLRIs"].toStringList().count() > 0) {
            ui->showInfo->setVisible(true);
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

void InfoManager::setContext(const MediaItem &category)
{
    m_contextCategory = category;
}


//----------------------
//-- Helper functions --
//----------------------
void InfoManager::loadSelectedInfo()
{
    //Show the Info Item page
    //NOTE:For each selection type a page will be added to the semantics stack
    //ui->semanticsStack->setCurrentIndex(1);
    
    //Determine type of items in mediaview
    bool mediaViewHasMedia = false;
    if (m_parent->m_mediaItemModel->rowCount()>0) {
        if (m_parent->m_mediaItemModel->mediaItemAt(0).type == "Audio" || m_parent->m_mediaItemModel->mediaItemAt(0).type == "Video") {
            mediaViewHasMedia = true;
        } else {
            mediaViewHasMedia = false;
        }
    }
    
    //Determine Information context
    bool selected = true;
    QList<MediaItem> context;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    if (selectedRows.count() > 0) {
        //If items are selected then the context is the selected items
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            context.append(m_parent->m_mediaItemModel->mediaItemAt(selectedRows.at(i).row()));
        }
    } else if (m_parent->m_mediaItemModel->rowCount()>0) {
        //If nothing is selected then the information context is 
        //the category selected to produce the list of media in the mediaview
        selected = false;
        context.append(m_parent->m_mediaItemModel->mediaListProperties().category);
    } else {
        return;
    }
    
    //Determine type of context data
    bool contextIsMedia = false;
    if (context.at(0).type == "Audio" || context.at(0).type == "Video") {
        contextIsMedia = true;
    }
    bool contextIsCategory = false;
    if (context.at(0).type == "Category") {
        contextIsCategory = true;
    }

    //Load contextual data into info model and info boxes
    if (selected && contextIsMedia) {
        m_infoItemModel->loadInfo(context);
        ui->semanticsStack->setCurrentIndex(1);
        updateViewsLayout();
    } else if (contextIsCategory) {
        m_infoCategoryModel->loadInfo(context);
        ui->semanticsStack->setCurrentIndex(0);
        m_infoCategoryModel->setSourceModel(m_parent->m_mediaItemModel);
        
        m_infoCategoryModel->downloadInfo();
        
        //Load any context infoboxes
        MediaItem contextCategory = context.at(0);
        QStringList contextTitles = contextCategory.fields["contextTitles"].toStringList();
        QStringList contextLRIs = contextCategory.fields["contextLRIs"].toStringList();
        for (int i = 0; i < contextLRIs.count(); i++) {
            QString title = contextTitles.at(i);
            QString lri = contextLRIs.at(i);
            if (i < ui->infoBoxHolder->layout()->count()) {
                InfoBox * infoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(i)->widget();
                infoBox->setInfo(title, lri);
            } else {
                InfoBox *infoBox = new InfoBox;
                infoBox->setMainWindow(m_parent);
                infoBox->setInfo(title, lri);
                ui->infoBoxHolder->layout()->addWidget(infoBox);
            }
        }
        //Remove any unused infoboxes
        int totalInfoBoxes = ui->infoBoxHolder->layout()->count();
        if (contextLRIs.count() < totalInfoBoxes) {
            for (int i = contextLRIs.count(); i < totalInfoBoxes; i++) {
                int lastItemIndex = ui->infoBoxHolder->layout()->count() - 1;
                QWidget * unusedInfoBox = ui->infoBoxHolder->layout()->itemAt(lastItemIndex)->widget(); 
                ui->infoBoxHolder->layout()->removeWidget(unusedInfoBox);
                delete unusedInfoBox;
            }
        }
        
        updateViewsLayout();
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
    if (m_infoCategoryModel->rowCount()> 0) {
        ui->infoCategoryView->setVisible(true);
        ui->infoCategoryView->verticalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        ui->infoCategoryView->horizontalHeader()->setResizeMode(QHeaderView::ResizeToContents);
        ui->infoCategoryView->setMinimumHeight(m_infoCategoryDelegate->heightForAllRows());
        ui->infoCategoryView->setMaximumHeight(m_infoCategoryDelegate->heightForAllRows());
    } else {
        ui->infoCategoryView->setVisible(false);
    }
}

void InfoManager::infoDataChangedSlot(const QModelIndex &topleft, const QModelIndex &bottomright)
{
    updateViewsLayout();
    Q_UNUSED(topleft);
    Q_UNUSED(bottomright);
}
