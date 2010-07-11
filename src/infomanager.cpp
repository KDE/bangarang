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
#include "bangarangapplication.h"
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
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_parent = parent;
    ui = m_parent->ui;

    Nepomuk::ResourceManager::instance()->init();
    if (Nepomuk::ResourceManager::instance()->initialized()) {
        m_nepomukInited = true;
    } else {
        m_nepomukInited = false;
    }
    
    m_infoItemModel = new InfoItemModel(this);
    m_infoItemModel->setSourceModel(m_application->browsingModel());
    ui->infoItemView->setModel(m_infoItemModel);
    InfoItemDelegate * infoItemDelegate = new InfoItemDelegate(m_parent);
    infoItemDelegate->setView(ui->infoItemView);
    ui->infoItemView->setItemDelegate(infoItemDelegate);
    ui->infoSaveHolder->setVisible(false);
    
    //Set up selection timer
    m_selectionTimer = new QTimer(this);
    m_selectionTimer->setSingleShot(true);
    connect(m_selectionTimer, SIGNAL(timeout()), this, SLOT(loadSelectedInfo()));
    
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
    connect(m_infoCategoryModel, SIGNAL(infoChanged(bool)), ui->infoSaveHolder, SLOT(setVisible(bool)));
    connect(m_application->browsingModel(), SIGNAL(mediaListChanged()), this, SLOT(loadSelectedInfo()));
    connect(m_application->browsingModel(), SIGNAL(mediaListPropertiesChanged()), this, SLOT(mediaListPropertiesChanged()));
    connect(ui->collectionButton, SIGNAL(clicked()), this, SLOT(updateViewsLayout()));
    
}

InfoManager::~InfoManager()
{
}


//---------------------
//-- UI Widget Slots --
//---------------------
void InfoManager::toggleInfoView()
{
    bool makeVisible = !m_infoViewVisible;
    ui->semanticsHolder->setVisible(makeVisible);
    
    if (makeVisible) {
        m_infoViewVisible = true;
        loadSelectedInfo();
        ui->showInfo->setToolTip(i18n("<b>Showing Information</b><br>Click to hide information."));
        ui->showInfo->setIcon(Utilities::turnIconOff(KIcon("help-about"), QSize(22, 22)));
    } else {
        m_infoViewVisible = false;
        ui->showInfo->setToolTip(i18n("Show Information"));
        ui->showInfo->setIcon(KIcon("help-about"));
    } 
}

void InfoManager::showInfoView()
{
    ui->semanticsHolder->setVisible(true);
    loadSelectedInfo();
    m_infoViewVisible = true;
}

void InfoManager::hideInfoView()
{
    ui->semanticsHolder->setVisible(false);
    m_infoViewVisible = false;
}

bool InfoManager::infoViewVisible()
{
    return m_infoViewVisible;
}

void InfoManager::mediaSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected )
{
    //Delay updating info for 400 milliseconds to prevent rapid-fire nepomuk queries.
    m_selectionTimer->start(400);
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void InfoManager::saveItemInfo()
{
    if (ui->semanticsStack->currentIndex() == 0) {
        //Save changed category info in model
        m_infoCategoryModel->saveChanges();
    } else {
        //Save changed item info in model
        m_infoItemModel->saveChanges();
        
        //Update Now Playing and Playlist views
        m_application->playlist()->nowPlayingModel()->updateMediaItems(m_infoItemModel->mediaList());
        m_application->playlist()->playlistModel()->updateMediaItems(m_infoItemModel->mediaList());
    }    
    
    //Now that data is saved hide Save/Cancel controls
    ui->infoSaveHolder->setVisible(false);
    updateViewsLayout();
}

void InfoManager::cancelItemEdit()
{
    if (ui->semanticsStack->currentIndex() == 0) {
        m_infoCategoryModel->cancelChanges();
    } else {
        m_infoItemModel->cancelChanges();
    }
    ui->infoSaveHolder->setVisible(false);
    updateViewsLayout();
}

void InfoManager::showInfoViewForMediaItem(const MediaItem &mediaItem)
{
    m_parent->switchMainWidget(MainWindow::MainMediaList);
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

void InfoManager::removeSelectedItemsInfo()
{
    QList<MediaItem> selectedItems;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    if (selectedRows.count() > 0) {
        //If items are selected then the context is the selected items
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            selectedItems.append(m_application->browsingModel()->mediaItemAt(selectedRows.at(i).row()));
        }
    }
    if (selectedItems.count() > 0) {
        m_application->browsingModel()->removeSourceInfo(selectedItems);
    }
}

void InfoManager::addSelectedItemsInfo()
{
    QList<MediaItem> selectedItems;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    if (selectedRows.count() > 0) {
        //If items are selected then the context is the selected items
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            selectedItems.append(m_application->browsingModel()->mediaItemAt(selectedRows.at(i).row()));
        }
    }
    if (selectedItems.count() > 0) {
        m_application->browsingModel()->updateSourceInfo(selectedItems);
    }
}

//----------------------
//-- Helper functions --
//----------------------
void InfoManager::loadSelectedInfo()
{

    bool templateIsSelected = false;
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    MediaSortFilterProxyModel *proxy = (MediaSortFilterProxyModel *) ui->mediaView->model();
    if (selectedRows.count() == 1) {
        int row = proxy->mapToSource(selectedRows.at(0)).row();
        MediaItem mediaItem = m_application->browsingModel()->mediaItemAt(row);
        templateIsSelected = mediaItem.fields["isTemplate"].toBool();
    }
    
    //Make sure info view is visble before doing anything
    if (!m_infoViewVisible && !templateIsSelected) {
        return;
    }
    
    //Automatically show info view if template is selected
    if (templateIsSelected) {
        ui->semanticsHolder->setVisible(true);
        m_infoViewVisible = true;
    }
    
    m_selectedInfoBoxMediaItems.clear();
    emit infoBoxSelectionChanged(m_selectedInfoBoxMediaItems);

    //Determine type of items in mediaview
    bool mediaViewHasMedia = false;
    if (m_application->browsingModel()->rowCount()>0) {
        if (m_application->browsingModel()->mediaItemAt(0).type == "Audio" || m_application->browsingModel()->mediaItemAt(0).type == "Video") {
            mediaViewHasMedia = true;
        } else {
            mediaViewHasMedia = false;
        }
    }
    
    //Determine Information context
    bool selected = true;
    QList<MediaItem> context;
    if (selectedRows.count() > 0) {
        //If items are selected then the context is the selected items
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            int row = proxy->mapToSource(selectedRows.at(i)).row();
            context.append(m_application->browsingModel()->mediaItemAt(row));
        }
    } else if (m_application->browsingModel()->rowCount()>0) {
        //If nothing is selected then the information context is 
        //the category selected to produce the list of media in the mediaview
        selected = false;
        context.append(m_application->browsingModel()->mediaListProperties().category);
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
        m_infoCategoryModel->setSourceModel(m_application->browsingModel());
        
        //Get context data for info boxes
        QStringList contextLRIs;
        QStringList contextTitles;
        for (int i = 0; i < context.count(); i++) {
            MediaItem contextCategory = context.at(i);
            if (i == 0) {
                contextTitles = contextCategory.fields["contextTitles"].toStringList();
                contextLRIs = contextCategory.fields["contextLRIs"].toStringList();
                continue;
            }
            QStringList currentContextTitles = contextCategory.fields["contextTitles"].toStringList();
            QStringList currentContextLRIs = contextCategory.fields["contextLRIs"].toStringList();
            QStringList mergedContextLRIs;
            QStringList mergedContextTitles;
            for (int j = 0; j < currentContextLRIs.count(); j++) {
                if (j < contextLRIs.count() ) {
                    QString mergedContextLRI = Utilities::mergeLRIs(contextLRIs.at(j), currentContextLRIs.at(j));
                    if (!mergedContextLRI.isEmpty()) {
                        mergedContextLRIs.append(mergedContextLRI);
                        mergedContextTitles.append(currentContextTitles.at(j));
                    }
                }
            }
            contextTitles = mergedContextTitles;
            contextLRIs = mergedContextLRIs;
        }

        //Load any context infoboxes
        //NOTE:Currently infoboxes are only loaded for one category at a time.
        /*MediaItem contextCategory = context.at(0);
        QStringList contextTitles = contextCategory.fields["contextTitles"].toStringList();
        contextLRIs = contextCategory.fields["contextLRIs"].toStringList();*/
        
        int totalInfoBoxes = ui->infoBoxHolder->layout()->count() - 1;
        for (int i = 0; i < contextLRIs.count(); i++) {
            QString title = contextTitles.at(i);
            QString lri = contextLRIs.at(i);
            if (i < totalInfoBoxes) {
                InfoBox * infoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(i)->widget();
                MediaItemModel * infoBoxModel = (MediaItemModel *)infoBox->mediaView()->sourceModel();
                if (infoBoxModel->mediaListProperties().lri != lri) {
                    infoBox->setInfo(title, lri);
                } else {
                    infoBox->setTitle(title);
                }
            } else {
                int insertIndex = ui->infoBoxHolder->layout()->count() - 1;
                if (insertIndex >= 0) {
                    InfoBox *infoBox = new InfoBox;
                    infoBox->setMainWindow(m_parent);
                    infoBox->setInfo(title, lri);
                    QVBoxLayout * infoBoxHolderLayout = (QVBoxLayout *)ui->infoBoxHolder->layout();
                    infoBoxHolderLayout->insertWidget(insertIndex, infoBox);
                    connect(infoBox->mediaView()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(infoBoxSelectionChanged(const QItemSelection, const QItemSelection)));
                }
            }
        }
        //Remove any unused infoboxes
        totalInfoBoxes = ui->infoBoxHolder->layout()->count() - 1;
        if (contextLRIs.count() < totalInfoBoxes) {
            for (int i = contextLRIs.count(); i < totalInfoBoxes; i++) {
                int lastItemIndex = ui->infoBoxHolder->layout()->count() - 2;
                if (lastItemIndex >= 0) {
                    InfoBox * unusedInfoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(lastItemIndex)->widget(); 
                    disconnect(unusedInfoBox->mediaView()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(infoBoxSelectionChanged(const QItemSelection, const QItemSelection)));
                    ui->infoBoxHolder->layout()->removeWidget(unusedInfoBox);
                    delete unusedInfoBox;
                }
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
        if (field == "artwork" || field == "title" || field == "description") {
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
        int infoCategoryViewHeight = m_infoCategoryDelegate->heightForAllRows();
        ui->infoCategoryView->setMinimumHeight(infoCategoryViewHeight);
        ui->infoCategoryView->setMaximumHeight(infoCategoryViewHeight);
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

void InfoManager::infoBoxSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected)
{
    if (selected.indexes().count() > 0) {
        //Only allow one item in one infobox to be selected at a time.
        int totalInfoBoxes = ui->infoBoxHolder->layout()->count() - 1;
        for (int i = 0; i < totalInfoBoxes; i++) {
            InfoBox * infoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(i)->widget(); 
            if (infoBox->mediaView()->selectionModel()->selectedRows().count() > 0) {
                if (infoBox->mediaView()->selectionModel()->selection().indexes().at(0) != selected.indexes().at(0)) {
                    infoBox->mediaView()->selectionModel()->clearSelection();
                }
            }
        }
        
        //Store selected Media Item
        m_selectedInfoBoxMediaItems.clear();
        MediaSortFilterProxyModel * proxyModel = (MediaSortFilterProxyModel *)selected.indexes().at(0).model();
        MediaItemModel * model = (MediaItemModel *)proxyModel->sourceModel();
        int selectedRow = selected.indexes().at(0).row();
        m_selectedInfoBoxMediaItems.append(model->mediaItemAt(selectedRow));
        emit infoBoxSelectionChanged(m_selectedInfoBoxMediaItems);
        
        //Show "Play Selected" button
        ui->playAll->setVisible(false);
        ui->playSelected->setVisible(true);
    } else {
        //Check to see if other infoboxes has something selected;
        //this handles deselection of infobox mediaItems
        bool selected = false;
        int totalInfoBoxes = ui->infoBoxHolder->layout()->count() - 1;
        for (int i = 0; i < totalInfoBoxes; i++) {
            InfoBox * infoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(i)->widget(); 
            if (infoBox->mediaView()->selectionModel()->selectedRows().count() > 0) {
                selected = true;
                break;
            }
        }
        if (!selected) {
            m_selectedInfoBoxMediaItems.clear();
            emit infoBoxSelectionChanged(m_selectedInfoBoxMediaItems);
        }
    }
                
    Q_UNUSED(deselected);
}

const QList<MediaItem> InfoManager::selectedInfoBoxMediaItems()
{
    return m_selectedInfoBoxMediaItems;
}

void InfoManager::clearInfoBoxSelection()
{
    int totalInfoBoxes = ui->infoBoxHolder->layout()->count() - 1;
    for (int i = 0; i < totalInfoBoxes; i++) {
        InfoBox * infoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(i)->widget(); 
        if (infoBox->mediaView()->selectionModel()->selectedRows().count() > 0) {
            infoBox->mediaView()->selectionModel()->clearSelection();
        }
    }
    m_selectedInfoBoxMediaItems.clear();
    emit infoBoxSelectionChanged(m_selectedInfoBoxMediaItems);
}

void InfoManager::mediaListPropertiesChanged()
{
    QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
    if (selectedRows.count() == 0) {
        loadSelectedInfo();
    }
}
