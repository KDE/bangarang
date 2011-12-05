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
#include "medialistsmanager.h"
#include "../common/bangarangapplication.h"
#include "infoitemdelegate.h"
#include "infobox.h"
#include "../common/mainwindow.h"
#include "../common/flickcharm.h"
#include "ui_mainwindow.h"
#include "../common/mediaitemdelegate.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/infoitemmodel.h"
#include "../../platform/playlist.h"
#include "../../platform/utilities/utilities.h"
#include "../../platform/infofetchers/infofetcher.h"
#include "../../platform/infofetchers/dbpediainfofetcher.h"
#include <KUrlRequester>
#include <KLineEdit>
#include <KGlobalSettings>
#include <KAction>
#include <KDebug>
#include <Soprano/QueryResultIterator>
#include <Soprano/Vocabulary/Xesam>
#include <Soprano/Vocabulary/RDF>
#include <Soprano/Vocabulary/XMLSchema>
#include <nepomuk/resource.h>
#include <Nepomuk/ResourceManager>
#include <nepomuk/variant.h>
#include <QComboBox>
#include <QDateEdit>
#include <QDesktopServices>
#include <QSpinBox>
#include <taglib/fileref.h>
#include <taglib/tstring.h>
#include <taglib/id3v2tag.h>

InfoManager::InfoManager(MainWindow * parent) : QObject(parent)
{
    m_application = (BangarangApplication *)KApplication::kApplication();
    m_parent = parent;
    ui = m_parent->ui;

    m_nepomukInited = Utilities::nepomukInited();
    m_enableTouch = false;
    
    m_infoItemModel = (InfoItemModel *)ui->infoItemView->model();
    m_infoItemModel->setSourceModel(m_application->browsingModel());
    ui->infoSaveHolder->setVisible(false);
    ui->infoIndexerHolder->setVisible(false);
    ui->infoFetcherHolder->setVisible(false);
    ui->infoFetcherExpander->setVisible(false);
    ui->infoFetcherExpander->setCurrentIndex(0);
    ui->infoFetcherLink->setVisible(false);
    QFont fetcherMessageFont = KGlobalSettings::smallestReadableFont();
    ui->infoFetcherLabel->setFont(fetcherMessageFont);
    ui->infoFetcherSelector->setFont(fetcherMessageFont);
    ui->infoFetch->setFont(fetcherMessageFont);
    ui->infoAutoFetch->setFont(fetcherMessageFont);
    
    //Set up selection timer
    m_selectionTimer = new QTimer(this);
    m_selectionTimer->setSingleShot(true);
    connect(m_selectionTimer, SIGNAL(timeout()), this, SLOT(loadSelectedInfo()));
    
    //Set up info fetching
    m_currentInfoFetcher = 0;
    connect(ui->infoAutoFetch, SIGNAL(clicked()), this, SLOT(autoFetchInfo()));
    connect(ui->infoFetch, SIGNAL(clicked()), this, SLOT(fetchInfo()));
    connect(ui->showInfoFetcherExpander, SIGNAL(clicked()), this, SLOT(toggleShowInfoFetcherExpander()));
    connect(ui->infoFetcherSelector, SIGNAL(currentIndexChanged(int)), this, SLOT(selectInfoFetcher(int)));
    connect(ui->infoFetcherLink, SIGNAL(clicked()), this, SLOT(openInfoFetcherLink()));
    connect(m_infoItemModel, SIGNAL(fetching()), this, SLOT(showFetching()));
    connect(m_infoItemModel, SIGNAL(fetchingStatusUpdated()), this, SLOT(fetchingStatusUpdated()));
    connect(m_infoItemModel, SIGNAL(fetchComplete()), this, SLOT(fetchComplete()));
    connect(ui->fetchedMatches, SIGNAL(currentRowChanged(int)), m_infoItemModel, SLOT(selectFetchedMatch(int)));

    connect(m_infoItemModel, SIGNAL(infoChanged(bool)), this, SLOT(infoChanged(bool)));
    connect(ui->infoItemCancelEdit, SIGNAL(clicked()), this, SLOT(cancelItemEdit()));
    connect(ui->infoItemSave, SIGNAL(clicked()), this, SLOT(saveItemInfo()));
    connect(ui->infoIndexSelected, SIGNAL(clicked()), this, SLOT(addSelectedItemsInfo()));
    connect(ui->mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_application->browsingModel(), SIGNAL(mediaListChanged()), this, SLOT(loadSelectedInfo()));
    connect(m_application->browsingModel(), SIGNAL(mediaListPropertiesChanged()), this, SLOT(mediaListPropertiesChanged()));

    m_infoViewVisible = ui->semanticsHolder->isVisible();
}

InfoManager::~InfoManager()
{
}


//---------------------
//-- UI Widget Slots --
//---------------------
void InfoManager::toggleInfoView(bool force)
{
    if ((m_application->mainWindow()->currentMainWidget() != MainWindow::MainMediaList) &&
        !force) {
        return;
    }
    bool makeVisible = !m_infoViewVisible;
    ui->semanticsHolder->setVisible(makeVisible);
    
    if (makeVisible) {
        m_infoViewVisible = true;
        loadSelectedInfo();
    } else {
        m_infoViewVisible = false;
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
    //Save changed item info in model
    m_infoItemModel->saveChanges();

    //Update Now Playing and Playlist views
    m_application->playlist()->nowPlayingModel()->updateMediaItems(m_infoItemModel->mediaList());
    m_application->playlist()->playlistModel()->updateMediaItems(m_infoItemModel->mediaList());
    
    //Now that data is saved hide Save/Cancel controls
    ui->infoSaveHolder->setVisible(false);
    showIndexer();
    if (ui->infoFetcherExpander->isVisible()) {
        toggleShowInfoFetcherExpander();
    }
}

void InfoManager::cancelItemEdit()
{
    m_infoItemModel->cancelChanges();
    ui->infoSaveHolder->setVisible(false);
    showIndexer();
    if (ui->infoFetcherExpander->isVisible()) {
        toggleShowInfoFetcherExpander();
    }
}

void InfoManager::autoFetchInfo()
{
    if (m_currentInfoFetcher) {
        m_infoItemModel->autoFetch(m_currentInfoFetcher);
    }
}

void InfoManager::fetchInfo()
{
    if (m_currentInfoFetcher) {
        m_infoItemModel->fetch(m_currentInfoFetcher);
    }
}

void InfoManager::selectInfoFetcher(int index)
{
    if (index < 0) {
      return;
    }
    m_currentInfoFetcher = m_infoItemModel->availableInfoFetchers().at(index);
    if (m_currentInfoFetcher->about().isEmpty()) {
        ui->infoFetcherSelector->setToolTip(QString(""));
    } else {
        ui->infoFetcherSelector->setToolTip(QString("<b>%1</b><br>%2")
                                            .arg(m_currentInfoFetcher->name())
                                            .arg(m_currentInfoFetcher->about()));
    }
    ui->infoFetcherLink->setToolTip(QString("<b>%1</b><br>%2")
                                    .arg(m_currentInfoFetcher->name())
                                    .arg(m_currentInfoFetcher->url().prettyUrl()));
    ui->infoFetcherLink->setVisible(m_currentInfoFetcher->url().isValid());
}

void InfoManager::selectInfoFetcher(QAction * infoFetcherAction)
{
    QList<InfoFetcher *> infoFetchers = m_infoItemModel->availableInfoFetchers();
    for (int i = 0; i < infoFetchers.count(); i++) {
        if (infoFetchers.at(i)->name() == infoFetcherAction->text()) {
            m_currentInfoFetcher = infoFetchers.at(i);
            break;
        }
    }
}

void InfoManager::showInfoViewForMediaItem(const MediaItem &mediaItem)
{
    m_parent->switchMainWidget(MainWindow::MainMediaList);
    if (!ui->semanticsHolder->isVisible()) {
        ui->semanticsHolder->setVisible(true);
    }
    
    QList<MediaItem> mediaList;
    mediaList << mediaItem;
    if (mediaList.count() == 0) {
        return;
    }
    m_infoItemModel->loadInfo(mediaList);
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
            QSortFilterProxyModel* proxyModel = ui->mediaView->filterProxyModel();
            QModelIndex sourceIndex = proxyModel->mapToSource(selectedRows.at(i));
            selectedItems.append(m_application->browsingModel()->mediaItemAt(sourceIndex.row()));
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
            QSortFilterProxyModel* proxyModel = ui->mediaView->filterProxyModel();
            QModelIndex sourceIndex = proxyModel->mapToSource(selectedRows.at(i));
            selectedItems.append(m_application->browsingModel()->mediaItemAt(sourceIndex.row()));
       }
    }
    if (selectedItems.count() > 0) {
        m_application->browsingModel()->updateSourceInfo(selectedItems);
    }
}

void InfoManager::infoChanged(bool modified)
{
    ui->infoSaveHolder->setVisible(modified);
    if (modified) {
        ui->infoIndexerHolder->setVisible(false);
    } else {
        showIndexer();
    }
    showInfoFetcher();
}

void InfoManager::toggleShowInfoFetcherExpander()
{
    ui->infoFetcherExpander->setVisible(!ui->infoFetcherExpander->isVisible());
    if (ui->infoFetcherExpander->isVisible()) {
        ui->showInfoFetcherExpander->setToolTip(i18n("Click to hide"));
    } else {
        ui->showInfoFetcherExpander->setToolTip(i18n("Additional information may be available. <br> Click to show more..."));
        ui->infoFetcherExpander->setCurrentIndex(0);  //switch to infofetcher selector page everytime it is hidden.
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

    //Determine Information context
    m_context.clear();
    if (selectedRows.count() > 0) {
        //If items are selected then the context is the selected items
        for (int i = 0 ; i < selectedRows.count() ; ++i) {
            int row = proxy->mapToSource(selectedRows.at(i)).row();
            MediaItem mediaItem = m_application->browsingModel()->mediaItemAt(row);
            mediaItem.subTitle = QString();
            mediaItem.fields["isTemplate"] = false;
            m_context.append(mediaItem);
        }
    } else if (m_application->browsingModel()->rowCount()>0) {
        //If nothing is selected then the information context is 
        //the category selected to produce the list of media in the mediaview
        m_context.append(m_application->browsingModel()->mediaListProperties().category);
    } else {
        return;
    }

    //Show/Hide indexer
    showIndexer();
    
    //Load contextual data into info model and info boxes
    if (!(m_context.count() == 1 &&
          m_infoItemModel->mediaList().count() == 1 &&
          m_context.at(0).url == m_infoItemModel->mediaList().at(0).url)) {
        m_infoItemModel->loadInfo(m_context);

        if (m_context.at(0).fields["audioType"].toString() == "Audio Stream" ||
                m_context.at(0).fields["categoryType"].toString() == "Audio Feed" ||
                m_context.at(0).fields["categoryType"].toString() == "Video Feed") {
            ui->infoItemView->suppressEditing(false);
        } else {
            ui->infoItemView->suppressEditing(m_application->isTouchEnabled());
        }
    }

    //Show/Hide Info Fetcher
    ui->infoFetcherExpander->setCurrentIndex(0);
    ui->infoFetcherExpander->setVisible(false);
    showInfoFetcher();

    //Get context data for info boxes
    int totalInfoBoxes = ui->infoBoxHolder->layout()->count();
    QStringList contextLRIs;
    QStringList contextTitles;
    if (m_context.count() <= 20) { //TODO: This should be configurable
        for (int i = 0; i < m_context.count(); i++) {
            MediaItem contextCategory = m_context.at(i);
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
        totalInfoBoxes = ui->infoBoxHolder->layout()->count();
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
                InfoBox *infoBox = new InfoBox;
                infoBox->setMainWindow(m_parent);
                infoBox->setInfo(title, lri);
                if (m_enableTouch) {
                    infoBox->enableTouch();
                }
                QVBoxLayout * infoBoxHolderLayout = (QVBoxLayout *)ui->infoBoxHolder->layout();
                infoBoxHolderLayout->addWidget(infoBox);
                connect(infoBox->mediaView()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(infoBoxSelectionChanged(const QItemSelection, const QItemSelection)));
            }
        }

    }

    //Remove any unused infoboxes
    totalInfoBoxes = ui->infoBoxHolder->layout()->count();
    if (contextLRIs.count() < totalInfoBoxes) {
        for (int i = contextLRIs.count(); i < totalInfoBoxes; i++) {
            int lastItemIndex = ui->infoBoxHolder->layout()->count() - 1;
            if (lastItemIndex >= 0) {
                InfoBox * unusedInfoBox = (InfoBox *)ui->infoBoxHolder->layout()->itemAt(lastItemIndex)->widget();
                disconnect(unusedInfoBox->mediaView()->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(infoBoxSelectionChanged(const QItemSelection, const QItemSelection)));
                ui->infoBoxHolder->layout()->removeWidget(unusedInfoBox);
                delete unusedInfoBox;
            }
        }
    }
}

void InfoManager::showIndexer()
{
    //Show indexer for selected local filelistengine items
    bool indexerVisible = false;
    if (m_nepomukInited) {
        bool isLocalFileList = false;
        MediaListProperties viewProperties = m_application->browsingModel()->mediaListProperties();
        if (viewProperties.lri.startsWith("files://") && viewProperties.engineFilterList().count() >= 2) {
            KUrl viewUrl(viewProperties.engineFilterList().at(1));
            if (!viewUrl.isEmpty() && viewUrl.isLocalFile()) {
                isLocalFileList = true;
            }
        }

        QModelIndexList selectedRows = ui->mediaView->selectionModel()->selectedRows();
        MediaSortFilterProxyModel *proxy = (MediaSortFilterProxyModel *) ui->mediaView->model();
        if (selectedRows.count() > 0) {
            for (int i = 0 ; i < selectedRows.count() ; ++i) {
                int row = proxy->mapToSource(selectedRows.at(i)).row();
                MediaItem selectedItem = m_application->browsingModel()->mediaItemAt(row);
                MediaListProperties selectedProperties;
                selectedProperties.lri = selectedItem.url;
                if (selectedProperties.lri.startsWith("files://") && selectedProperties.engineFilterList().count() >= 2) {
                    KUrl selectedUrl(selectedProperties.engineFilterList().at(1));
                    if (!selectedUrl.isEmpty() && selectedUrl.isLocalFile()) {
                        indexerVisible = true;
                    } else {
                        indexerVisible = false;
                        break;
                    }
                } else if (isLocalFileList && (selectedItem.type == "Audio" || selectedItem.type == "Video")) {
                    KUrl selectedUrl(selectedItem.url);
                    if (!selectedUrl.isEmpty() && selectedUrl.isLocalFile()) {
                        indexerVisible = true;
                    } else {
                        indexerVisible = false;
                        break;
                    }
                } else {
                    indexerVisible = false;
                    break;
                }
            }
        }
    }
    ui->infoIndexerHolder->setVisible(indexerVisible);
}

void InfoManager::showInfoFetcher()
{
    if (m_infoItemModel->availableInfoFetchers().count() > 0) {

        //Load available InfoFetchers and determine if any are fetching info
        bool isFetching = false;
        ui->infoFetcherSelector->clear();
        for (int i = 0; i < m_infoItemModel->availableInfoFetchers().count(); i++) {
            InfoFetcher * infoFetcher = m_infoItemModel->availableInfoFetchers().at(i);
            ui->infoFetcherSelector->addItem(infoFetcher->icon(), infoFetcher->name());
            if (infoFetcher->isFetching()) {
                ui->infoFetcherSelector->setCurrentIndex(i);
                m_currentInfoFetcher = infoFetcher;
                isFetching = true;
            }
        }

        //Select default InfoFetcher
        if (ui->infoFetcherSelector->currentIndex() == -1) {
            m_currentInfoFetcher = m_infoItemModel->availableInfoFetchers().at(0);
            ui->infoFetcherSelector->setCurrentIndex(0);
        }

        //Show Info Fetcher UI
        if (!isFetching) {
            ui->showInfoFetcherExpander->setVisible(true);
            ui->infoFetcherHolder->setVisible(true);
        }

        //Offer Autofetch and Fetch buttons
        bool autoFetchVisible = m_infoItemModel->autoFetchIsAvailable(m_currentInfoFetcher);
        bool fetchVisible = m_infoItemModel->fetchIsAvailable(m_currentInfoFetcher);
        ui->infoAutoFetch->setVisible(autoFetchVisible);
        ui->infoFetch->setVisible(fetchVisible);

    } else {
        ui->infoFetcherHolder->setVisible(false);
    }
}

void InfoManager::showFetching()
{
    //Hide everything but the fetching message
    if (ui->infoFetcherExpander->isVisible()) {
        toggleShowInfoFetcherExpander();
    }
    ui->infoFetcherHolder->setVisible(false);
    ui->showInfoFetcherExpander->setVisible(false);
}

void InfoManager::fetchComplete()
{
    ui->showInfoFetcherExpander->setVisible(true);

    //Determine if multiple matches are available for fetched info and show them
    if (m_infoItemModel->fetchedMatches().count() > 1) {
        disconnect(ui->fetchedMatches, SIGNAL(currentRowChanged(int)), m_infoItemModel, SLOT(selectFetchedMatch(int)));
        ui->fetchedMatches->clear();
        QList<MediaItem> fetchedMatches = m_infoItemModel->fetchedMatches();
        for (int i = 0; i < fetchedMatches.count(); i++) {
            ui->fetchedMatches->addItem(fetchedMatches.at(i).title);
        }
        ui->fetchedMatches->setCurrentRow(0);;
        connect(ui->fetchedMatches, SIGNAL(currentRowChanged(int)), m_infoItemModel, SLOT(selectFetchedMatch(int)));
        if (!ui->infoFetcherExpander->isVisible()) {
            toggleShowInfoFetcherExpander();
        }
        ui->infoFetcherExpander->setCurrentIndex(1);
        ui->fetchedMatches->setFocus();
    } else {
        ui->infoFetcherExpander->setCurrentIndex(0);
        if (ui->infoFetcherExpander->isVisible()) {
            toggleShowInfoFetcherExpander();
        }
    }
    ui->infoFetcherHolder->setVisible(true);
}

void InfoManager::infoBoxSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected)
{
    if (selected.indexes().count() > 0) {
        //Only allow one item in one infobox to be selected at a time.
        int totalInfoBoxes = ui->infoBoxHolder->layout()->count();
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

void InfoManager::fetchingStatusUpdated()
{
    QHash<QString, QVariant> status = m_infoItemModel->fetchingStatus();
    QString description = status["description"].toString();
    int progress = status["progress"].toInt();
    if (!description.isEmpty()) {
        ui->notificationWidget->setVisible(true);
        QFontMetrics fm(ui->notificationText->font());
        QString notificationText = fm.elidedText(description, Qt::ElideRight, ui->notificationText->width());
        ui->notificationText->setText(notificationText);
    } else {
        m_application->mediaListsManager()->delayedNotificationHide();
    }
    if (progress >= 0 && progress <= 100) {
        ui->notificationProgress->setValue(progress);
        ui->notificationProgress->setVisible(true);
    } else {
        ui->notificationProgress->setVisible(false);
    }
}

void InfoManager::openInfoFetcherLink()
{
    if (m_currentInfoFetcher->url().isValid()) {
        QDesktopServices::openUrl(m_currentInfoFetcher->url());
    }
}

void InfoManager::enableTouch()
{
    int tTouchable = BangarangApplication::TOUCH_TOUCHABLE_METRIC;
    int tVisual = BangarangApplication::TOUCH_VISUAL_METRIC;
    ui->showInfoFetcherExpander->setMinimumSize(tTouchable, tTouchable);
    ui->showInfoFetcherExpander->setIconSize(QSize(tVisual, tVisual));
    ui->infoFetcherHolder->setMaximumHeight(160);
    ui->infoFetcherSelector->setMinimumHeight(tTouchable);
    ui->infoFetcherLink->setMinimumSize(tTouchable, tTouchable);
    ui->infoFetcherLink->setIconSize(QSize(tVisual, tVisual));
    ui->infoFetch->setMinimumHeight(tTouchable);
    ui->infoAutoFetch->setMinimumHeight(tTouchable);
    ui->infoItemSave->setMinimumHeight(tTouchable);
    ui->infoItemCancelEdit->setMinimumHeight(tTouchable);
    ui->infoIndexSelected->setMinimumHeight(tTouchable);
    ui->infoItemView->enableTouch();
    FlickCharm *charm = new FlickCharm(ui->infoItemViewHolder);
    charm->activateOn(ui->infoItemViewHolder);
    m_enableTouch = true;
}
