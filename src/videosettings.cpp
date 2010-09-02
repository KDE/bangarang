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

#include <phonon/videowidget.h>
#include "ui_mainwindow.h"

using namespace Phonon;

VideoSettings::VideoSettings(VideoWidget *widget, MainWindow *mainWindow): QWidget(mainWindow)
{
  //current settings more than 1 times used
  VideoWidget::AspectRatio ratio = widget->aspectRatio();
  VideoWidget::ScaleMode scaleMode = widget->scaleMode();
  
  //member variable initialization
  m_videoWidget = widget;
  m_brightnessSlider = mainWindow->ui->brightnessSlider;
  m_contrastSlider = mainWindow->ui->contrastSlider;
  m_hueSlider = mainWindow->ui->hueSlider;
  m_saturationSlider = mainWindow->ui->saturationSlider;
  
  m_aspectRatio16_9 = mainWindow->ui->aspectRatio16_9;
  m_aspectRatio4_3 = mainWindow->ui->aspectRatio4_3;
  m_aspectRatioAuto = mainWindow->ui->aspectRatioAuto;
  m_aspectRatioWidget = mainWindow->ui->aspectRatioWidget;
  
  m_scaleModeFitInView = mainWindow->ui->scaleModeFitInView;
  m_scaleModeScaleAndCrop = mainWindow->ui->scaleModeScaleAndCrop;
  
  m_brightnessSlider->setValue(int(widget->brightness() * 100));
  m_contrastSlider->setValue(int(widget->contrast() * 100));
  m_hueSlider->setValue(int(widget->hue() * 100));
  m_saturationSlider->setValue(int(widget->saturation() * 100));
  
  m_aspectRatioAuto->setChecked(ratio == VideoWidget::AspectRatioAuto);
  m_aspectRatioWidget->setChecked(ratio == VideoWidget::AspectRatioWidget);
  m_aspectRatio4_3->setChecked(ratio == VideoWidget::AspectRatio4_3);
  m_aspectRatio16_9->setChecked(ratio == VideoWidget::AspectRatio16_9);

  m_scaleModeFitInView->setChecked(scaleMode == VideoWidget::FitInView);
  m_scaleModeScaleAndCrop->setChecked(scaleMode == VideoWidget::ScaleAndCrop);

  setupConnections();
}

void
VideoSettings::setupConnections()
{
  connect(m_brightnessSlider,SIGNAL(valueChanged(int)),this,SLOT(setBrightness(int)));
  connect(m_contrastSlider,SIGNAL(valueChanged(int)),this,SLOT(setContrast(int)));
  connect(m_hueSlider,SIGNAL(valueChanged(int)),this,SLOT(setHue(int)));
  connect(m_saturationSlider,SIGNAL(valueChanged(int)),this,SLOT(setSaturation(int)));
  
  connect(m_aspectRatioAuto,SIGNAL(toggled(bool)), this,SLOT(setAspectRatioAuto(bool)));
  connect(m_aspectRatioWidget,SIGNAL(toggled(bool)), this,SLOT(setAspectRatioWidget(bool)));
  connect(m_aspectRatio4_3,SIGNAL(toggled(bool)), this,SLOT(setAspectRatio4_3(bool)));
  connect(m_aspectRatio16_9,SIGNAL(toggled(bool)), this,SLOT(setAspectRatio16_9(bool)));

  connect(m_scaleModeFitInView,SIGNAL(toggled(bool)), this,SLOT(setScaleModeFitInView(bool)));
  connect(m_scaleModeScaleAndCrop,SIGNAL(toggled(bool)), this,SLOT(setScaleModeScaleAndCrop(bool)));
 
  connect(m_restoreButton,SIGNAL(clicked()),this,SLOT(restoreClicked()));
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
  setScaleSettingsEnabled(true);
}

void VideoSettings::setAspectRatio4_3(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setAspectRatio(VideoWidget::AspectRatio4_3);
  setScaleSettingsEnabled(true);
}

void VideoSettings::setAspectRatio16_9(bool checked)
{
  Q_UNUSED(checked);
  m_videoWidget->setAspectRatio(VideoWidget::AspectRatio16_9);
  setScaleSettingsEnabled(true);
}

void VideoSettings::setAspectRatioWidget(bool checked)
{
    Q_UNUSED(checked);
    m_videoWidget->setAspectRatio(VideoWidget::AspectRatioWidget);
    setScaleSettingsEnabled(false);
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

void VideoSettings::restoreClicked()
{
  m_videoWidget->setBrightness(0);
  m_videoWidget->setContrast(0);
  m_videoWidget->setHue(0);
  m_videoWidget->setSaturation(0);

  m_videoWidget->setAspectRatio(VideoWidget::AspectRatioAuto);
  m_videoWidget->setScaleMode(VideoWidget::FitInView);  
  
  m_brightnessSlider->setValue(0);
  m_contrastSlider->setValue(0);
  m_hueSlider->setValue(0);
  m_saturationSlider->setValue(0);
  m_aspectRatioAuto->setChecked(true);
  m_scaleModeFitInView->setChecked(true);
}

void VideoSettings::setHideAction(QAction * hideAction)
{
    connect(m_hideButton, SIGNAL(clicked()), hideAction, SLOT(trigger()));
}

void VideoSettings::setScaleSettingsEnabled(bool enabled)
{
//    m_scaleModeHolder->setEnabled(enabled);
}

void VideoSettings::setAngle(int idx)
{

}

void VideoSettings::setSubtitle(int idx)
{

}

#include "moc_videosettings.cpp"
