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

#include "videosettings.h"
#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "bangarangapplication.h"
#include "actionsmanager.h"

#include <KFileDialog>
#include <KDebug>
#include <KEncodingProber>

#include <phonon/videowidget.h>

using namespace Phonon;

VideoSettings::VideoSettings(MainWindow *parent, VideoWidget *widget) : QObject( parent )
{
  //member vars
  m_application = (BangarangApplication *) KApplication::kApplication();
  m_mediaController = NULL;
  ui = parent->ui;
  m_videoWidget = widget;
  
  ui->angleSelectionHolder->setEnabled(false);
  ui->subtitleSelectionHolder->setEnabled(false);
  
  setupConnections();
}

void VideoSettings::setupConnections()
{
  connect(ui->angleSelection, SIGNAL(currentIndexChanged(int)),this,SLOT(setAngle(int)));
  connect(ui->subtitleSelection, SIGNAL(currentIndexChanged(int)),this,SLOT(setSubtitle(int)));
    
  connect(ui->brightnessSlider,SIGNAL(valueChanged(int)),this,SLOT(setBrightness(int)));
  connect(ui->contrastSlider,SIGNAL(valueChanged(int)),this,SLOT(setContrast(int)));
  connect(ui->hueSlider,SIGNAL(valueChanged(int)),this,SLOT(setHue(int)));
  connect(ui->saturationSlider,SIGNAL(valueChanged(int)),this,SLOT(setSaturation(int)));
  
  connect(ui->aspectRatioAuto,SIGNAL(toggled(bool)), this,SLOT(setAspectRatioAuto(bool)));
  connect(ui->aspectRatioWidget,SIGNAL(toggled(bool)), this,SLOT(setAspectRatioWidget(bool)));
  connect(ui->aspectRatio4_3,SIGNAL(toggled(bool)), this,SLOT(setAspectRatio4_3(bool)));
  connect(ui->aspectRatio16_9,SIGNAL(toggled(bool)), this,SLOT(setAspectRatio16_9(bool)));

  connect(ui->scaleModeFitInView,SIGNAL(toggled(bool)), this,SLOT(setScaleModeFitInView(bool)));
  connect(ui->scaleModeScaleAndCrop,SIGNAL(toggled(bool)), this,SLOT(setScaleModeScaleAndCrop(bool)));
 
  connect(ui->restoreDefaultVideoSettings, SIGNAL(clicked()), this, SLOT(restoreDefaults()));
  connect(ui->hideVideoSettings, SIGNAL(clicked()), m_application->actionsManager()->action("show_video_settings"), SLOT(trigger()));

  connect(m_application->playlist()->nowPlayingModel(), SIGNAL(mediaListChanged()), this, SLOT(updateSubtitles()));
}

VideoSettings::~VideoSettings()
{

}

void VideoSettings::setMediaController(MediaController* mctrl)
{
    m_mediaController = mctrl;
    updateAngles(m_mediaController->availableAngles());
    updateSubtitles();
    connect(m_mediaController, SIGNAL(angleChanged(int)), this, SLOT(updateAngleCombo(int)));
    connect(m_mediaController, SIGNAL(availableAnglesChanged(int)), this, SLOT(updateAngles(int)));
    connect(m_mediaController, SIGNAL(availableSubtitlesChanged()), this, SLOT(updateSubtitles()));
    connectAngleCombo();
    connectSubtitleCombo();
}


void VideoSettings::setBrightness(int ch)
{
  m_videoWidget->setBrightness(qreal(ch)/100);
}

void VideoSettings::setContrast(int ch)
{
  m_videoWidget->setContrast(qreal(ch)/100);
}

void VideoSettings::setHue(int ch)
{
  m_videoWidget->setHue(qreal(ch)/100);
}

void VideoSettings::setSaturation(int ch)
{
  m_videoWidget->setSaturation(qreal(ch)/100);
}

void VideoSettings::setAspectRatioAuto(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setAspectRatio(VideoWidget::AspectRatioAuto);
  ui->scalingModeHolder->setEnabled(true);
}

void VideoSettings::setAspectRatio4_3(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setAspectRatio(VideoWidget::AspectRatio4_3);
  ui->scalingModeHolder->setEnabled(true);
}

void VideoSettings::setAspectRatio16_9(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setAspectRatio(VideoWidget::AspectRatio16_9);
  ui->scalingModeHolder->setEnabled(true);
}

void VideoSettings::setAspectRatioWidget(bool checked)
{
    Q_UNUSED(checked);
    m_videoWidget->setAspectRatio(VideoWidget::AspectRatioWidget);
    ui->scalingModeHolder->setEnabled(false);
}

void VideoSettings::setScaleModeFitInView(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setScaleMode(VideoWidget::FitInView);
}

void VideoSettings::setScaleModeScaleAndCrop(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setScaleMode(VideoWidget::ScaleAndCrop);
}

void VideoSettings::restoreDefaults()
{
  m_videoWidget->setBrightness(0);
  m_videoWidget->setContrast(0);
  m_videoWidget->setHue(0);
  m_videoWidget->setSaturation(0);

  m_videoWidget->setAspectRatio(VideoWidget::AspectRatioAuto);
  m_videoWidget->setScaleMode(VideoWidget::FitInView);  
  
  ui->brightnessSlider->setValue(0);
  ui->contrastSlider->setValue(0);
  ui->hueSlider->setValue(0);
  ui->saturationSlider->setValue(0);
  ui->aspectRatioAuto->setChecked(true);
  ui->scaleModeFitInView->setChecked(true);
}

void VideoSettings::setAngle(int idx)
{
    if ( idx < 0 || idx > m_mediaController->availableAngles() )
        return;
    m_mediaController->setCurrentAngle(idx);
}

void VideoSettings::setSubtitle(int idx)
{
    if ( idx < 0 )
        return;
    int sidx = ui->subtitleSelection->itemData(idx).toInt();
    if (sidx >= -1) {
        SubtitleDescription sub = SubtitleDescription::fromIndex(sidx);
        m_mediaController->setCurrentSubtitle(sub);
        if (sidx == -1) {
            readExternalSubtitles(KUrl());
        }
    } else {
        //Read external subtitles
        KUrl subtitleUrl(m_extSubtitleFiles.at(-sidx - 100));
        readExternalSubtitles(subtitleUrl);
    }
}

void VideoSettings::updateAngles(int no)
{
    disconnectAngleCombo();
    QComboBox *cb = ui->angleSelection;
    ui->angleSelectionHolder->setEnabled( no > 1 ); //1 is always set, thats no selection
    cb->clear();
    for (int i = 1; i <= no; i++ ) {
        cb->addItem(QString("%1").arg(i));
    }
    updateAngleCombo(m_mediaController->currentAngle(), true); //will reconnect angle combo
}

void VideoSettings::updateAngleCombo(int selected, bool afterUpdate)
{
  disconnectAngleCombo();
  if ( selected < 0 )
      selected = m_mediaController->currentAngle();
  if ( ui->angleSelection->count() < selected ) {
      if ( !afterUpdate ) { //try to update the angles if not just done
          updateAngles(m_mediaController->availableAngles());
      }
      else
          connectAngleCombo();
      return;
  }
  ui->angleSelection->setCurrentIndex(selected);
  connectAngleCombo();
}

void VideoSettings::updateSubtitles()
{
    disconnectSubtitleCombo();
    QComboBox *cb = ui->subtitleSelection;
    QList<SubtitleDescription> subs = m_mediaController->availableSubtitles();
    int no = subs.count();
    ui->subtitleSelectionHolder->setEnabled( no > 0 ); //can have no subtitles at all
    cb->clear();
    for (int i = 0; i < no; i++ ) { //no subtitles + disable subtitle
        if ( i == 0 ) {
            cb->addItem(i18n("Disable"), QVariant( -1 ));
            continue;
        }
        SubtitleDescription sub = subs.at((i - 1)); //-1 because of the first (disable) item
        QString descr = sub.description().trimmed();
        QString more = descr.isEmpty() ? QString() : QString(" (%1)").arg(descr);
        QString name = sub.name().trimmed();
        QString trans_name =  m_application->locale()->languageCodeToName( name );
        QString display_name = trans_name.isEmpty() ? name : trans_name;
        cb->addItem( display_name + more, QVariant( sub.index() ));
    }

    if (m_application->playlist()->nowPlayingModel()->rowCount() > 0) {
        MediaItem nowPlayingItem = m_application->playlist()->nowPlayingModel()->mediaItemAt(0);
        if (nowPlayingItem.type == "Video") {
            //Search for Subtitles
            m_extSubtitleFiles = findSubtitleFiles(KUrl(nowPlayingItem.url));
            if (!m_extSubtitleFiles.isEmpty()) {
                if (cb->model()->rowCount() == 0) {
                    cb->insertItem(0, i18n("Disable"), QVariant(-1));
                }
                for (int i = 0; i < m_extSubtitleFiles.count(); i++) {
                    KUrl fileUrl(m_extSubtitleFiles.at(i));
                    cb->addItem(fileUrl.fileName(), QVariant(-100 - i));
                    cb->model()->setData(cb->model()->index(cb->model()->rowCount()-1, 0), fileUrl.path(), Qt::ToolTipRole);
                }
                ui->subtitleSelectionHolder->setEnabled(true);
            }
        } else {
            ui->extSubtitle->hide();
        }
    }
    updateSubtitleCombo(); //will reconnect subtitle combo
}

void VideoSettings::updateSubtitleCombo()
{
    disconnectSubtitleCombo();
    int curIdx = m_mediaController->currentSubtitle().index();
    QComboBox *cb = ui->subtitleSelection;
    for (int i = 0; i < cb->count(); i++) {
        if (cb->itemData(i).toInt() == curIdx) {
            cb->setCurrentIndex(i);
            break;
        }
    }
    connectSubtitleCombo();
}

void VideoSettings::restoreVideoSettings(KConfigGroup* config)
{
  ui->brightnessSlider->setValue(config->readEntry("VideoBrightness", 0));
  ui->contrastSlider->setValue(config->readEntry("VideoContrast", 0));
  ui->hueSlider->setValue(config->readEntry("VideoHue", 0));
  ui->saturationSlider->setValue(config->readEntry("VideoSaturation", 0));
  
  VideoWidget::AspectRatio ratio = (VideoWidget::AspectRatio) config->readEntry("VideoAspectRatio", (int) VideoWidget::AspectRatioAuto);
  VideoWidget::ScaleMode scaleMode = (VideoWidget::ScaleMode) config->readEntry<int>("VideoScaleMode", (int) VideoWidget::FitInView);
  
  ui->aspectRatioAuto->setChecked(ratio == VideoWidget::AspectRatioAuto);
  ui->aspectRatioWidget->setChecked(ratio == VideoWidget::AspectRatioWidget);
  ui->aspectRatio4_3->setChecked(ratio == VideoWidget::AspectRatio4_3);
  ui->aspectRatio16_9->setChecked(ratio == VideoWidget::AspectRatio16_9);

  ui->scaleModeFitInView->setChecked(scaleMode == VideoWidget::FitInView);
  ui->scaleModeScaleAndCrop->setChecked(scaleMode == VideoWidget::ScaleAndCrop);
  
  ui->scalingModeHolder->setEnabled(ratio != VideoWidget::AspectRatioWidget);
}

void VideoSettings::saveVideoSettings(KConfigGroup* config)
{
  config->writeEntry("VideoBrightness", (int) (m_videoWidget->brightness() * 100 ));
  config->writeEntry("VideoContrast", (int) (m_videoWidget->contrast() * 100 ));
  config->writeEntry("VideoHue", (int) (m_videoWidget->hue() * 100 ));
  config->writeEntry("VideoSaturation", (int) (m_videoWidget->saturation() * 100 ));
  config->writeEntry("VideoAspectRatio", (int) m_videoWidget->aspectRatio() );
  config->writeEntry("VideoScaleMode", (int) m_videoWidget->scaleMode() );

}

void VideoSettings::connectAngleCombo()
{
    connect(ui->angleSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setAngle(int)));
}

void VideoSettings::disconnectAngleCombo()
{
    disconnect(ui->angleSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setAngle(int)));
}

void VideoSettings::connectSubtitleCombo()
{
    connect(ui->subtitleSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setSubtitle(int)));
}

void VideoSettings::disconnectSubtitleCombo()
{
    disconnect(ui->subtitleSelection, SIGNAL(currentIndexChanged(int)), this, SLOT(setSubtitle(int)));
}

QStringList VideoSettings::findSubtitleFiles(const KUrl &url)
{
    if (!url.isLocalFile()) {
        return QStringList();
    }

    QDir dir(url.directory(KUrl::AppendTrailingSlash));
    QFileInfoList files = dir.entryInfoList(QStringList("*.srt"), QDir::Files);
    QFileInfoList dirs = dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (int i = 0; i < dirs.count(); i++) {
        QDir subDir(dirs.at(i).absoluteFilePath());
        QFileInfoList subDirFiles = subDir.entryInfoList(QStringList("*.srt"), QDir::Files);
        files.append(subDirFiles);
    }
    QStringList subtitleFiles;
    for (int i = 0; i < files.count(); i++) {
        subtitleFiles.append(files.at(i).absoluteFilePath());
    }
    return subtitleFiles;
}

void VideoSettings::readExternalSubtitles(const KUrl &subtitleUrl)
{
    m_extSubtitleTimes.clear();
    m_extSubtitles.clear();
    if (subtitleUrl.isLocalFile()) {
        QFile file(subtitleUrl.path());
        if (file.open(QFile::ReadOnly)) {
            QTextStream in(&file);
            bool lastLineWasTime = false;
            kDebug() << "Loading subtitles...";
            QString encodingTest;
            QByteArray encodingName;
             while (!in.atEnd()) {
                 QString line = in.readLine().trimmed();
                 //Collect enough data to attempt to perform encoding check
                 if (encodingName.isEmpty() && encodingTest.length() > 200) {
                     KEncodingProber prober(KEncodingProber::Universal);
                     KEncodingProber::ProberState result = prober.feed(encodingTest.toAscii());
                     if (result != KEncodingProber::NotMe) {
                         if (prober.confidence() > 0.5 ) {
                             encodingName = prober.encoding().toLower();
                         } else if ((prober.confidence() <= 0.3 || encodingName != "utf-8")
                             && QTextCodec::codecForLocale()->name().toLower() != "utf-8") {
                             encodingName = QTextCodec::codecForLocale()->name().toLower();
                         }
                         if (!encodingName.isEmpty()) {
                             for (int i = 0; i < m_extSubtitles.count(); i++) {
                                 QString encSubtitle = m_extSubtitles.at(i);
                                 encSubtitle = QTextCodec::codecForName(encodingName)->toUnicode(encSubtitle.toAscii());
                                 kDebug() << encSubtitle;
                                 m_extSubtitles.replace(i, encSubtitle);
                             }
                         }
                     }
                 }
                 if (!encodingName.isEmpty()) {
                     line = QTextCodec::codecForName(encodingName)->toUnicode(line.toAscii());
                 }
                 if (line.contains("-->")) {
                     QStringList times = line.split("-->");
                     if (times.count() != 2) {
                         break;
                     }
                     QTime startTime = QTime::fromString(times.at(0).trimmed(), "hh:mm:ss,zzz");
                     QTime endTime = QTime::fromString(times.at(1).trimmed(), "hh:mm:ss,zzz");
                     if (!startTime.isValid() || !endTime.isValid()) {
                         break;
                     }
                     int start = QTime(0,0,0,0).msecsTo(startTime);
                     int end = QTime(0,0,0,0).msecsTo(endTime);
                     m_extSubtitleTimes.append(QString("%1,%2").arg(start).arg(end));
                     lastLineWasTime = true;
                 } else if (lastLineWasTime) {
                     QString subtitle = line;
                     while (!in.atEnd() && !line.isEmpty()) {
                         line = in.readLine().trimmed();
                         if (subtitle.contains("<i>") || line.contains("<i>") || line.contains("</i>")) {
                             subtitle.append(QString("<br>%1").arg(line));
                         } else {
                             subtitle.append(QString("\n%1").arg(line));
                         }
                     }
                     m_extSubtitles.append(subtitle.trimmed());
                     if (encodingName.isEmpty()) {
                         encodingTest.append(subtitle);
                     }

                     lastLineWasTime = false;
                 }

             }
            kDebug() << "Finished loading subtitles....";
        }
    }
    if (!m_extSubtitleTimes.isEmpty()) {
        connect(m_application->playlist()->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(showExternalSubtitles(qint64)));
        m_application->playlist()->mediaObject()->setTickInterval(100);
    } else {
        disconnect(m_application->playlist()->mediaObject(), SIGNAL(tick(qint64)), this, SLOT(showExternalSubtitles(qint64)));
        ui->extSubtitle->hide();
        m_application->playlist()->mediaObject()->setTickInterval(500);
    }
}

void VideoSettings::showExternalSubtitles(qint64 time)
{
    QString subtitle;

    //Find subtitle index corresponding to time
    for (int i = 0; i < m_extSubtitleTimes.count(); i++) {
        QStringList times = m_extSubtitleTimes.at(i).split(",");
        int startTime = times.at(0).trimmed().toInt();
        int endTime = times.at(1).trimmed().toInt();
        if (time >= startTime && time <= endTime) {
            subtitle = m_extSubtitles.at(i);
        }
        if (startTime > time) {
            break;
        }
    }

    //Update external subtitle display
    if (!subtitle.isEmpty()) {
        QFontMetrics fm(ui->extSubtitle->font());
        QString textForSize = subtitle;
        if (textForSize.contains("<i>")) {
            textForSize.remove("<i>");
            textForSize.remove("</i>");
            textForSize.replace("<br>", "\n");
            QFont font = ui->extSubtitle->font();
            font.setItalic(true);
            fm = QFontMetrics(font);
        }
        QSize textSize = fm.boundingRect(QRect(0, 0, ui->extSubtitle->maximumWidth(), fm.lineSpacing()),
                                         Qt::AlignCenter | Qt::TextWordWrap,
                                         textForSize).size();
        int top = ui->nowPlayingHolder->geometry().bottom() - 20 - textSize.height();
        int left = (ui->nowPlayingHolder->width() - textSize.width()) / 2;
        ui->extSubtitle->setGeometry(left - 8, top - 8, textSize.width() + 8, textSize.height() + 8);
        ui->extSubtitle->setText(subtitle);
        ui->extSubtitle->show();
        ui->extSubtitle->raise();
    } else {
        ui->extSubtitle->hide();
    }
}

#include "moc_videosettings.cpp"
