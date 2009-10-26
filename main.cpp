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
