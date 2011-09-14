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

#include <QScrollBar>

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

    //Setup media list filter
    ui->mediaListFilterProxyLine->lineEdit()->setClickMessage(QString());
    ui->mediaListFilterProxyLine->setProxy(ui->mediaView->filterProxyModel());
    ui->mediaListFilter->setVisible(false);
    connect(ui->closeMediaListFilter, SIGNAL(clicked()), this, SLOT(closeMediaListFilter()));

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
    if (selected.indexes().count() > 0) {
        int firstRow = selected.indexes().at(0).row();
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
    } else if (listSelection == VideoList) {
        ui->audioListsStackHolder->layout()->removeWidget(m_application->mainWindow()->audioListsStack());
        ui->audioListsStackHolder->hide();
        ui->audioListsSelectHolder->show();
        ui->audioListLabel->hide();
        ui->videoListsStackHolder->layout()->addWidget(m_application->mainWindow()->videoListsStack());
        ui->videoListsStackHolder->show();
        ui->videoListsSelectHolder->hide();
        ui->videoListLabel->show();
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
