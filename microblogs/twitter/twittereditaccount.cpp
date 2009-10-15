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
#include <QProgressBar>
#include <accountmanager.h>
#include <choqoktools.h>


TwitterEditAccountWidget::TwitterEditAccountWidget(TwitterMicroBlog *microblog,
                                                    TwitterAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    setupUi(this);
    tabwidget->setTabBarHidden(true);//To hide configurations until their be functional :D
    kcfg_test->setIcon(KIcon("edit-find-user"));
    connect(kcfg_test, SIGNAL(clicked(bool)), SLOT(verifyCredentials()));
    if(mAccount) {
        groupBoxRegister->hide();
        kcfg_username->setText( mAccount->username() );
        kcfg_password->setText( mAccount->password() );
        kcfg_alias->setText( mAccount->alias() );
        kcfg_secure->setChecked( mAccount->useSecureConnection() );
    } else {
        QString newAccountAlias = microblog->serviceName();
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias))
            newAccountAlias = QString("%1%2").arg(newAccountAlias).arg(counter);
        setAccount( mAccount = new TwitterAccount(microblog, newAccountAlias) );
        kcfg_alias->setText( newAccountAlias );
    }
    kcfg_alias->setFocus(Qt::OtherFocusReason);
    connect( kcfg_register, SIGNAL( clicked() ), SLOT( slotRegisterNewAccount() ) );
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
    mAccount->setUsername( kcfg_username->text().toLower() );
    mAccount->setPassword( kcfg_password->text() );
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setUseSecureConnection(kcfg_secure->isChecked());
    mAccount->writeConfig();
    return mAccount;
}

void TwitterEditAccountWidget::slotRegisterNewAccount()
{
    Choqok::openUrl( KUrl("http://twitter.com/signup") );
}

void TwitterEditAccountWidget::verifyCredentials()
{
    kDebug();
    kcfg_test->setIcon(KIcon("edit-find-user"));
    KUrl url( "http://twitter.com/account/verify_credentials.xml" );
    if(kcfg_secure->isChecked())
        url.setScheme("https");
    url.setUserName(kcfg_username->text().toLower());
    url.setPassword(kcfg_password->text());

     KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if ( !job ) {
        kDebug() << "Cannot create an http GET request.";
        return;
//         QString errMsg = i18n ( "Cannot create an http GET request. Please check your KDE installation." );
//         KMessageBox::error(this, errMsg);
    }
    progress = new QProgressBar(this);
    progress->setRange(0, 0);
    kcfg_credentialsBox->layout()->addWidget(progress);
    connect(job, SIGNAL(result(KJob*)), SLOT(slotVerifyCredentials(KJob*)));
    job->start();
}

void TwitterEditAccountWidget::slotVerifyCredentials(KJob* job)
{
    kDebug();
    if(progress){
        progress->deleteLater();
        progress = 0L;
    }
    bool success = false;
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob*>(job);
    QDomDocument document;
    document.setContent ( stj->data() );
    QDomElement root = document.documentElement();
    if ( root.tagName() == "user" ) {
        QDomNode node2 = root.firstChild();
        QString timeStr;
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "id" ) {
                mAccount->setUserId( node2.toElement().text() );
                success= true;
                break;
            }
            node2 = node2.nextSibling();
        }
    } else if ( root.tagName() == "hash" ) {
        QDomNode node2 = root.firstChild();
        while ( !node2.isNull() ) {
            if ( node2.toElement().tagName() == "error" ) {
                KMessageBox::detailedError(this, i18n ( "Authentication failed" ), node2.toElement().text() );
            }
            node2 = node2.nextSibling();
        }
    } else {
        kDebug() << "ERROR, unrecognized result, buffer is: " << stj->data();
        KMessageBox::error( this, i18n ( "Unrecognized result." ) );
    }
    if(success)
        kcfg_test->setIcon(KIcon("dialog-ok"));
    else
        kcfg_test->setIcon(KIcon("dialog-error"));
}
