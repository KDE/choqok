/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "choqokuiglobal.h"

#include <QApplication>
#include <QPointer>

#include "notifymanager.h"
#include "postwidget.h"
#include "quickpost.h"

namespace Choqok
{

namespace
{
QPointer<Choqok::UI::MainWindow> g_mainWidget;
QPointer<UI::QuickPost> g_quickPost;
}

void UI::Global::setMainWindow(Choqok::UI::MainWindow *widget)
{
    g_mainWidget = widget;
}

Choqok::UI::MainWindow *UI::Global::mainWindow()
{
    return g_mainWidget;
}

UI::QuickPost *UI::Global::quickPostWidget()
{
    return g_quickPost;
}

void UI::Global::setQuickPostWidget(UI::QuickPost *quickPost)
{
    g_quickPost = quickPost;
}

UI::Global::SessionManager::SessionManager():
    QObject(qApp)
{

}

UI::Global::SessionManager::~SessionManager()
{

}

UI::Global::SessionManager *UI::Global::SessionManager::m_self = nullptr;

UI::Global::SessionManager *UI::Global::SessionManager::self()
{
    if (!m_self) {
        m_self = new SessionManager;
    }
    return m_self;
}

void UI::Global::SessionManager::emitNewPostWidgetAdded(UI::PostWidget *widget, Choqok::Account *theAccount,
        const QString &timelineName)
{
    Q_EMIT newPostWidgetAdded(widget, theAccount, timelineName);
}

void UI::Global::SessionManager::resetNotifyManager()
{
    NotifyManager::resetNotifyManager();
}

}

#include "moc_choqokuiglobal.cpp"
