#include "mainwindow.h"
#include "platform/bangarangvideowidget.h"

extern "C" {
  KDE_EXPORT QObject* krossmoduleMainWindow() {
    return new MainWindow();
  }
}

extern "C" {
  KDE_EXPORT QObject* krossmoduleBangarangVideoWidget() {
    return new BangarangVideoWidget();
  }
}
