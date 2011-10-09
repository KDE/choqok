/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <qtimer.h>
#include <qregexp.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kmessagebox.h>
#include "mainwindow.h"
#include <pluginmanager.h>
#include "choqokbehaviorsettings.h"
#include <choqokuiglobal.h>
#include <dbushandler.h>

ChoqokApplication::ChoqokApplication()
: Choqok::Application()
{
    kDebug();
    setQuitOnLastWindowClosed( false );

    Choqok::ChoqokDbus();
    
    m_mainWindow = new MainWindow;

    Choqok::UI::Global::setMainWindow( m_mainWindow );

}

ChoqokApplication::~ChoqokApplication()
{
    kDebug() ;
    //kDebug() << "Done";
}

int ChoqokApplication::newInstance()
{
//    kDebug(14000) ;
//     handleURLArgs();

    return KUniqueApplication::newInstance();
}

void ChoqokApplication::quitChoqok()
{
    kDebug() ;

    setShuttingDown(true);

    if ( m_mainWindow )
    {
        Choqok::BehaviorSettings::setShowMainWinOnStart(m_mainWindow->isVisible());
        m_mainWindow->deleteLater();
        m_mainWindow = 0;
    }
    this->quit();
}

void ChoqokApplication::commitData( QSessionManager &sm )
{
    setShuttingDown(true);
    KUniqueApplication::commitData( sm );
}

#include "choqokapplication.moc"
// vim: set noet ts=4 sts=4 sw=4:
