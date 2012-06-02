/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "choqokuiglobal.h"

#include <qpointer.h>
#include "quickpost.h"
#include <QApplication>
#include "postwidget.h"
#include "notifymanager.h"

namespace Choqok
{


namespace
{
    QPointer<Choqok::UI::MainWindow> g_mainWidget;
    QPointer<UI::QuickPost> g_quickPost;
}

void UI::Global::setMainWindow( Choqok::UI::MainWindow* widget )
{
    g_mainWidget = widget;
}

Choqok::UI::MainWindow *UI::Global::mainWindow()
{
    return g_mainWidget;
}

UI::QuickPost* UI::Global::quickPostWidget()
{
    return g_quickPost;
}

void UI::Global::setQuickPostWidget(UI::QuickPost* quickPost)
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

UI::Global::SessionManager *UI::Global::SessionManager::m_self = 0L;

UI::Global::SessionManager* UI::Global::SessionManager::self()
{
    if(!m_self)
        m_self = new SessionManager;
    return m_self;
}

void UI::Global::SessionManager::emitNewPostWidgetAdded( UI::PostWidget* widget, Choqok::Account *theAccount,
                                                         const QString &timelineName )
{
    emit newPostWidgetAdded(widget, theAccount, timelineName);
}

void UI::Global::SessionManager::resetNotifyManager()
{
    NotifyManager::resetNotifyManager();
}

}

#include "choqokuiglobal.moc"
