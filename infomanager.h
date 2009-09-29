#ifndef INFOMANAGER_H
#define INFOMANAGER_H

#include <QObject>

namespace Ui
{
    class MainWindowClass;
}
class MainWindow;
class MediaItemModel;
class MediaItemDelegate;
class MediaIndexer;

class InfoManager : public QObject
{
    Q_OBJECT
    
    public:
        InfoManager(MainWindow * parent);
        ~InfoManager();
        void loadInfoView();
        
        MediaItemModel *m_infoMediaItemsModel;
        MediaItemDelegate *m_infoItemDelegate;
        
    public slots:
        void saveInfoView();
        
        
    private:
        MainWindow *m_parent; 
        Ui::MainWindowClass *ui;
        MediaIndexer *m_mediaIndexer;
        QVariant commonValue(QString field);
        QStringList valueList(QString field);
        void saveMusicInfoToFiles();
        void saveInfoToMediaModel();
        QList<int> m_rows;
        void showAudioType(int index);
        void showAudioFields();
        void showAudioMusicFields();
        void showAudioStreamFields();
        void showVideoType(int index);
        void showVideoFields();
        void showVideoMovieFields();
        void showVideoSeriesFields();
        
    private slots:
        void mediaTypeChanged(int index);
};
#endif //INFOMANAGER_H