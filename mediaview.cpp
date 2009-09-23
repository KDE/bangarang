#include <KIcon>
#include <QMenu>
#include "mediaview.h"
#include "mainwindow.h"
#include "platform/mediaitemmodel.h"

MediaView::MediaView(QWidget * parent):QTreeView (parent) 
{
    
}

MediaView::~MediaView() 
{
}

void MediaView::setMainWindow(MainWindow * mainWindow)
{
    m_mainWindow = mainWindow;

    //Setup context menu actions
    playAllAction = new QAction(KIcon("media-playback-start"), tr("Play all"), this);
    connect(playAllAction, SIGNAL(triggered()), m_mainWindow, SLOT(playAll()));
    playSelectedAction = new QAction(KIcon("media-playback-start"), tr("Play selected"), this);
    connect(playSelectedAction, SIGNAL(triggered()), m_mainWindow, SLOT(playSelected()));    
    addSelectedToPlayListAction = new QAction(KIcon("mail-mark-notjunk"), tr("Add to playlist"), this);
    connect(addSelectedToPlayListAction, SIGNAL(triggered()), m_mainWindow, SLOT(addSelectedToPlaylist()));    
    removeSelectedToPlayListAction = new QAction(KIcon(), tr("Remove from playlist"), this);
    connect(removeSelectedToPlayListAction, SIGNAL(triggered()), m_mainWindow, SLOT(removeSelectedFromPlaylist()));    
}

void MediaView::contextMenuEvent(QContextMenuEvent * event)
{
    if (selectionModel()->selectedIndexes().count() != 0) {
        QModelIndex index = selectionModel()->selectedIndexes().at(0);
        QString type = index.data(MediaItem::TypeRole).toString();
        if ((type != "Action") && (type != "Message")) {
            QMenu menu(this);
            if ((type == "Audio") ||(type == "Video") || (type == "Image")) {
                menu.addAction(addSelectedToPlayListAction);
                menu.addAction(removeSelectedToPlayListAction);
            }
            menu.addSeparator();
            menu.addAction(playSelectedAction);
            menu.addAction(playAllAction);
            menu.exec(event->globalPos());
        }
    }
}

