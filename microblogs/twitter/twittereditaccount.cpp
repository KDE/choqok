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

#include "twittereditaccount.h"
#include "twittermicroblog.h"
#include "twitteraccount.h"
#include <KDebug>
#include <kio/jobclasses.h>
#include <kio/netaccess.h>
#include <kio/job.h>
#include <KMessageBox>
#include <QDomDocument>
#include <KToolInvocation>


TwitterEditAccountWidget::TwitterEditAccountWidget(TwitterMicroBlog *microblog,
                                                    TwitterAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    setupUi(this);
    if(mAccount) {
        groupBoxRegister->hide();
        kcfg_username->setText( mAccount->username() );
        kcfg_password->setText( mAccount->password() );
        kcfg_alias->setText( mAccount->alias() );
        kcfg_readonly->setChecked( mAccount->isReadOnly() );
        kcfg_secure->setChecked( mAccount->useSecureConnection() );
    } else {
        setAccount( mAccount = new TwitterAccount(microblog, microblog->serviceName()) );
        kcfg_alias->setText( microblog->serviceName() );
    }
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

TwitterEditAccountWidget::~TwitterEditAccountWidget()
{
}

bool TwitterEditAccountWidget::validateData()
{
    if(kcfg_alias->text().isEmpty() || kcfg_username->text().isEmpty() ||
        kcfg_password->text().isEmpty() )
        return false;
    else
        return true;
}

Choqok::Account* TwitterEditAccountWidget::apply()
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

void TwitterEditAccountWidget::slotRegisterNewAccount()
{
    KToolInvocation::invokeBrowser( "http://twitter.com/signup" );
}

bool TwitterEditAccountWidget::verifyCredentials()
{
    kDebug();
    KUrl url( "http://identi.ca/api/account/verify_credentials.xml" );
    if(kcfg_secure->isChecked())
        url.setScheme("https");
    url.setUserName(kcfg_username->text());
    url.setPassword(kcfg_password->text());

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

