#include <QtGui/QApplication>
#include "mainwindow.h"
#include <KApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KLocalizedString>
#include <KAboutData>

static KAboutData aboutData( "Bangarang", 0,
        ki18n("Bangarang"), "0.9",
        ki18n("Enterainment... Now."), KAboutData::License_GPL_V2,
        ki18n("Copyright 2009, Andrew Lake"), ki18n(""),
        "" );

int main(int argc, char *argv[])
{
    aboutData.setOrganizationDomain( "mpris.org" ); //for DBus
    aboutData.addCredit( ki18n("Andrew Lake"), ki18n("Creator") );

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Play 'URL'" ));
    options.add("play-dvd", ki18n( "Play DVD Video" ));
    options.add("play-cd", ki18n( "Play CD Music" ));
    options.add("debug", ki18n( "Show Additional Debug Output" ));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication application;

    MainWindow w;
    w.show();
    return application.exec();
}
