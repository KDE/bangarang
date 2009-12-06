/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andreas Marschke (xxtjaxx@gmail.com)
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

#ifndef VIDEOSETTINGS_H
#define VIDEOSETTINGS_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QSlider>
#include <QLabel>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>

#include <KPushButton>
#include <KButtonGroup>

#include <phonon/videowidget.h>

/**
 * @short Video Settings
 * @author Andreas Marschke xxtjaxx@gmail.com
 * @version 0.1
 **/
using namespace Phonon;

class VideoSettings : public QWidget
{
    Q_OBJECT
public:

    /**
     * Default constructor
     **/
    VideoSettings(VideoWidget *videoWidget ,QWidget *parent);

    /**
     * Destructor
     */
    virtual ~VideoSettings();
    void setHideAction(QAction *hideAction);

private:
    //Qt
    QVBoxLayout *m_layout;
    QGridLayout *colorSettings_layout;
    QVBoxLayout *sizeSettings_layout;
    QHBoxLayout *button_layout;

    //to have to uninterferring radio groups 
    QWidget *videoColorWidget;
    QSlider *brightnessSlider;
    QSlider *contrastSlider;
    QSlider *hueSlider;
    QSlider *saturationSlider;

    QLabel *brightnessLabel;
    QLabel *contrastLabel;
    QLabel *hueLabel;
    QLabel *saturationLabel;

    QWidget *aspectRatio_Widget;
    QVBoxLayout *aspectRatio_layout;
    QLabel *aspectRatioLabel;
    QRadioButton *aspectRatioAuto;
    QRadioButton *aspectRatioWidget;
    QRadioButton *aspectRatio4_3;
    QRadioButton *aspectRatio16_9;

    QWidget *scaleMode_Widget;
    QVBoxLayout *scaleMode_layout;
    QLabel *scaleModeLabel;
    QRadioButton *scaleModeFitInView;
    QRadioButton *scaleModeScaleAndCrop;

    KPushButton *restoreButton;
    KPushButton *hideButton;
    
    VideoWidget *videoWidget;
    void setupConnections();
    void setScaleSettingsEnabled(bool enabled);
        
signals:
    void brightnessChanged(qreal num);
    void contrastChanged(qreal num);
    void hueChanged(qreal num);
    void saturationChanged(qreal num);
    void okClicked();
private slots:
    void setBrightness(int ch);
    void setContrast(int ch);
    void setHue(int ch);
    void setSaturation(int ch);
    
    void setAspectRatioAuto(bool checked);
    void setAspectRatioWidget(bool checked);
    void setAspectRatio4_3(bool checked);
    void setAspectRatio16_9(bool checked);

    void setScaleModeFitInView(bool checked);
    void setScaleModeScaleAndCrop(bool checked);

    void restoreClicked();

};

#endif // VIDEOSETTINGS_H
