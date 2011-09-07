#include "bangarangapplication.h"
#include "mainwindow.h"
#include "medialistsmanager.h"
#include "savedlistsmanager.h"
#include "medialistsettings.h"
#include "platform/mediaitemmodel.h"
#include "videolistsstack.h"

VideoListsStack::VideoListsStack(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoListsStack)
{
    ui->setupUi(this);
    m_application = (BangarangApplication *)KApplication::kApplication();
}

VideoListsStack::~VideoListsStack()
{
    delete ui;
}

void VideoListsStack::enableTouch() {
    ui->addVideoList->setMinimumSize(32, 32);
    ui->removeVideoList->setMinimumSize(32, 32);
    ui->configureVideoList->setMinimumSize(32, 32);
    ui->videoLists->setIconSize(QSize(32, 32));
    ui->videoLists->setGridSize(QSize(0,32));
}


void VideoListsStack::on_configureVideoList_clicked()
{
    if (ui->videoLists->selectionModel()->selectedIndexes().count() > 0) {
        int selectedRow = ui->videoLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_application->mediaListsManager()->videoListsModel()->mediaItemAt(selectedRow);
        if (selectedItem.url.startsWith("savedlists://")) {
            m_application->savedListsManager()->showVideoSavedListSettings();
        } else if (selectedItem.url.startsWith("semantics://recent") ||
            selectedItem.url.startsWith("semantics://frequent") ||
            selectedItem.url.startsWith("semantics://highest")) {
            m_application->mainWindow()->mediaListSettings()->showMediaListSettings();
        }
    }
}
