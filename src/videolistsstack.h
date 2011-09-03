#ifndef VIDEOLISTSSTACK_H
#define VIDEOLISTSSTACK_H

#include <QWidget>

namespace Ui {
    class VideoListsStack;
}

#include "ui_videolistsstack.h"

class BangarangApplication;

class VideoListsStack : public QWidget
{
    Q_OBJECT

public:
    explicit VideoListsStack(QWidget *parent = 0);
    ~VideoListsStack();
    void enableTouch();

    Ui::VideoListsStack *ui;

private:
    BangarangApplication *m_application;

private slots:
    void on_configureVideoList_clicked();
};

#endif // VIDEOLISTSSTACK_H
