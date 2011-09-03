#include "bangarangapplication.h"
#include "mainwindow.h"
#include "savedlistsmanager.h"
#include "medialistsettings.h"
#include "platform/mediaitemmodel.h"
#include "audiolistsstack.h"

AudioListsStack::AudioListsStack(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AudioListsStack)
{
    ui->setupUi(this);
    m_application = (BangarangApplication *)KApplication::kApplication();
}

AudioListsStack::~AudioListsStack()
{
    delete ui;
}

void AudioListsStack::on_configureAudioList_clicked()
{
    if (ui->audioLists->selectionModel()->selectedIndexes().count() > 0) {
        int selectedRow = ui->audioLists->selectionModel()->selectedIndexes().at(0).row();
        MediaItem selectedItem = m_application->mainWindow()->m_audioListsModel->mediaItemAt(selectedRow);
        if (selectedItem.url.startsWith("savedlists://")) {
            m_application->savedListsManager()->showAudioSavedListSettings();
        } else if (selectedItem.url.startsWith("semantics://recent") ||
            selectedItem.url.startsWith("semantics://frequent") ||
            selectedItem.url.startsWith("semantics://highest")) {
            m_application->mainWindow()->m_mediaListSettings->showMediaListSettings();
        }
    }
}
