#include "../common/bangarangapplication.h"
#include "../common/mainwindow.h"
#include "../common/flickcharm.h"
#include "medialistsmanager.h"
#include "savedlistsmanager.h"
#include "medialistsettings.h"
#include "../../platform/mediaitemmodel.h"
#include "videolistsstack.h"

VideoListsStack::VideoListsStack(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::VideoListsStack)
{
    ui->setupUi(this);
    m_application = (BangarangApplication *)KApplication::kApplication();
    ui->videoLists->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

VideoListsStack::~VideoListsStack()
{
    delete ui;
}

void VideoListsStack::enableTouch() {
    int tTouchable = BangarangApplication::TOUCH_TOUCHABLE_METRIC;
    int tVisual = BangarangApplication::TOUCH_VISUAL_METRIC;
    ui->addVideoList->setMinimumSize(tTouchable, tTouchable);
    ui->addVideoList->setIconSize(QSize(tVisual, tVisual));
    ui->removeVideoList->setMinimumSize(tTouchable, tTouchable);
    ui->removeVideoList->setIconSize(QSize(tVisual, tVisual));
    ui->configureVideoList->setMinimumSize(tTouchable, tTouchable);
    ui->configureVideoList->setIconSize(QSize(tVisual, tVisual));
    ui->videoLists->setIconSize(QSize(tTouchable, tTouchable));
    ui->videoLists->setGridSize(QSize(0,tTouchable));
    FlickCharm* charm = new FlickCharm(this);
    charm->activateOn(ui->videoLists);
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
