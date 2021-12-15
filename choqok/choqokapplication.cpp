/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "choqokapplication.h"

#include "choqokbehaviorsettings.h"
#include "choqokdebug.h"
#include "choqokuiglobal.h"
#include "dbushandler.h"
#include "mainwindow.h"

ChoqokApplication::ChoqokApplication(int &argc, char **argv)
    : Choqok::Application(argc, argv)
    , m_mainWindow(nullptr)
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
        m_mainWindow = nullptr;
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
        m_mainWindow = nullptr;
    }
    this->quit();
}

