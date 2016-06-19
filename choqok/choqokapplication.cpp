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

#include "choqokapplication.h"

#include "choqokbehaviorsettings.h"
#include "choqokdebug.h"
#include "choqokuiglobal.h"
#include "dbushandler.h"
#include "mainwindow.h"

ChoqokApplication::ChoqokApplication(int &argc, char **argv)
    : Choqok::Application(argc, argv)
    , m_mainWindow(0)
{
    qCDebug(CHOQOK);

    setQuitOnLastWindowClosed(false);

    Choqok::ChoqokDbus();
}

ChoqokApplication::~ChoqokApplication()
{
    qCDebug(CHOQOK);
}

void ChoqokApplication::setupMainWindow()
{
    if (m_mainWindow) {
        m_mainWindow->deleteLater();
        m_mainWindow = 0;
    }

    m_mainWindow = new MainWindow(this);

    Choqok::UI::Global::setMainWindow(m_mainWindow);
}

void ChoqokApplication::quitChoqok()
{
    qCDebug(CHOQOK) ;

    setShuttingDown(true);

    if (m_mainWindow) {
        Choqok::BehaviorSettings::setShowMainWinOnStart(m_mainWindow->isVisible());
        m_mainWindow->deleteLater();
        m_mainWindow = 0;
    }
    this->quit();
}

