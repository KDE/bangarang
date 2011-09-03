#ifndef AUDIOLISTSSTACK_H
#define AUDIOLISTSSTACK_H

#include <QWidget>

namespace Ui {
    class AudioListsStack;
}

#include "ui_audiolistsstack.h"

class BangarangApplication;

class AudioListsStack : public QWidget
{
    Q_OBJECT

public:
    explicit AudioListsStack(QWidget *parent = 0);
    ~AudioListsStack();
    void enableTouch();
    Ui::AudioListsStack *ui;

private:
    BangarangApplication *m_application;

private slots:
    void on_configureAudioList_clicked();
};

#endif // AUDIOLISTSSTACK_H
