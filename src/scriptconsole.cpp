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
#include <kross/core/action.h>
#include <KLocale>
#include <KUrl>
#include <KDebug>

ScriptConsole::ScriptConsole(QWidget *parent) : QWidget(parent) ,
						m_action(0) ,
						m_layout(0) ,
						m_listWidget(0) ,
						m_sourceEdit(0) ,
						m_runScriptButton(0)
{
  m_layout = new QVBoxLayout(this);
  m_listWidget = new QListWidget();
  m_sourceEdit = new KTextEdit();
  m_runScriptButton = new KPushButton(i18n("Run Script"));
  
  m_layout->addWidget(m_listWidget);
  m_layout->addWidget(m_sourceEdit);
  m_layout->addWidget(m_runScriptButton);

  setLayout(m_layout);
  connect(m_runScriptButton,SIGNAL(clicked()),this,SLOT(runScript()));
}

ScriptConsole::~ScriptConsole()
{}

void 
ScriptConsole::runScript()
{ 
  m_action = new Kross::Action(this,KUrl(""));
  m_action->setInterpreter("javascript");
  
  QByteArray code;
  code =  m_sourceEdit->document()->toPlainText().toAscii();
  m_action->addObject(this,"ScriptConsole");
  m_action->addObject(m_layout,"ConsoleLayout");
  m_action->setCode(code);
  
  m_action->trigger();
  if( m_action->hadError() ) {
    kDebug() << m_action->errorMessage();
    m_listWidget->addItem(m_action->errorMessage());
  }

}


// Kross::Action*
// ScriptConsole::action()
// {
//   return m_action;
// }

#include "moc_scriptconsole.cpp"


