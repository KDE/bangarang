/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)
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

#include <QtGui/QApplication>
#include <QtGui/QAction>
#include "mainwindow.h"
#include <KApplication>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KLocalizedString>
#include <KAboutData>

static KAboutData aboutData( "bangarang", 0,
        ki18n("Bangarang"), "0.99 (1.0~RC)",
        ki18n("A Media Player"), KAboutData::License_GPL_V2,
        ki18n("Copyright 2010, Andrew Lake"), KLocalizedString(),
        "" );

int main(int argc, char *argv[])
{
    aboutData.setProgramIconName("bangarang");
    aboutData.setOrganizationDomain( "mpris.org" ); //for DBus
    aboutData.addAuthor( ki18n("Andrew (Jamboarder) Lake"), ki18n("Creator"), "jamboarder@yahoo.com");
    aboutData.addCredit(ki18n("Sebastian Jambor"), ki18n("Contributor"));
    aboutData.addCredit(ki18n("Janusz Lewandowski"), ki18n("Contributor"));
    aboutData.addCredit(ki18n("Andreas Marschke"), ki18n("Contributor"));
    aboutData.setBugAddress("http://code.google.com/p/bangarangissuetracking/");
    aboutData.setCustomAuthorText(ki18n("Defects may be reported at http://code.google.com/p/bangarangissuetracking/"), ki18n("Defects may be reported at <a href='http://code.google.com/p/bangarangissuetracking/'>Bangarang Issue Tracker</a>"));
    aboutData.setHomepage("http://gitorious.org/bangarang");
    aboutData.setLicense(KAboutData::License_GPL_V3);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Play 'URL'" ));
    options.add("play-dvd", ki18n( "Play DVD Video" ));
    options.add("play-cd", ki18n( "Play CD Music" ));
    options.add("debug", ki18n( "Show Additional Debug Output" ));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication application;

    MainWindow w;
    w.setAboutData(&aboutData);
    w.show();
    return application.exec();
}
