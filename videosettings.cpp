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
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QButtonGroup>
#include <QRadioButton>
#include <QAbstractButton>
#include <KButtonGroup>
#include <QLabel>
#include <KLocale>
#include <QAction>

#include <phonon/videowidget.h>

#include <KPushButton>

using namespace Phonon;

VideoSettings::VideoSettings(VideoWidget *widget ,
			     QWidget *parent): QWidget(parent) ,
					       m_layout(0) ,
					       colorSettings_layout(0) ,
					       sizeSettings_layout(0) ,
					       button_layout(0) ,
					       brightnessSlider(0) ,
					       contrastSlider(0) ,
					       hueSlider(0) ,
					       saturationSlider(0) ,
					       brightnessLabel(0) ,
					       contrastLabel(0) ,
					       hueLabel(0) ,
					       saturationLabel(0) ,
					       aspectRatio_Widget(0) ,
					       aspectRatio_layout(0) ,
					       aspectRatioLabel(0) ,
					       aspectRatioAuto(0) ,
					       aspectRatioWidget(0) ,
					       aspectRatio4_3(0) ,
					       aspectRatio16_9(0) ,
					       scaleMode_Widget(0) ,
					       scaleMode_layout(0) ,
					       scaleModeLabel(0) ,
					       scaleModeFitInView(0) ,
					       scaleModeScaleAndCrop(0) ,
					       restoreButton(0) ,
					       videoWidget(0)
{
  m_layout = new QVBoxLayout(this) ;
  colorSettings_layout = new QGridLayout();
  videoColorWidget = new QWidget();
  button_layout = new QHBoxLayout();
  
  brightnessLabel  = new QLabel(i18n("Brightness"));
  brightnessSlider = new QSlider();

  brightnessSlider->setMaximum(99);
  brightnessSlider->setMinimum(-99);
  brightnessSlider->setOrientation(Qt::Horizontal);
  brightnessSlider->setValue(int(widget->brightness() * 100));
  colorSettings_layout->addWidget(brightnessLabel , 0,0);
  colorSettings_layout->addWidget(brightnessSlider, 0,1);

  contrastLabel= new QLabel(i18n("Contrast"));
  contrastSlider = new QSlider();

  contrastSlider->setMaximum(99);
  contrastSlider->setMinimum(-99);
  contrastSlider->setOrientation(Qt::Horizontal);
  contrastSlider->setValue(int(widget->contrast() * 100));
  colorSettings_layout->addWidget(contrastLabel , 1,0);
  colorSettings_layout->addWidget(contrastSlider, 1,1);

  hueLabel= new QLabel(i18n("Hue"));
  hueSlider = new QSlider();

  hueSlider->setMaximum(99);
  hueSlider->setMinimum(-99);
  hueSlider->setOrientation(Qt::Horizontal);
  hueSlider->setValue(int(widget->hue() * 100));
  colorSettings_layout->addWidget(hueLabel , 2,0);
  colorSettings_layout->addWidget(hueSlider, 2,1);

  saturationLabel= new QLabel(i18n("Saturation"));
  saturationSlider = new QSlider();

  saturationSlider->setMaximum(99);
  saturationSlider->setMinimum(-99);
  saturationSlider->setOrientation(Qt::Horizontal);
  saturationSlider->setValue(int(widget->saturation() * 100));
  colorSettings_layout->addWidget(saturationLabel , 3,0);
  colorSettings_layout->addWidget(saturationSlider, 3,1);
  
  videoColorWidget->setLayout(colorSettings_layout);
  
  sizeSettings_layout = new QVBoxLayout();
  aspectRatio_Widget = new QWidget();
  aspectRatio_layout = new QVBoxLayout();
  aspectRatioLabel = new QLabel(i18n("Aspect Ratio Settings"));
  aspectRatio_layout->addWidget(aspectRatioLabel);
  aspectRatioAuto = new QRadioButton(i18n("Automatic"));
  aspectRatio_layout->addWidget(aspectRatioAuto);
  aspectRatio4_3 = new QRadioButton(i18n("4:3"));
  aspectRatio_layout->addWidget(aspectRatio4_3);
  aspectRatio16_9 = new QRadioButton(i18n("16:9"));
  aspectRatio_layout->addWidget(aspectRatio16_9);
  aspectRatioWidget = new QRadioButton(i18n("Fit"));
  aspectRatio_layout->addWidget(aspectRatioWidget);
  aspectRatio_Widget->setLayout(aspectRatio_layout);
  
  scaleMode_Widget = new QWidget();
  scaleMode_layout = new QVBoxLayout();

  scaleModeLabel = new QLabel(i18n("Scaling Mode")) ;
  scaleMode_layout->addWidget(scaleModeLabel);
  scaleModeFitInView = new QRadioButton(i18n("Scale to fit"));
  scaleMode_layout->addWidget(scaleModeFitInView);
  scaleModeScaleAndCrop = new QRadioButton(i18n("Scale and crop"));
  scaleMode_layout->addWidget(scaleModeScaleAndCrop);
  scaleMode_Widget->setLayout(scaleMode_layout);  
  
  sizeSettings_layout->addWidget(aspectRatio_Widget);
  sizeSettings_layout->addWidget(scaleMode_Widget);
  
  
  restoreButton = new KPushButton();
  restoreButton->setIcon(KIcon("view-restore"));
  restoreButton->setText(i18n("Restore Defaults"));
  hideButton = new KPushButton();
  hideButton->setText(i18n("Hide"));
  button_layout->addWidget(restoreButton);
  button_layout->addWidget(hideButton);
  
  m_layout->addWidget(videoColorWidget);
  m_layout->addLayout(sizeSettings_layout);
  m_layout->addLayout(button_layout);
  videoWidget = widget;
  
  if(widget->aspectRatio() == VideoWidget::AspectRatioAuto)
    aspectRatioAuto->setChecked(true);
  if(widget->aspectRatio() == VideoWidget::AspectRatioWidget)
    aspectRatioWidget->setChecked(true);
  if(widget->aspectRatio() == VideoWidget::AspectRatio4_3)
    aspectRatio4_3->setChecked(true);
  if(widget->aspectRatio() == VideoWidget::AspectRatio16_9)
    aspectRatio16_9->setChecked(true);

  if(widget->scaleMode() == VideoWidget::FitInView)
    scaleModeFitInView->setChecked(true);
  if(widget->scaleMode() == VideoWidget::ScaleAndCrop)
    scaleModeScaleAndCrop->setChecked(true);

  setLayout(m_layout);
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
  setupConnections();
}
void
VideoSettings::setupConnections()
{
  connect(brightnessSlider,SIGNAL(valueChanged(int)),this,SLOT(setBrightness(int)));
  connect(contrastSlider,SIGNAL(valueChanged(int)),this,SLOT(setContrast(int)));
  connect(hueSlider,SIGNAL(valueChanged(int)),this,SLOT(setHue(int)));
  connect(saturationSlider,SIGNAL(valueChanged(int)),this,SLOT(setSaturation(int)));
  
  connect(aspectRatioAuto,SIGNAL(toggled(bool)),
	  this,SLOT(setAspectRatioAuto(bool)));
  connect(aspectRatioWidget,SIGNAL(toggled(bool)),
	  this,SLOT(setAspectRatioWidget(bool)));
  connect(aspectRatio4_3,SIGNAL(toggled(bool)),
	  this,SLOT(setAspectRatio4_3(bool)));
  connect(aspectRatio16_9,SIGNAL(toggled(bool)),
	  this,SLOT(setAspectRatio16_9(bool)));

  connect(scaleModeFitInView,SIGNAL(toggled(bool)),
	  this,SLOT(setScaleModeFitInView(bool)));
  connect(scaleModeScaleAndCrop,SIGNAL(toggled(bool)),
	  this,SLOT(setScaleModeScaleAndCrop(bool)));
 
  connect(restoreButton,SIGNAL(clicked()),this,SLOT(restoreClicked()));
}

VideoSettings::~VideoSettings()
{

}

void VideoSettings::setBrightness(int ch)
{
  videoWidget->setBrightness(qreal(ch)/100);
}

void VideoSettings::setContrast(int ch)
{
  videoWidget->setContrast(qreal(ch)/100);
}

void VideoSettings::setHue(int ch)
{
  videoWidget->setHue(qreal(ch)/100);
}

void VideoSettings::setSaturation(int ch)
{
  videoWidget->setSaturation(qreal(ch)/100);
}

void VideoSettings::setAspectRatioAuto(bool checked)
{
  Q_UNUSED(checked);
  videoWidget->setAspectRatio(VideoWidget::AspectRatioAuto);
  setScaleSettingsEnabled(true);
}

void VideoSettings::setAspectRatio4_3(bool checked)
{
  Q_UNUSED(checked);
  videoWidget->setAspectRatio(VideoWidget::AspectRatio4_3);
  setScaleSettingsEnabled(true);
}

void VideoSettings::setAspectRatio16_9(bool checked)
{
  Q_UNUSED(checked);
  videoWidget->setAspectRatio(VideoWidget::AspectRatio16_9);
  setScaleSettingsEnabled(true);
}

void VideoSettings::setAspectRatioWidget(bool checked)
{
    Q_UNUSED(checked);
    videoWidget->setAspectRatio(VideoWidget::AspectRatioWidget);
    setScaleSettingsEnabled(false);
}

void VideoSettings::setScaleModeFitInView(bool checked)
{
  Q_UNUSED(checked);
  videoWidget->setScaleMode(VideoWidget::FitInView);
}

void VideoSettings::setScaleModeScaleAndCrop(bool checked)
{
  Q_UNUSED(checked);
  videoWidget->setScaleMode(VideoWidget::ScaleAndCrop);
}

void VideoSettings::restoreClicked()
{
  videoWidget->setBrightness(0);
  videoWidget->setContrast(0);
  videoWidget->setHue(0);
  videoWidget->setSaturation(0);

  videoWidget->setAspectRatio(VideoWidget::AspectRatioAuto);
  videoWidget->setScaleMode(VideoWidget::FitInView);  
  
  brightnessSlider->setValue(0);
  contrastSlider->setValue(0);
  hueSlider->setValue(0);
  saturationSlider->setValue(0);
  aspectRatioAuto->setChecked(true);
  scaleModeFitInView->setChecked(true);
}

void VideoSettings::setHideAction(QAction * hideAction)
{
    connect(hideButton, SIGNAL(clicked()), hideAction, SLOT(trigger()));
}

void VideoSettings::setScaleSettingsEnabled(bool enabled)
{
    scaleModeFitInView->setEnabled(enabled);
    scaleModeScaleAndCrop->setEnabled(enabled);
}
#include "moc_videosettings.cpp"
