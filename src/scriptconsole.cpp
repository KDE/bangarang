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

#include <QUrl>
#include <QList>
#include <QWidget>
#include <QTextEdit>
#include <QByteArray>
#include <QVBoxLayout> 
#include <QListWidget>
#include <QTemporaryFile>

#include <KDebug>
#include <KLocale>
#include <KTextEdit>
#include <KPushButton>
#include <KPushButton>

#include <kross/core/action.h>
#include <kross/core/manager.h>
#include <kross/core/actioncollection.h>

ScriptConsole::ScriptConsole(QWidget *parent) : QWidget(parent) ,
						m_action(0) ,
						m_layout(0) ,
						m_toolLayout(0) ,
						m_listWidget(0) ,
						m_language(0) ,
						m_sourceEdit(0) ,
						m_runScriptButton(0) ,
						m_objectList(0) ,
						m_stringList(0)
												
{
  m_layout = new QVBoxLayout(this);
  m_listWidget = new QListWidget();
  m_toolLayout = new QHBoxLayout(this);
  m_sourceEdit = new KTextEdit();
  m_runScriptButton = new KPushButton(i18n("Run Script"));
  m_language = new QComboBox();
  
  m_toolLayout->addWidget(m_language);
  m_toolLayout->addWidget(m_runScriptButton);
  m_objectList = new QList<QObject*>();
  m_stringList = new QStringList();
  
  m_layout->addWidget(m_listWidget);
  
  m_layout->addWidget(m_sourceEdit);
  m_layout->addLayout(m_toolLayout);
    
  setLayout(m_layout);
  
  connect(m_runScriptButton,SIGNAL(clicked()),this,SLOT(runScript()));
  connect(m_language,SIGNAL(activated(const QString & )),this, SLOT(interpreterActivated(const QString &)));

  m_language->addItem(i18n("Choose Interpreter:"),"");
  foreach(QString str,Kross::Manager::self().interpreters()) {
    m_language->addItem(str);
  }
  setWindowTitle(i18n("Bangarang - Scripting Console"));
  m_listWidget->addItem(i18n("BANGARANG MEDIA PLAYER\nCopyright (C) 2009 Andrew Lake (jamboarder@yahoo.com)\n<http://gitorious.org/bangarang>"));
}

ScriptConsole::~ScriptConsole()
{
  
}

void 
ScriptConsole::runScript()
{ 
  m_action = new Kross::Action(m_action,QUrl(""));
  m_action->setCode(m_sourceEdit->document()->toPlainText().toAscii());  
 
  if(interpreter.isEmpty()) {
    m_action->setInterpreter(Kross::Manager::self().interpreters().at(0));
  } else {
    m_action->setInterpreter(interpreter);
  }  
  
  for(int i = 0; i < m_objectList->size() && i < m_stringList->size() ; i++) {
    m_action->addObject(m_objectList->at(i),m_stringList->at(i));
  }
  
  m_action->addObject(this,"ScriptConsole"); 
  m_action->addObject(m_layout,"ConsoleLayout");
  connect(m_action,SIGNAL(finalized(Kross::Action*)),this ,SLOT(finished(Kross::Action*)));
  connect(m_action,SIGNAL(finished(Kross::Action* )),this ,SLOT(finished(Kross::Action*)));
  m_action->trigger();
  
}

void 
ScriptConsole::finished(Kross::Action *action)
{
  m_runScriptButton->setEnabled(true);
  if( action->hadError() ) {
      kDebug() << action->errorMessage();
      m_listWidget->addItem(action->errorMessage());
      kDebug() << action->code();
  } else {
    kDebug() << "code: Succeeded" << action->code();
    m_listWidget->addItem("success!");
  }
  
}


void
ScriptConsole::interpreterActivated(const QString &selectedInterpreter)
{
  if(selectedInterpreter.isEmpty()) {
    interpreter = Kross::Manager::self().interpreters().at(0);
    kDebug() << "Interpreter:" << interpreter;
   return;
  } else { //make sure its really an interpreter
    for(int i= 0; i < Kross::Manager::self().interpreters().size();i++) {
      if( Kross::Manager::self().interpreters().at(i) == selectedInterpreter){
	interpreter = selectedInterpreter;
      } 
    }
    kDebug() << "Interpreter:" << interpreter;
  }
}

void ScriptConsole::addObject(QObject* obj, QString str)
{
  m_objectList->append(obj);
  m_stringList->append(str);
  kDebug() << "Add " << str << " as " << obj << " to the list";
}

#include "moc_scriptconsole.cpp"


