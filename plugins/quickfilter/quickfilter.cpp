/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2011 Farhad Hedayati-Fard <hf.farhad@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "quickfilter.h"

#include <QAction>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QToolBar>

#include <KActionCollection>
#include <KLocalizedString>
#include <KPluginFactory>

#include "choqoktypes.h"
#include "choqokuiglobal.h"
#include "microblogwidget.h"
#include "postwidget.h"
#include "quickpost.h"
#include "timelinewidget.h"

K_PLUGIN_CLASS_WITH_JSON(QuickFilter, "choqok_quickfilter.json")

QuickFilter::QuickFilter(QObject *parent, const QList< QVariant > &args)
    : Choqok::Plugin(QLatin1String("choqok_quickfilter"), parent)
{
    Q_UNUSED(args);
    m_authorAction = new QAction(QIcon::fromTheme(QLatin1String("document-preview")), i18n("Filter by author"), this);
    m_authorAction->setCheckable(true);
    m_textAction = new QAction(QIcon::fromTheme(QLatin1String("document-preview")), i18n("Filter by content"), this);
    m_textAction->setCheckable(true);
    actionCollection()->addAction(QLatin1String("filterByAuthor"), m_authorAction);
    actionCollection()->addAction(QLatin1String("filterByContent"), m_textAction);
    setXMLFile(QLatin1String("quickfilterui.rc"));
    createUiInterface();
    connect(Choqok::UI::Global::mainWindow(), &Choqok::UI::MainWindow::currentMicroBlogWidgetChanged,
            this, &QuickFilter::showAllPosts);
}

QuickFilter::~QuickFilter()
{

}

void QuickFilter::filterByAuthor()
{
    m_filterUser = m_aledit->text();
    if (!m_filterUser.isEmpty() && Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()) {
        for (Choqok::UI::PostWidget *postwidget:
                   Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
            if (!postwidget->currentPost()->author.userName.contains(m_filterUser, Qt::CaseInsensitive)) {
                postwidget->hide();
            } else {
                postwidget->show();
            }
        }
        connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
                this, &QuickFilter::filterNewPost);
    } else {
        showAllPosts();
    }
}

void QuickFilter::filterByContent()
{
    m_filterText = m_tledit->text();
    if (!m_filterText.isEmpty() && Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()) {
        for (Choqok::UI::PostWidget *postwidget:
                   Choqok::UI::Global::mainWindow()->currentMicroBlog()->currentTimeline()->postWidgets()) {
            if (!postwidget->currentPost()->content.contains(m_filterText, Qt::CaseInsensitive)) {
                postwidget->hide();
            } else {
                postwidget->show();
            }
        }
        connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
                this, &QuickFilter::filterNewPost);
    } else {
        showAllPosts();
    }
}

void QuickFilter::createUiInterface()
{
    m_authorToolbar = new QToolBar(i18n("Filter out timeline by author"), Choqok::UI::Global::mainWindow());
    m_authorToolbar->setObjectName(QLatin1String("authorFilterToolbar"));
    m_textToolbar = new QToolBar(i18n("Filter out timeline by text"), Choqok::UI::Global::mainWindow());
    m_textToolbar->setObjectName(QLatin1String("textFilterToolbar"));
    connect(m_authorAction, &QAction::toggled, m_authorToolbar, &QToolBar::setVisible);
    connect(m_textAction, &QAction::toggled, m_textToolbar, &QToolBar::setVisible);
    connect(m_authorToolbar, &QToolBar::visibilityChanged, this, &QuickFilter::showAuthorFilterUiInterface);
    connect(m_textToolbar, &QToolBar::visibilityChanged, this, &QuickFilter::showContentFilterUiInterface);
    m_aledit = new QLineEdit(m_authorToolbar);
    m_aledit->setClearButtonEnabled(true);

    m_tledit = new QLineEdit(m_textToolbar);
    m_tledit->setClearButtonEnabled(true);

    QLabel *alabel = new QLabel(i18n("Author"), m_authorToolbar);
    QLabel *tlabel = new QLabel(i18n("Text"), m_textToolbar);
    m_authorToolbar->addWidget(alabel);
    m_authorToolbar->addWidget(m_aledit);
    QPushButton *authorCloseButton = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-close")), QString() , m_authorToolbar);
    authorCloseButton->setMaximumWidth(authorCloseButton->height());
    connect(authorCloseButton, &QPushButton::clicked, m_authorToolbar, &QToolBar::hide);
    m_authorToolbar->addWidget(authorCloseButton);

    m_textToolbar->addWidget(tlabel);
    m_textToolbar->addWidget(m_tledit);
    QPushButton *textCloseButton = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-close")), QString() , m_textToolbar);
    textCloseButton->setMaximumWidth(textCloseButton->height());
    connect(textCloseButton, &QPushButton::clicked, m_textToolbar, &QToolBar::hide);
    m_textToolbar->addWidget(textCloseButton);

    connect(m_aledit, &QLineEdit::editingFinished, this, &QuickFilter::filterByAuthor);
    connect(m_aledit, &QLineEdit::textChanged, this, &QuickFilter::updateUser);

    connect(m_tledit, &QLineEdit::editingFinished, this, &QuickFilter::filterByContent);
    connect(m_tledit, &QLineEdit::textChanged, this, &QuickFilter::updateContent);

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
        for (Choqok::UI::PostWidget *postwidget:
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
