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

#include "medialistsmanager.h"
#include "../common/bangarangapplication.h"
#include "../common/mainwindow.h"
#include "ui_mainwindow.h"
#include "../common/mediaview.h"
#include "infomanager.h"
#include "../common/actionsmanager.h"
#include "../../platform/mediaitemmodel.h"
#include "../../platform/utilities/artwork.h"
#include "../../platform/utilities/general.h"

#include <QScrollBar>
#include <KDebug>

MediaListsManager::MediaListsManager(MainWindow* parent) : QObject(parent)
{
    m_application = (BangarangApplication*)KApplication::kApplication();
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;

    //Set up Audio lists view
    MediaListProperties audioListsProperties;
    audioListsProperties.lri = "medialists://audio";
    m_audioListsModel = new MediaItemModel(this);
    m_audioListsModel->setMediaListProperties(audioListsProperties);
    QListView* audioLists = m_application->mainWindow()->audioListsStack()->ui->audioLists;
    audioLists->setModel(m_audioListsModel);
    connect(audioLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(audioListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_audioListsModel, SIGNAL(mediaListChanged()), this, SLOT(audioListsChanged()));
    m_audioListsModel->load();
    QToolButton* audioListSelect = m_application->mainWindow()->ui->audioListSelect;
    connect(audioListSelect, SIGNAL(clicked()), this, SLOT(selectAudioList()));

    //Set up Video lists view
    MediaListProperties videoListsProperties;
    videoListsProperties.lri = "medialists://video";
    m_videoListsModel = new MediaItemModel(this);
    m_videoListsModel->setMediaListProperties(videoListsProperties);
    QListView* videoLists = m_application->mainWindow()->videoListsStack()->ui->videoLists;
    videoLists->setModel(m_videoListsModel);
    connect(videoLists->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(videoListsSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(m_videoListsModel, SIGNAL(mediaListChanged()), this, SLOT(videoListsChanged()));
    m_videoListsModel->load();
    QToolButton* videoListSelect = m_application->mainWindow()->ui->videoListSelect;
    connect(videoListSelect, SIGNAL(clicked()), this, SLOT(selectVideoList()));

    //Set up media list view
    m_loadingProgress = 0;
    MediaView* mediaView = ui->mediaView;
    mediaView->setMainWindow(m_application->mainWindow());
    mediaView->setSourceModel(m_application->browsingModel());
    connect(m_application->browsingModel(), SIGNAL(mediaListChanged()), this, SLOT(mediaListChanged()));
    connect(m_application->browsingModel(), SIGNAL(mediaListPropertiesChanged()), this, SLOT(mediaListPropertiesChanged()));
    connect(m_application->browsingModel(), SIGNAL(loading()), this, SLOT(mediaListLoading()));
    connect(m_application->browsingModel(), SIGNAL(loadingStateChanged(bool)), this, SLOT(mediaListLoadingStateChanged(bool)));
    connect(m_application->browsingModel(), SIGNAL(propertiesChanged()), this, SLOT(updateListHeader()));
    connect((MediaItemDelegate *)mediaView->itemDelegate(), SIGNAL(categoryActivated(QModelIndex)), this, SLOT(mediaListCategoryActivated(QModelIndex)));
    connect((MediaItemDelegate *)mediaView->itemDelegate(), SIGNAL(actionActivated(QModelIndex)), this, SLOT(mediaListActionActivated(QModelIndex)));
    connect(mediaView->selectionModel(), SIGNAL(selectionChanged(const QItemSelection, const QItemSelection)), this, SLOT(mediaSelectionChanged(const QItemSelection, const QItemSelection)));
    connect(ui->previous, SIGNAL(clicked()), this, SLOT(loadPreviousList()));
    connect(m_application->playlist()->nowPlayingModel(), SIGNAL(mediaListChanged()), this, SLOT(nowPlayingChanged()));
    connect(m_application->mainWindow(), SIGNAL(switchedMainWidget(MainWindow::MainWidget)), this, SLOT(defaultListLoad(MainWindow::MainWidget)));

    //Setup media list filter
    ui->mediaListFilterProxyLine->lineEdit()->setClickMessage(QString());
    ui->mediaListFilterProxyLine->setProxy(ui->mediaView->filterProxyModel());
    ui->mediaListFilter->setVisible(false);
    connect(ui->closeMediaListFilter, SIGNAL(clicked()), this, SLOT(closeMediaListFilter()));

    //Set up play select/all buttons
    connect(ui->playAll, SIGNAL(clicked()), this, SLOT(playAll()));
    connect(ui->playSelected, SIGNAL(clicked()), this, SLOT(playSelected()));

    //Setup browsing model status notifications
    connect(m_application->browsingModel(), SIGNAL(statusUpdated()), this, SLOT(browsingModelStatusUpdated()));

    //Set up search
    KLineEdit* searchField = m_application->mainWindow()->ui->Filter;
    connect(searchField, SIGNAL(returnPressed()), this, SLOT(loadSearch()));

    //Set up device notifier
    connect(DeviceManager::instance(), SIGNAL(deviceListChanged(DeviceManager::RelatedType)),
            this, SLOT(updateDeviceList(DeviceManager::RelatedType)));

    //Set default media list selection
    showMediaList(AudioList);

}

//--- Audio lists slots ---
void MediaListsManager::audioListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if ((m_mediaListSelection == AudioList) &&
        (m_application->mainWindow()->currentMainWidget() == MainWindow::MainMediaList) &&
        (selected.indexes().count() > 0)) {
        int selectedRow = selected.indexes().at(0).row();
        loadMediaList(m_audioListsModel, selectedRow);
    }
    Q_UNUSED(deselected);
}

void MediaListsManager::audioListsChanged()
{
    QModelIndex index = m_audioListsModel->index(0, 0);
    if (index.isValid()) {
        QListView* audioLists = m_application->mainWindow()->audioListsStack()->ui->audioLists;
        audioLists->selectionModel()->select( index, QItemSelectionModel::Select );
    }
}

//--- Video Lists slots ---
void MediaListsManager::videoListsSelectionChanged(const QItemSelection & selected, const QItemSelection & deselected)
{
    if ((m_mediaListSelection == VideoList) &&
        (m_application->mainWindow()->currentMainWidget() == MainWindow::MainMediaList) &&
        (selected.indexes().count() > 0)) {
        int selectedRow = selected.indexes().at(0).row();
        loadMediaList(m_videoListsModel, selectedRow);
    }
    Q_UNUSED(deselected);
}

void MediaListsManager::videoListsChanged()
{
    QModelIndex index = m_videoListsModel->index(0, 0);
    if (index.isValid()) {
        QListView* videoLists = m_application->mainWindow()->videoListsStack()->ui->videoLists;
        videoLists->selectionModel()->select( index, QItemSelectionModel::Select );
    }
}

//--- Main Media List slots ---
void MediaListsManager::mediaListChanged()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);

    MediaItemModel* model = m_application->browsingModel();

    if ((model->rowCount() > 0) && (ui->mediaViewHolder->currentIndex() ==0)) {
        MediaItem firstListItem = model->mediaItemAt(0);
        QString listItemType = firstListItem.type;
        if ((listItemType == "Audio") || (listItemType == "Video") || (listItemType == "Category")) {
            if (!firstListItem.fields["isTemplate"].toBool()) {
                ui->playAll->setVisible(true);
            }
        } else {
            ui->playAll->setVisible(false);
        }
        ui->playSelected->setVisible(false);
    }
}

void MediaListsManager::mediaListLoading()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (ui->mediaListFilter->isVisible()) {
        ui->mediaListFilterProxyLine->lineEdit()->clear();
        hidePlayButtons();
    }
}

void MediaListsManager::mediaListLoadingStateChanged(bool loading)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (loading) {
        if (!ui->loadingIndicator->isVisible()) {
            ui->loadingIndicator->show();
            showMediaListLoading();
        }
    } else {
        ui->loadingIndicator->hide();
    }
}

void MediaListsManager::mediaListPropertiesChanged()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
}

void MediaListsManager::updateListHeader()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->listTitle->setText(m_application->browsingModel()->mediaListProperties().name);
    ui->listSummary->setText(m_application->browsingModel()->mediaListProperties().summary);
}

void MediaListsManager::mediaListCategoryActivated(QModelIndex index)
{
    addListToHistory();
    m_application->browsingModel()->categoryActivated(index);
}

void MediaListsManager::mediaListActionActivated(QModelIndex index)
{
    addListToHistory();
    m_application->browsingModel()->actionActivated(index);
}

void MediaListsManager::mediaSelectionChanged (const QItemSelection & selected, const QItemSelection & deselected )
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    QModelIndexList selectedIndexes = ui->mediaView->selectionModel()->selectedIndexes(); // use this instead of selected which works better after a selectAll
    if (selectedIndexes.count() > 0) {
        int firstRow = selectedIndexes.at(0).row();
        MediaItem firstSelectedItem = m_application->browsingModel()->mediaItemAt(firstRow);
        QString itemType = firstSelectedItem.type;
        if ((itemType == "Audio") || (itemType == "Video") || (itemType == "Category")) {
            if (firstSelectedItem.fields.contains("isTemplate") &&
                firstSelectedItem.fields["isTemplate"].toBool()) {
                ui->playSelected->setVisible(false);
            } else {
                ui->playSelected->setVisible(true);
            }
            ui->playAll->setVisible(false);
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(false);
        }
    } else {
        MediaItem firstListItem = m_application->browsingModel()->mediaItemAt(0);
        QString itemType = firstListItem.type;
        if ((itemType == "Audio") || (itemType == "Video") || (itemType == "Category")) {
            if (firstListItem.fields.contains("isTemplate") &&
                firstListItem.fields["isTemplate"].toBool()) {
                ui->playAll->setVisible(false);
            } else {
                ui->playAll->setVisible(true);
            }
            ui->playSelected->setVisible(false);
        } else {
            ui->playSelected->setVisible(false);
            ui->playAll->setVisible(false);
        }
    }
    Q_UNUSED(selected);
    Q_UNUSED(deselected);
}

void MediaListsManager::closeMediaListFilter()
{
    m_application->actionsManager()->action("toggle_filter")->trigger();
}

//--- General Functions ---
void MediaListsManager::loadMediaList(MediaItemModel *listsModel, int row)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    MediaListProperties currentProperties;
    MediaItem selectedItem = listsModel->mediaItemAt(row);
    currentProperties.name = selectedItem.title;
    currentProperties.lri = selectedItem.url;
    currentProperties.category = selectedItem;
    if (m_application->browsingModel()->mediaListProperties().lri != currentProperties.lri) {
        m_application->browsingModel()->clearMediaListData();
        m_application->browsingModel()->setMediaListProperties(currentProperties);
        m_application->browsingModel()->load();
        m_mediaListHistory.clear();
        m_mediaListPropertiesHistory.clear();
        ui->previous->setVisible(false);
        ui->mediaViewHolder->setCurrentIndex(0);
    }

    //Update InfoManager Context
    m_application->infoManager()->setContext(selectedItem);

    //Determine if selected list is configurable
    bool isAudioList = (listsModel->mediaListProperties().lri == "medialists://audio");
    bool isVideoList = (listsModel->mediaListProperties().lri == "medialists://video");
    bool selectedIsConfigurable = selectedItem.fields["isConfigurable"].toBool();
    if (isAudioList) {
        m_application->mainWindow()->audioListsStack()->ui->configureAudioList->setVisible(selectedIsConfigurable);
    } else if (isVideoList) {
        m_application->mainWindow()->videoListsStack()->ui->configureVideoList->setVisible(selectedIsConfigurable);
    }
}

void MediaListsManager::loadPreviousList()
{
    if (m_mediaListHistory.isEmpty()) {
        return;
    }
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    m_application->browsingModel()->clearMediaListData();
    m_application->browsingModel()->setMediaListProperties(m_mediaListPropertiesHistory.last());
    m_application->browsingModel()->loadMediaList(m_mediaListHistory.last(), true);
    QApplication::processEvents(); // Allows view to get updated before scrollbar is set
    ui->mediaView->verticalScrollBar()->setValue(m_mediaListScrollHistory.last());

    //Clean up history and update previous button
    m_mediaListHistory.removeLast();
    m_mediaListPropertiesHistory.removeLast();
    m_mediaListScrollHistory.removeLast();
    if (m_mediaListPropertiesHistory.count() > 0) {
        ui->previous->setVisible(true);
        ui->previous->setText(m_mediaListPropertiesHistory.last().name);
    } else {
        ui->previous->setVisible(false);
    }

}

void MediaListsManager::addListToHistory()
{
    //Add medialList to history
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    QList<MediaItem> mediaList = m_application->browsingModel()->mediaList();
    m_mediaListHistory.append(mediaList);
    m_mediaListPropertiesHistory << m_application->browsingModel()->mediaListProperties();
    m_mediaListScrollHistory << ui->mediaView->verticalScrollBar()->value();
    QString previousButtonText(m_application->browsingModel()->mediaListProperties().name);
    ui->previous->setText(previousButtonText);
    ui->previous->setVisible(true);
}

void MediaListsManager::hidePlayButtons()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->playSelected->setVisible(false);
    ui->playAll->setVisible(false);
}

void MediaListsManager::showMediaListLoading()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (ui->loadingIndicator->isVisible()) {
        m_loadingProgress++;
        if ((m_loadingProgress > 7) || (m_loadingProgress < 0)) {
            m_loadingProgress = 0;
        }
        QString iconName = QString("bangarang-loading-%1").arg(m_loadingProgress);
        QPixmap pixmap = KIcon(iconName).pixmap(16, 16);
        ui->loadingIndicator->setPixmap(pixmap);
        QTimer::singleShot(100, this, SLOT(showMediaListLoading()));
    }
}

void MediaListsManager::selectAudioList()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    showMediaList(AudioList);
    m_application->mainWindow()->audioListsStack()->ui->audioListsStack->setCurrentIndex(0);
    QListView* audioLists = m_application->mainWindow()->audioListsStack()->ui->audioLists;
    if (audioLists->selectionModel()->selectedIndexes().count() > 0){
        int selectedRow = audioLists->selectionModel()->selectedIndexes().at(0).row();
        loadMediaList(m_audioListsModel, selectedRow);
    }
    m_application->mainWindow()->audioListsStack()->ui->audioLists->setFocus();
    ui->Filter->setClickMessage(i18n("Search for audio"));
    ui->Filter->clear();

}

void MediaListsManager::selectVideoList()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    showMediaList(VideoList);
    m_application->mainWindow()->videoListsStack()->ui->videoListsStack->setCurrentIndex(0);
    QListView* videoLists = m_application->mainWindow()->videoListsStack()->ui->videoLists;
    if (videoLists->selectionModel()->selectedIndexes().count() > 0){
        int selectedRow = videoLists->selectionModel()->selectedIndexes().at(0).row();
        loadMediaList(m_videoListsModel, selectedRow);
    }
    m_application->mainWindow()->videoListsStack()->ui->videoLists->setFocus();
    ui->Filter->setClickMessage(i18n("Search for video"));
    ui->Filter->clear();

}

void MediaListsManager::loadSearch()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    ui->mediaView->setFocus();
    QListView* audioLists = m_application->mainWindow()->audioListsStack()->ui->audioLists;
    audioLists->selectionModel()->clearSelection();
    QListView* videoLists = m_application->mainWindow()->videoListsStack()->ui->videoLists;
    videoLists->selectionModel()->clearSelection();
    if (!ui->Filter->text().isEmpty()) {
        addListToHistory();
        if (m_mediaListSelection == MediaListsManager::AudioList) {
            MediaListProperties searchProperties;
            searchProperties.name = i18n("Audio Search");
            searchProperties.lri = QString("music://search?%1").arg(ui->Filter->text());
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(searchProperties);
            m_application->browsingModel()->load();
          } else {
            MediaListProperties searchProperties;
            searchProperties.name = i18n("Video Search");
            searchProperties.lri = QString("video://search?%1").arg(ui->Filter->text());
            m_application->browsingModel()->clearMediaListData();
            m_application->browsingModel()->setMediaListProperties(searchProperties);
            m_application->browsingModel()->load();
        }
    }

}

void MediaListsManager::showMediaList(MediaListSelection listSelection)
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    if (listSelection == AudioList) {
        ui->videoListsStackHolder->layout()->removeWidget(m_application->mainWindow()->videoListsStack());
        ui->videoListsStackHolder->hide();
        ui->videoListsSelectHolder->show();
        ui->videoListLabel->hide();
        ui->audioListsStackHolder->layout()->addWidget(m_application->mainWindow()->audioListsStack());
        ui->audioListsStackHolder->show();
        ui->audioListsSelectHolder->hide();
        ui->audioListLabel->show();
        m_application->mainWindow()->resetTabOrder();
        m_application->mainWindow()->audioListsStack()->ui->audioLists->setFocus();
    } else if (listSelection == VideoList) {
        ui->audioListsStackHolder->layout()->removeWidget(m_application->mainWindow()->audioListsStack());
        ui->audioListsStackHolder->hide();
        ui->audioListsSelectHolder->show();
        ui->audioListLabel->hide();
        ui->videoListsStackHolder->layout()->addWidget(m_application->mainWindow()->videoListsStack());
        ui->videoListsStackHolder->show();
        ui->videoListsSelectHolder->hide();
        ui->videoListLabel->show();
        m_application->mainWindow()->resetTabOrder();
        m_application->mainWindow()->videoListsStack()->ui->videoLists->setFocus();
    }
    m_mediaListSelection = listSelection;
}

MediaListsManager::MediaListSelection MediaListsManager::currentMediaListSelection()
{
    return m_mediaListSelection;
}

MediaItemModel* MediaListsManager::audioListsModel()
{
    return m_audioListsModel;
}

MediaItemModel* MediaListsManager::videoListsModel()
{
    return m_videoListsModel;
}

void MediaListsManager::updateDeviceList(DeviceManager::RelatedType type) {
    if ( type == DeviceManager::AudioType || type == DeviceManager::AllTypes ) {
        m_audioListsModel->clearMediaListData();
        m_audioListsModel->load();
    }
    if ( type == DeviceManager::VideoType || type == DeviceManager::AllTypes ) {
        m_videoListsModel->clearMediaListData();
        m_videoListsModel->load();
    }
}

void MediaListsManager::playSelected()
{
    m_application->actionsManager()->setContextMenuSource(MainWindow::Default);
    m_application->actionsManager()->action("play_selected")->trigger();
}

void MediaListsManager::playAll()
{
    m_application->actionsManager()->action("play_all")->trigger();
}

void MediaListsManager::browsingModelStatusUpdated()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    QHash<QString, QVariant> status = m_application->browsingModel()->status();
    QString description = status["description"].toString();
    int progress = status["progress"].toInt();
    if (!description.isEmpty()) {
        ui->notificationWidget->setVisible(true);
        QFontMetrics fm(ui->notificationText->font());
        QString notificationText = fm.elidedText(description, Qt::ElideRight, ui->notificationText->width());
        ui->notificationText->setText(notificationText);
    } else {
        ui->notificationText->setText(i18n("Complete"));
        delayedNotificationHide();
    }
    if (progress >= 0 && progress <= 100) {
        ui->notificationProgress->setValue(progress);
        ui->notificationProgress->setVisible(true);
    } else {
        ui->notificationProgress->setVisible(false);
    }
}

void MediaListsManager::delayedNotificationHide()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    QTimer::singleShot(3000, ui->notificationWidget, SLOT(hide()));
}

void MediaListsManager::updateSeekTime(qint64 time)
{
    //Add currently playing item to browsing model if contents is "Recently Played"
    if (time > 12000 && time < 13000) {
        if (m_application->playlist()->nowPlayingModel()->rowCount() == 0) {
            return;
        }
        MediaItem nowPlayingItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
        MediaListProperties mediaListProperties = m_application->browsingModel()->mediaListProperties();
        if (!mediaListProperties.lri.startsWith(QString("semantics://recent?%1").arg(nowPlayingItem.type.toLower()))) {
            return;
        }
        QStringList filterList = mediaListProperties.engineFilterList();
        int filterIndex = -1;
        for (int i = 0; i < filterList.count(); i++) {
            if (filterList.at(i).startsWith("lastPlayed")) {
                filterIndex = i;
                break;
            }
        }
        if (filterIndex >= 0) {
            if (mediaListProperties.filterOperator(filterList.at(filterIndex)) != ">") {
                return;
            }
        }
        nowPlayingItem.artwork = Utilities::defaultArtworkForMediaItem(nowPlayingItem);
        nowPlayingItem.semanticComment = Utilities::wordsForTimeSince(nowPlayingItem.fields["lastPlayed"].toDateTime());
        m_application->browsingModel()->insertMediaItemAt(0, nowPlayingItem);
    }
 }

void MediaListsManager::nowPlayingChanged()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;

    //Update Media List view
    int startRow = ui->mediaView->indexAt(ui->mediaView->rect().topLeft()).row();
    int endRow = ui->mediaView->indexAt(ui->mediaView->rect().bottomRight()).row();
    if (endRow == -1) {
        endRow = startRow + ui->mediaView->model()->rowCount();
    }
    for  (int i = startRow; i <= endRow; i++) {
        ui->mediaView->update(ui->mediaView->model()->index(i, 0));
    }
}

void MediaListsManager::defaultListLoad(MainWindow::MainWidget which)
{
    if (which != MainWindow::MainMediaList) {
        return;
    }
    if (!m_application->browsingModel()->mediaListProperties().lri.isEmpty()) {
        return;
    }
    if (m_mediaListSelection == MediaListsManager::AudioList) {
        selectAudioList();
    } else {
        selectVideoList();
    }
}

void MediaListsManager::showMenu()
{
    Ui::MainWindowClass* ui = m_application->mainWindow()->ui;
    QMenu * menu = m_application->actionsManager()->mediaViewMenu(true);
    QPoint menuLocation = ui->showMediaViewMenu->mapToGlobal(QPoint(0,ui->showMediaViewMenu->height()));
    menu->exec(menuLocation);
}
