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

#include <phonon/videowidget.h>

using namespace Phonon;

VideoSettings::VideoSettings(MainWindow *parent, VideoWidget *widget) : QObject( parent )
{
  //member vars
  m_application = (BangarangApplication *) KApplication::kApplication();
  ui = parent->ui;
  m_videoWidget = widget;
  
  setupConnections();
}

void
VideoSettings::setupConnections()
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
}

VideoSettings::~VideoSettings()
{

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

}

void VideoSettings::setSubtitle(int idx)
{

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


#include "moc_videosettings.cpp"
