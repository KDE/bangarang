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

#include "scriptconsole.h"
#include <QWidget>
#include <QList>
#include <KPushButton>
#include <QVBoxLayout>
#include <QListWidget>
#include <KTextEdit>
#include <QTextEdit>
#include <QByteArray>
#include <KPushButton>
#include <KLocale>
#include <KUrl>
#include <KDebug>

#include <kross/core/action.h>
#include <kross/core/manager.h>

ScriptConsole::ScriptConsole(QWidget *parent) : QWidget(parent) ,
						m_action(0) ,
						m_layout(0) ,
						m_toolLayout(0) ,
						m_listWidget(0) ,
						m_language(0) ,
						m_sourceEdit(0) ,
						m_runScriptButton(0) ,
						m_stopButton(0)
						
{
  m_layout = new QVBoxLayout(this);
  m_listWidget = new QListWidget();
  m_toolLayout = new QHBoxLayout(this);
  m_sourceEdit = new KTextEdit();
  m_runScriptButton = new KPushButton(i18n("Run Script"));
  m_stopButton = new KPushButton(i18n("Stop"));
  m_language = new QComboBox();
  
  m_toolLayout->addWidget(m_stopButton);
  m_toolLayout->addWidget(m_language);
  m_toolLayout->addWidget(m_runScriptButton);
  
  m_layout->addWidget(m_listWidget);
  m_layout->addWidget(m_sourceEdit);
  m_layout->addLayout(m_toolLayout);
  
  setLayout(m_layout);
  
  connect(m_stopButton,SIGNAL(clicked()),this,SLOT(hardfinish()));
  connect(m_runScriptButton,SIGNAL(clicked()),this,SLOT(runScript()));
  connect(m_language,SIGNAL(activated(const QString & )),this, SLOT(interpreterActivated(const QString &)));
  
  //these are the objects we definitely _will_ make avaiable
  //@TODO decide which parts of bangarang should be avaiable to the Script developer. 
  //UseCase: Make a scrobbler for last.fm/libre.fm you want the data off the played song.
  //UseCase: You want to create a graph from your listening/watching Statistics.
  m_action = new Kross::Action(this,KUrl(""));
  m_action->addObject(this,"ScriptConsole"); 
  m_action->addObject(m_layout,"ConsoleLayout");
  m_action->setInterpreter(Kross::Manager::self().interpreters().at(0));
  connect(m_action,SIGNAL(finalized(Kross::Action*)),this ,SLOT(finished(Kross::Action*)));

  m_language->addItem(i18n("Choose Interpreter:"),"");
  foreach(QString str,Kross::Manager::self().interpreters()) {
    m_language->addItem(str);
  }

}

ScriptConsole::~ScriptConsole()
{}

void 
ScriptConsole::runScript()
{ 
  QByteArray code;
  code =  m_sourceEdit->document()->toPlainText().toAscii();
  m_action->setCode(code);  
  m_action->trigger();
  //m_runScriptButton->setEnabled(false);
}


void
ScriptConsole::interpreterActivated(const QString &selectedInterpreter)
{
  if(selectedInterpreter.isEmpty()) {
    m_action->setInterpreter(Kross::Manager::self().interpreters().at(0));
    return;
  } else { 
    m_action->setInterpreter(selectedInterpreter);
  }
}

Kross::Action*
ScriptConsole::action()
{
  return m_action;
}

void 
ScriptConsole::finished(Kross::Action *action)
{
  m_runScriptButton->setEnabled(true);
  if( action->hadError() ) {
      kDebug() << action->errorMessage();
      m_listWidget->addItem(action->errorMessage());
  } else {
    kDebug() << "code: Succeeded" << action->code();
  }
  
}
void ScriptConsole::hardfinish()
{
  if(m_action != NULL){
    m_action->~Action();
  }
}
#include "moc_scriptconsole.cpp"


