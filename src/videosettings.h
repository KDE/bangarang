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
#include <QGroupBox>
#include <QRadioButton>
#include <KConfigGroup>
#include <KUrl>


#include <phonon/videowidget.h>
#include <phonon/mediacontroller.h>

class MainWindow;
class BangarangApplication;

namespace Ui {
   class MainWindowClass;
};

/**
 * This class provides Settings for the VideoWidget 
 * such as it AspectRatio and Colorization
 * @short Video Settings
 * @author Andreas Marschke xxtjaxx@gmail.com
 * @version 0.1
 **/
using namespace Phonon;

class VideoSettings : public QObject
{
    Q_OBJECT
public:

    /**
     * Default constructor
     **/
    VideoSettings(MainWindow* parent, VideoWidget* widget);

    /**
     * Destructor
     */
    virtual ~VideoSettings();
    
    void setMediaController( MediaController *mctrl );
    MediaController *mediaController() { return m_mediaController; }
    
    void restoreVideoSettings(KConfigGroup* config);
    void saveVideoSettings(KConfigGroup* config);
    
public slots: //it should also be possible to use them as normal functions
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

    void restoreDefaults();

    void updateSubtitleCombo();
    void updateAngleCombo(int selected = -1, bool afterUpdate = false);
    void readExternalSubtitles(const KUrl &subtitleUrl);

private:
    void connectAngleCombo();
    void disconnectAngleCombo();
    void connectSubtitleCombo();
    void disconnectSubtitleCombo();
    
    Ui::MainWindowClass *ui;
    BangarangApplication * m_application;
    MediaController * m_mediaController;
    VideoWidget *m_videoWidget;
    QStringList m_extSubtitleTimes;
    QStringList m_extSubtitles;
    QStringList m_extSubtitleFiles;
    void setupConnections();
    QStringList findSubtitleFiles(const KUrl &url);
        
private slots:  
    void updateAngles(int no);
    void updateSubtitles();
    void showExternalSubtitles(qint64 time);

};

#endif // VIDEOSETTINGS_H
