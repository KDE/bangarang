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

#ifndef SCRIPTCONSOLE_H
#define SCRIPTCONSOLE_H

#include <QWidget>
#include <QList>
#include <KPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <KTextEdit>
#include <KPushButton>
#include <QComboBox>
#include <kross/core/action.h>
#include <kross/core/manager.h>

class ScriptConsole : public QWidget
{
  Q_OBJECT
  
  public: 
    ScriptConsole(QWidget *parent = 0 );
    ~ScriptConsole();
    
    Kross::Action* action();

  private:

    Kross::Action *m_action;
    QVBoxLayout *m_layout;
    QHBoxLayout *m_toolLayout;
    QListWidget *m_listWidget;
    QComboBox *m_language;
    KTextEdit *m_sourceEdit;
    KPushButton *m_runScriptButton;
    KPushButton *m_stopButton;
  private slots:
    void runScript();
    void interpreterActivated(const QString &);
    void finished(Kross::Action*);
    void hardfinish();
};

#endif //SCRIPTCONSOLE_H
