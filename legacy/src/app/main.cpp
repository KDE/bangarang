/* BANGARANG MEDIA PLAYER
* Copyright (C) 2009 Andrew Lake (jamboarder@gmail.com)
* <https://commits.kde.org/bangarang>
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

#include "common/mainwindow.h"
#include "common/bangarangapplication.h"
#include <QtGui/QApplication>
#include <QtGui/QAction>
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KLocalizedString>
#include <KAboutData>

static KAboutData aboutData( "bangarang", 0,
        ki18n("Bangarang"), "2.1",
        ki18n("A media player for your KDE desktop"), KAboutData::License_GPL_V3,
        ki18n("Copyright 2011, Andrew Lake"),
        ki18n("<b>Note:</b> This product uses the TMDb API but is not endorsed or certified by TMDb. Please help improve available information by visiting http://themoviedb.org<br>"
              "<b>Note:</b> This product uses the Last.fm API but is not endorsed or certified by Last.fm.  Please help improve available information by visiting http://last.fm<br>"
              "<b>Note:</b> This product uses the TheTVDB.com API but is not endorsed or certified by TheTVDB.com.  Please help improve available information by visiting http://thetvdb.com"),
        "http://bangarangkde.wordpress.org" );

int main(int argc, char *argv[])
{
    aboutData.setProgramIconName("bangarang");
    aboutData.setOrganizationDomain( "mpris.org" ); //for DBus
    aboutData.addAuthor( ki18n("Andrew (Jamboarder) Lake"), ki18n("Creator"), "jamboarder@gmail.com");
    aboutData.addCredit(ki18n("Stefan Burnicki"), ki18n("Contributor"));
    aboutData.addCredit(ki18n("Elias Probst"), ki18n("Contributor"));
    aboutData.setBugAddress("https://bugs.kde.org/enter_bug.cgi?product=bangarang&format=guided");
    aboutData.setCustomAuthorText(ki18n("Defects may be reported at https://bugs.kde.org/enter_bug.cgi?product=bangarang&format=guided"), ki18n("Defects may be reported at <a href='https://bugs.kde.org/enter_bug.cgi?product=bangarang&format=guided'>KDE Bugtracker</a>"));
    aboutData.setHomepage("https://commits.kde.org/bangarang");
    aboutData.setLicense(KAboutData::License_GPL_V3);

    KCmdLineArgs::init( argc, argv, &aboutData );

    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "Play 'URL'" ));
    options.add("play-dvd", ki18n( "Play DVD Video" ));
    options.add("play-cd", ki18n( "Play CD Music" ));
    options.add("debug", ki18n( "Show Additional Debug Output" ));
    options.add("touch", ki18n("Enable touch mode for interface"));
    KCmdLineArgs::addCmdLineOptions( options );

    BangarangApplication application;
    application.setup();

    MainWindow * w = application.mainWindow();
    w->show();
    application.processCommandLineArgs();
    return application.exec();
}
