/*
    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "quickfilter.h"
#include <KGenericFactory>
#include <choqokuiglobal.h>
#include "postwidget.h"
#include "choqoktypes.h"
#include <qmutex.h>
#include <QDomDocument>
#include <microblogwidget.h>
#include <KLineEdit>
#include <QToolBar>
#include <timelinewidget.h>
#include <KMenuBar>
#include <KAction>
#include <KActionCollection>
#include <QLabel>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < QuickFilter > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_quickfilter" ) )

QuickFilter::QuickFilter(QObject* parent, const QList< QVariant >& args) : Choqok::Plugin(MyPluginFactory::componentData(), parent)
{
  KAction *authorAction = new KAction(KIcon("document-preview"), i18n("Filter by author"), this);
  authorAction->setCheckable(true);
  KAction *textAction = new KAction(KIcon("document-preview"), i18n("Filter by text"), this);
  textAction->setCheckable(true);
  actionCollection()->addAction("filterByAuthor", authorAction);
  actionCollection()->addAction("filterByText", textAction);
  connect(authorAction, SIGNAL(toggled(bool)), SLOT(showAuthorFilterUiInterface(bool)));
  connect(textAction, SIGNAL(triggered(bool)), SLOT(showTextFilterUiInterface(bool)));
  setXMLFile("quickfilterui.rc");
  createUiInterface();
}

QuickFilter::~QuickFilter()
{

}

void QuickFilter::filterByAuthor()
{
  if (!m_filterUser.isEmpty()) {
    foreach(Choqok::UI::PostWidget* postwidget, Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
	if (postwidget->currentPost().author.userName != m_filterUser) {
	  postwidget->hide();
      }
    }
  }
  else {
    foreach(Choqok::UI::PostWidget* postwidget, Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
	postwidget->show();
    }
  }
}

void QuickFilter::filterByText()
{
  if (!m_filterText.isEmpty()) {
    foreach(Choqok::UI::PostWidget* postwidget, Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
	if ( ! postwidget->currentPost().content.contains(m_filterText, Qt::CaseInsensitive) ) {
	  postwidget->hide();
	}
      }
  }
  else {
    foreach(Choqok::UI::PostWidget* postwidget, Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
	postwidget->show();
    }
  }
}

void QuickFilter::createUiInterface()
{
  m_authorToolbar = new QToolBar(Choqok::UI::Global::mainWindow());
  m_textToolbar = new QToolBar(Choqok::UI::Global::mainWindow());
  m_aledit = new KLineEdit(m_authorToolbar);
  m_tledit = new KLineEdit(m_textToolbar);
  QLabel *alabel = new QLabel(i18n("Author"), m_authorToolbar);
  QLabel *tlabel = new QLabel(i18n("Text"), m_textToolbar);
  m_authorToolbar->addWidget(alabel);
  m_authorToolbar->addWidget(m_aledit);
  m_textToolbar->addWidget(tlabel);
  m_textToolbar->addWidget(m_tledit);

  connect(m_aledit, SIGNAL(editingFinished()), this , SLOT(filterByAuthor()));
  connect(m_aledit, SIGNAL(textChanged(QString)), this, SLOT(updateUser(QString)));
  
  connect(m_tledit, SIGNAL(editingFinished()), this, SLOT(filterByText()));
  connect(m_tledit, SIGNAL(textChanged(QString)), this, SLOT(updateText(QString)));
  
  Choqok::UI::Global::mainWindow()->addToolBar(Qt::BottomToolBarArea, m_authorToolbar);
  Choqok::UI::Global::mainWindow()->addToolBar(Qt::BottomToolBarArea, m_textToolbar);
  m_authorToolbar->hide();
  m_textToolbar->hide();
}

void QuickFilter::showAuthorFilterUiInterface(bool show)
{
  if (show) {
    m_authorToolbar->show();
  }
  else {
    m_authorToolbar->hide();
  }
}

void QuickFilter::showTextFilterUiInterface(bool show)
{
  if (show) {
    m_textToolbar->show();
  }
  else {
    m_textToolbar->hide();
  }
}

void QuickFilter::updateUser(QString user)
{
  m_filterUser = user;
}

void QuickFilter::updateText(QString text)
{
  m_filterText = text;
}



