#include "audiolistsstack.h"
#include "../common/bangarangapplication.h"
#include "../common/mainwindow.h"
#include "../common/flickcharm.h"
#include "medialistsmanager.h"
#include "savedlistsmanager.h"
#include "medialistsettings.h"
#include "../../platform/mediaitemmodel.h"


AudioListsStack::AudioListsStack(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioListsStack)
{
    ui->setupUi(this);
    m_application = (BangarangApplication *)KApplication::kApplication();
    ui->audioLists->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
}

AudioListsStack::~AudioListsStack()
{
    delete ui;
}

void AudioListsStack::enableTouch() {
    int tTouchable = BangarangApplication::TOUCH_TOUCHABLE_METRIC;
    int tVisual = BangarangApplication::TOUCH_VISUAL_METRIC;
    ui->addAudioList->setMinimumSize(tTouchable, tTouchable);
    ui->addAudioList->setIconSize(QSize(tVisual, tVisual));
    ui->removeAudioList->setMinimumSize(tTouchable, tTouchable);
    ui->removeAudioList->setIconSize(QSize(tVisual, tVisual));
    ui->configureAudioList->setMinimumSize(tTouchable, tTouchable);
    ui->configureAudioList->setIconSize(QSize(tVisual, tVisual));
    ui->audioLists->setIconSize(QSize(tTouchable, tTouchable));
    ui->audioLists->setGridSize(QSize(0, tTouchable + 2));
    FlickCharm* charm = new FlickCharm(this);
    charm->activateOn(ui->audioLists);
    ui->aNewListName->setMinimumHeight(tTouchable);
    ui->saveAudioList->setMinimumHeight(tTouchable);
    ui->aCancelSaveList->setMinimumHeight(tTouchable);
    ui->aslsListName->setMinimumHeight(tTouchable);
    ui->aslsSave->setMinimumHeight(tTouchable);
    ui->exportSavedListLabel->hide();
    ui->aslsExport->hide();
    ui->aslsCancel->setMinimumHeight(tTouchable);
    ui->semALimit->setMinimumHeight(tTouchable);
    ui->semATimeComp->setMinimumHeight(tTouchable);
    ui->semATime->setMinimumHeight(tTouchable);
    ui->semARating->setMinimumHeight(tTouchable);
    ui->semARatingComp->setMinimumHeight(tTouchable);
    ui->semAFreq->setMinimumHeight(tTouchable);
    ui->semAFreqComp->setMinimumHeight(tTouchable);
    ui->semAConfigSave->setMinimumHeight(tTouchable);
    ui->aCancelSemConfigure->setMinimumHeight(tTouchable);
}

void AudioListsStack::on_configureAudioList_clicked()
{
    if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0) {
        int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_application->mediaListsManager()->audioListsModel()->mediaItemAt(selectedRow);
        if (selectedItem.url.startsWith(QLatin1String("savedlists://")) ||
            selectedItem.url.startsWith(QLatin1String("ampache://"))) {
            m_application->savedListsManager()->showAudioSavedListSettings();
        } else if (selectedItem.url.startsWith(QLatin1String("semantics://recent")) ||
            selectedItem.url.startsWith(QLatin1String("semantics://frequent")) ||
            selectedItem.url.startsWith(QLatin1String("semantics://highest")) ||
            selectedItem.url.startsWith(QLatin1String("semantics://recentlyadded"))) {
            m_application->mainWindow()->mediaListSettings()->showMediaListSettings();
        }
    }
}
