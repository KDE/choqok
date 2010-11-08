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

#include <accountmanager.h>
#include <account.h>
#include "microblog.h"
#include <microblogwidget.h>

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

    connect ( Choqok::AccountManager::self(), SIGNAL ( allAccountsLoaded() ), SLOT ( slotCanWorkWithAccs() ) );
}

MessageIndicatorManager::~MessageIndicatorManager()
{
}

void MessageIndicatorManager::slotCanWorkWithAccs()
{
    accList = Choqok::AccountManager::self()->accounts();

    QList<Choqok::UI::MicroBlogWidget*> lst = choqokMainWindow->microBlogsWidgetsList();
    for ( int i = 0;i < choqokMainWindow->microBlogsWidgetsList().count();i++ ) {
        connect ( lst.at ( i ), SIGNAL ( updateUnreadCount ( int, int ) ), SLOT ( slotupdateUnreadCount ( int, int ) ) );
    }
}

void MessageIndicatorManager::slotupdateUnreadCount ( int change, int sum )
{
    QString alias = qobject_cast<Choqok::UI::MicroBlogWidget*> ( sender() )->currentAccount()->alias();
    newPostInc ( sum, alias, QString() );
}


QImage MessageIndicatorManager::getIconByAlias ( const QString& alias )
{
    Choqok::Account* acc = Choqok::AccountManager::self()->findAccount ( alias );
    return KIcon ( acc->microblog()->pluginIcon() ).pixmap ( QSize ( 16, 16 ), QIcon::Normal, QIcon::On ).toImage();
}

void MessageIndicatorManager::newPostInc ( int unread, const QString& alias, const QString& timeline )
{
    Q_UNUSED ( timeline );

    if ( !showList.contains ( alias ) ) {
        showList.insert ( alias, unread );
        QIndicate::Indicator *newIndicator = new QIndicate::Indicator ( this );
        newIndicator->setNameProperty ( alias );
        newIndicator->setCountProperty ( unread );
        newIndicator->setDrawAttentionProperty ( true );
        newIndicator->show();
        newIndicator->setIconProperty ( getIconByAlias ( alias ) );
        iList.insert ( alias, newIndicator );
        connect ( iList.value ( alias ), SIGNAL ( display ( QIndicate::Indicator* ) ), SLOT ( slotDisplay ( QIndicate::Indicator* ) ) );
    } else {
        showList[ alias ] = unread;
        iList.value ( alias )->setCountProperty ( showList.value ( alias ) );
        iList.value ( alias )->setDrawAttentionProperty ( unread != 0 );
        iList.value ( alias )->show();
    }


}

void MessageIndicatorManager::slotDisplay ( QIndicate::Indicator* indicator )
{
    Q_UNUSED ( indicator );
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
