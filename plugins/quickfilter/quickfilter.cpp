/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    Copyright (C) 2011  Farhad Hedayati-Fard <hf.farhad@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/
*/

#include "quickfilter.h"

#include <QAction>
#include <QLabel>
#include <QPushButton>
#include <QToolBar>

#include <KActionCollection>
#include <KLineEdit>
#include <KLocalizedString>
#include <KPluginFactory>

#include "choqoktypes.h"
#include "choqokuiglobal.h"
#include "microblogwidget.h"
#include "postwidget.h"
#include "quickpost.h"
#include "timelinewidget.h"

K_PLUGIN_FACTORY_WITH_JSON(QuickFilterFactory, "choqok_quickfilter.json",
                           registerPlugin < QuickFilter > ();)

QuickFilter::QuickFilter(QObject *parent, const QList< QVariant > &args)
    : Choqok::Plugin("choqok_quickfilter", parent)
{
    Q_UNUSED(args);
    m_authorAction = new QAction(QIcon::fromTheme("document-preview"), i18n("Filter by author"), this);
    m_authorAction->setCheckable(true);
    m_textAction = new QAction(QIcon::fromTheme("document-preview"), i18n("Filter by content"), this);
    m_textAction->setCheckable(true);
    actionCollection()->addAction("filterByAuthor", m_authorAction);
    actionCollection()->addAction("filterByContent", m_textAction);
    setXMLFile("quickfilterui.rc");
    createUiInterface();
    connect(Choqok::UI::Global::mainWindow(), SIGNAL(currentMicroBlogWidgetChanged(Choqok::UI::MicroBlogWidget*)), this, SLOT(showAllPosts()));
}

QuickFilter::~QuickFilter()
{

}

void QuickFilter::filterByAuthor()
{
    m_filterUser = m_aledit->text();
    if (!m_filterUser.isEmpty() && Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()) {
        Q_FOREACH (Choqok::UI::PostWidget *postwidget,
                   Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
            if (!postwidget->currentPost()->author.userName.contains(m_filterUser, Qt::CaseInsensitive)) {
                postwidget->hide();
            } else {
                postwidget->show();
            }
        }
        connect(Choqok::UI::Global::SessionManager::self(),
                SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
                this, SLOT(filterNewPost(Choqok::UI::PostWidget*,Choqok::Account*,QString)));
    } else {
        showAllPosts();
    }
}

void QuickFilter::filterByContent()
{
    m_filterText = m_tledit->text();
    if (!m_filterText.isEmpty() && Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()) {
        Q_FOREACH (Choqok::UI::PostWidget *postwidget,
                   Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
            if (!postwidget->currentPost()->content.contains(m_filterText, Qt::CaseInsensitive)) {
                postwidget->hide();
            } else {
                postwidget->show();
            }
        }
        connect(Choqok::UI::Global::SessionManager::self(),
                SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
                this, SLOT(filterNewPost(Choqok::UI::PostWidget*,Choqok::Account*,QString)));
    } else {
        showAllPosts();
    }
}

void QuickFilter::createUiInterface()
{
    m_authorToolbar = new QToolBar(i18n("Filter out timeline by author"), Choqok::UI::Global::mainWindow());
    m_authorToolbar->setObjectName("authorFilterToolbar");
    m_textToolbar = new QToolBar(i18n("Filter out timeline by text"), Choqok::UI::Global::mainWindow());
    m_textToolbar->setObjectName("textFilterToolbar");
    connect(m_authorAction, SIGNAL(toggled(bool)), m_authorToolbar, SLOT(setVisible(bool)));
    connect(m_textAction, SIGNAL(toggled(bool)), m_textToolbar, SLOT(setVisible(bool)));
    connect(m_authorToolbar, SIGNAL(visibilityChanged(bool)), SLOT(showAuthorFilterUiInterface(bool)));
    connect(m_textToolbar, SIGNAL(visibilityChanged(bool)), SLOT(showContentFilterUiInterface(bool)));
    m_aledit = new KLineEdit(m_authorToolbar);
    m_aledit->setClearButtonShown(true);

    m_tledit = new KLineEdit(m_textToolbar);
    m_tledit->setClearButtonShown(true);

    QLabel *alabel = new QLabel(i18n("Author"), m_authorToolbar);
    QLabel *tlabel = new QLabel(i18n("Text"), m_textToolbar);
    m_authorToolbar->addWidget(alabel);
    m_authorToolbar->addWidget(m_aledit);
    QPushButton *authorCloseButton = new QPushButton(QIcon::fromTheme("dialog-close"), QString() , m_authorToolbar);
    authorCloseButton->setMaximumWidth(authorCloseButton->height());
    connect(authorCloseButton, SIGNAL(clicked(bool)), m_authorToolbar, SLOT(hide()));
    m_authorToolbar->addWidget(authorCloseButton);

    m_textToolbar->addWidget(tlabel);
    m_textToolbar->addWidget(m_tledit);
    QPushButton *textCloseButton = new QPushButton(QIcon::fromTheme("dialog-close"), QString() , m_textToolbar);
    textCloseButton->setMaximumWidth(textCloseButton->height());
    connect(textCloseButton, SIGNAL(clicked(bool)), m_textToolbar, SLOT(hide()));
    m_textToolbar->addWidget(textCloseButton);

    connect(m_aledit, SIGNAL(editingFinished()), this , SLOT(filterByAuthor()));
    connect(m_aledit, SIGNAL(textChanged(QString)), this, SLOT(updateUser(QString)));

    connect(m_tledit, SIGNAL(editingFinished()), this, SLOT(filterByContent()));
    connect(m_tledit, SIGNAL(textChanged(QString)), this, SLOT(updateContent(QString)));

    Choqok::UI::Global::mainWindow()->addToolBar(Qt::BottomToolBarArea, m_authorToolbar);
    Choqok::UI::Global::mainWindow()->addToolBar(Qt::BottomToolBarArea, m_textToolbar);
    m_authorToolbar->hide();
    m_textToolbar->hide();
}

void QuickFilter::showAuthorFilterUiInterface(bool show)
{
    m_authorToolbar->setVisible(show);
    if (show) {
        m_textAction->setChecked(false);
        m_aledit->setFocus();
    } else {
        m_aledit->clear();
        m_authorAction->setChecked(false);
    }
}

void QuickFilter::showContentFilterUiInterface(bool show)
{
    m_textToolbar->setVisible(show);
    if (show) {
        m_authorAction->setChecked(false);
        m_tledit->setFocus();
    } else {
        m_tledit->clear();
        m_textAction->setChecked(false);
    }
}

void QuickFilter::updateUser(QString user)
{
    if (user.isEmpty()) {
        filterByAuthor();
    }
}

void QuickFilter::updateContent(QString text)
{
    if (text.isEmpty()) {
        filterByContent();
    }
}

void QuickFilter::showAllPosts()
{
    if (Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()) {
        Q_FOREACH (Choqok::UI::PostWidget *postwidget,
                   Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
            postwidget->show();
        }
        m_aledit->clear();
        m_tledit->clear();
        disconnect(Choqok::UI::Global::SessionManager::self(),
                   SIGNAL(newPostWidgetAdded(Choqok::UI::PostWidget*,Choqok::Account*,QString)),
                   this, SLOT(filterNewPost(Choqok::UI::PostWidget*,Choqok::Account*,QString)));
    }
}

void QuickFilter::filterNewPost(Choqok::UI::PostWidget *np, Choqok::Account *acc, QString timeline)
{
    //qDebug()<<Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentAccount()->alias()<<acc->alias()<<timeline;
    if (Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentAccount() == acc &&
            Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->timelineName() == timeline) {
        if (!m_aledit->text().isEmpty()) {
            if (!np->currentPost()->author.userName.contains(m_aledit->text())) {
                np->hide();
            } else {
                np->show();
            }
        }
        if (!m_tledit->text().isEmpty()) {
            if (!np->currentPost()->content.contains(m_tledit->text())) {
                np->hide();
            } else {
                np->show();
            }
        }
    }
}

#include "quickfilter.moc"
