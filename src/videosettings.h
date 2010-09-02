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

#include <QPushButton>
#include <QSlider>
#include <QRadioButton>


#include <phonon/videowidget.h>

class MainWindow;

/**
 * This class provides Settings for the VideoWidget 
 * such as it AspectRatio and Colorization
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
    VideoSettings(VideoWidget* widget, MainWindow* mainwindow);

    /**
     * Destructor
     */
    virtual ~VideoSettings();
    void setHideAction(QAction *hideAction);

private:
    //to have to uninterferring radio groups 
    QSlider *m_brightnessSlider;
    QSlider *m_contrastSlider;
    QSlider *m_hueSlider;
    QSlider *m_saturationSlider;

    QRadioButton *m_aspectRatioAuto;
    QRadioButton *m_aspectRatioWidget;
    QRadioButton *m_aspectRatio4_3;
    QRadioButton *m_aspectRatio16_9;

    QRadioButton *m_scaleModeFitInView;
    QRadioButton *m_scaleModeScaleAndCrop;

    QPushButton *m_restoreButton;
    QPushButton *m_hideButton;
    
    VideoWidget *m_videoWidget;
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
    
    void setSubtitle(int idx);
    void setAngle(int idx);

    void setScaleModeFitInView(bool checked);
    void setScaleModeScaleAndCrop(bool checked);

    void restoreClicked();

};

#endif // VIDEOSETTINGS_H
