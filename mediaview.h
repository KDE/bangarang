#ifndef MEDIAVIEW_H
#define MEDIAVIEW_H

#include <QTreeView>
#include <QAction>

class MainWindow;
class MediaView : public QTreeView
{
    Q_OBJECT
    public:
        MediaView(QWidget * parent = 0);
        ~MediaView();
        void setMainWindow(MainWindow * mainWindow);

    protected:
        void contextMenuEvent (QContextMenuEvent * event);
        
    private:
        MainWindow * m_mainWindow;
        QAction * playAllAction;
        QAction * playSelectedAction;   
        QAction * addSelectedToPlayListAction;
        QAction * removeSelectedToPlayListAction;
};
#endif // MEDIAVIEW_H
