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

#include "nepomukwriter.h"
#include "../mediavocabulary.h"
#include <KCmdLineArgs>
#include <KCmdLineOptions>
#include <KLocalizedString>
#include <KAboutData>
#include <KDebug>
#include <QtCore>
#include <QFile>
#include <QString>
#include <QUrl>

static KAboutData aboutData( "bangarangnepomukwriter", 0,
                             KLocalizedString(), "2.0",
                             KLocalizedString(), KAboutData::License_GPL_V3,
        ki18n("Copyright 2011, Andrew Lake"), KLocalizedString(),
        "" );

int main(int argc, char *argv[])
{
    QTextStream cout(stdout, QIODevice::WriteOnly);
    cout << "started nepomukwriter\n";
    cout.flush();
    
    KCmdLineArgs::init( argc, argv, &aboutData );
    KCmdLineOptions options;
    options.add("+[URL]", ki18n( "File directive" ));
    KCmdLineArgs::addCmdLineOptions( options );
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication application;
    
    bool nepomukInited = Nepomuk::ResourceManager::instance()->initialized();
    if (!nepomukInited) {
        Nepomuk::ResourceManager::instance()->init();
        nepomukInited = Nepomuk::ResourceManager::instance()->initialized();
    }

    if (args->count() > 0 && nepomukInited) {
        
        QFile file(args->arg(0));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            cout << QString("BangarangError:Couldn't open %1\n").arg(args->arg(0));
            return 0;
        }

        NepomukWriter nepomukWriter;
        nepomukWriter.processJob(&file);

    } else {
        cout << "You didn't provide an argument OR Nepomuk is not active!\n";
    }
    cout.flush();
    
    return 0;
}
