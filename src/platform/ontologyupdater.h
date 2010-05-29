/* BANGARANG MEDIA PLAYER
* Copyright (C) 2010 Andrew Lake (jamboarder@yahoo.com)
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

#ifndef ONTOLOGYUPDATER_H
#define ONTOLOGYUPDATER_H

#include <Nepomuk/Resource>
#include <QtCore>

/* This class is used to update ontology of the types and properties used to 
 * store media data in nepomuk.  Bangarang may be released using draft ontologies
 * to store media data in nepomuk.  When these ontologies are finally released,
 * they may differ from the draft.  This class is intended to be updated, along with
 * MediaVocabulary, to reflect the released ontology and provide a mechanism to 
 * update existing nepomuk media data to the match the released ontology.
 */

class OntologyUpdater : public QObject
{
    Q_OBJECT
    
    public:
        /**
         * Constructor
         */
        OntologyUpdater(QObject *parent);
        
        /**
         * Destructor
         */
        ~OntologyUpdater();
    
    public slots:
        void start();
        void stopUpdate();
        
    signals:
        void infoMessage(QString message);
        void done();
        
    private:
        void removeType(Nepomuk::Resource res, QUrl mediaType);
        bool m_stopUpdate;

};
#endif // ONTOLOGYUPDATER_H
