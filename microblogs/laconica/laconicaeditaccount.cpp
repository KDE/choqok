/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "laconicaeditaccount.h"
#include "laconicamicroblog.h"
#include "laconicaaccount.h"
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <KMessageBox>
#include <QDomDocument>
#include <KToolInvocation>


LaconicaEditAccountWidget::LaconicaEditAccountWidget(LaconicaMicroBlog *microblog,
                                                    LaconicaAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    setupUi(this);
    if(mAccount) {
        kcfg_username->setText( mAccount->username() );
        kcfg_password->setText( mAccount->password() );
        kcfg_alias->setText( mAccount->alias() );
        kcfg_readonly->setChecked( mAccount->isReadOnly() );
        kcfg_secure->setChecked( mAccount->useSecureConnection() );
        kcfg_host->setText( mAccount->host() );
        kcfg_api->setText( mAccount->api() );
    } else {
        setAccount( mAccount = new LaconicaAccount(microblog, microblog->serviceName()) );
        kcfg_alias->setText( microblog->serviceName() );
    }
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

LaconicaEditAccountWidget::~LaconicaEditAccountWidget()
{
}

bool LaconicaEditAccountWidget::validateData()
{
    if(kcfg_alias->text().isEmpty() || kcfg_username->text().isEmpty() ||
        kcfg_password->text().isEmpty() )
        return false;
    else
        return true;
}

Choqok::Account* LaconicaEditAccountWidget::apply()
{
    kDebug();
    if ( !verifyCredentials() )
        return 0;
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setReadOnly(kcfg_readonly->isChecked());
    mAccount->setUseSecureConnection(kcfg_secure->isChecked());
    mAccount->writeConfig();
    return mAccount;
}

bool LaconicaEditAccountWidget::verifyCredentials()
{
    kDebug();
    KUrl url;
    url.setHost(kcfg_host->text());
    url.addPath(kcfg_api->text());
    url.addPath("/account/verify_credentials.xml");
    kDebug()<<url.prettyUrl();
    if(kcfg_secure->isChecked())
        url.setScheme("https");
    else
        url.setScheme("http");
    url.setUserName(kcfg_username->text());
    url.setPassword(kcfg_password->text());
    kDebug()<<url.prettyUrl();
    KIO::TransferJob *job = KIO::get( url, KIO::Reload, KIO::HideProgressInfo );
    if ( !job ) {
        kDebug() << "Cannot create an http GET request.";
        QString errMsg = i18n ( "Cannot create an http GET request." );
        KMessageBox::error(this, errMsg);
        return false;
    }
    QByteArray data;
    if( KIO::NetAccess::synchronousRun(job, this, &data) ) {
        QDomDocument document;
        document.setContent ( data );
        QDomElement root = document.documentElement();
        if ( root.tagName() == "user" ) {
            QDomNode node2 = root.firstChild();
            QString timeStr;
            while ( !node2.isNull() ) {
                if ( node2.toElement().tagName() == "id" ) {
                    mAccount->setUsername( kcfg_username->text() );
                    mAccount->setPassword( kcfg_password->text() );
                    mAccount->setUserId( node2.toElement().text() );
                    mAccount->setHost( kcfg_host->text() );
                    mAccount->setApi( kcfg_api->text() );
                    return true;
                    break;
                }
                node2 = node2.nextSibling();
            }
        } else if ( root.tagName() == "hash" ) {
            QDomNode node2 = root.firstChild();
            while ( !node2.isNull() ) {
                if ( node2.toElement().tagName() == "error" ) {
                    KMessageBox::detailedError(this, i18n ( "Authentication failed" ), node2.toElement().text() );
                    return false;
                }
                node2 = node2.nextSibling();
            }
        } else {
            kError() << "ERROR, unrecognized result, buffer is: " << data;
            KMessageBox::error( this, i18n ( "Unrecognized result." ) );
            return false;
        }
    } else {
        KMessageBox::detailedError(this, i18n("Authentication failed"),
                                    job->errorString());
        return false;
    }
    return false;
}

#include "laconicaeditaccount.moc"
