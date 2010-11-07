/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Andrey Esin <gmlastik@gmail.com>

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

#include "indicatormanager.h"
#include <QApplication>
#include "choqokuiglobal.h"

#include <QDebug>
#include <KIcon>

#define STR(x) #x
#define XSTR(x) STR(x)

namespace Choqok
{

MessageIndicatorManager::MessageIndicatorManager()
{
    iServer = QIndicate::Server::defaultInstance();
    iServer->setType ( "message.irc" );
    QString desktopFile = QString ( "%1/%2.desktop" )
                          .arg ( XSTR ( XDG_APPS_INSTALL_DIR ) )
                          .arg ( QCoreApplication::applicationFilePath().section ( '/', -1 ) );
    iServer->setDesktopFile ( desktopFile );
    connect ( iServer, SIGNAL ( serverDisplay() ), SLOT ( slotShowMainWindow() ) );
    iServer->show();

    iIndicator = new QIndicate::Indicator ( this );
    iIndicator->setNameProperty ( "Home" );
    iIndicator->setIconProperty ( KIcon ( "user-home" ).pixmap ( QSize ( 16, 16 ), QIcon::Normal, QIcon::On ).toImage() );

    connect ( iIndicator, SIGNAL ( display ( QIndicate::Indicator* ) ), SLOT ( slotDisplay ( QIndicate::Indicator* ) ) );
    allUnread = 0;

}

MessageIndicatorManager::~MessageIndicatorManager()
{
}

void MessageIndicatorManager::newPostInc ( int unread, const QString& alias, const QString& timeline )
{
    //Q_UNUSED(unread);
    //Q_UNUSED(alias);
    qDebug() << alias << timeline << unread;
    if ( timeline == QString ( "Home" ) ) {
        iIndicator->setDrawAttentionProperty ( true );
        iIndicator->show();
        iIndicator->setCountProperty ( allUnread += unread );
    }
}

void MessageIndicatorManager::slotDisplay ( QIndicate::Indicator* )
{
    iIndicator->hide();
    iIndicator->setDrawAttentionProperty ( false );
    allUnread = 0;
    slotShowMainWindow();
}

void MessageIndicatorManager::slotShowMainWindow()
{
    choqokMainWindow->activateChoqok();
}

MessageIndicatorManager * MessageIndicatorManager::mSelf = NULL;

MessageIndicatorManager * MessageIndicatorManager::self()
{
    if ( !mSelf )
        mSelf = new MessageIndicatorManager;
    return mSelf;
}

}
