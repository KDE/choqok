/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <QtOAuth/interface.h>
#include <QtOAuth/qoauth_namespace.h>
#include <kio/accessmanager.h>
#include <QCheckBox>

TwitterEditAccountWidget::TwitterEditAccountWidget(TwitterMicroBlog *microblog,
                                                    TwitterAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    setupUi(this);
    kcfg_authorize->setIcon(KIcon("edit-find-user"));
    connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));
    if(mAccount) {
        groupBoxRegister->hide();
        kcfg_username->setText( mAccount->username() );
        kcfg_password->setText( mAccount->password() );
        kcfg_alias->setText( mAccount->alias() );
    } else {
        QString newAccountAlias = microblog->serviceName();
	QString servName = newAccountAlias;
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias)){
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
	    counter++;
	}
        setAccount( mAccount = new TwitterAccount(microblog, newAccountAlias) );
        kcfg_alias->setText( newAccountAlias );
        const QRegExp userRegExp("([a-zA-Z0-9_]){1,20}");
        QValidator *userVal = new QRegExpValidator(userRegExp, 0);
        kcfg_username->setValidator(userVal);
    }
    loadTimelinesTableState();
    kcfg_alias->setFocus(Qt::OtherFocusReason);
    connect( kcfg_register, SIGNAL( clicked() ), SLOT( slotRegisterNewAccount() ) );
    connect( kcfg_username, SIGNAL( textChanged(QString) ), SLOT( dataChanged()) );
    connect( kcfg_password, SIGNAL( textChanged(QString) ), SLOT( dataChanged()) );
}

TwitterEditAccountWidget::~TwitterEditAccountWidget()
{
}

void TwitterEditAccountWidget::dataChanged()
{
  kcfg_authorize->setIcon(KIcon("edit-find-user"));
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
    QOAuth::ParamMap otherArgs;
    otherArgs.insert( "oauth_verifier", kcfg_password->text().toUtf8() );
//     otherArgs.insert( "misc_arg2", "value2" );

    // send a request to exchange Request Token for an Access Token
    QOAuth::ParamMap reply =
        qoauth->accessToken( "https://api.twitter.com/oauth/access_token", QOAuth::POST, token,
                            tokenSecret, QOAuth::HMAC_SHA1, otherArgs );

    QString username;
    // if no error occurred, read the Access Token (and other arguments, if applicable)
    if ( qoauth->error() == QOAuth::NoError ) {
        username = reply.value( "screen_name" );
        token = reply.value( QOAuth::tokenParameterName() );
        tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        kDebug()<<"token: "<<token<<" tokenSecret: "<<tokenSecret;
    } else {
        kDebug()<<"ERROR: "<<qoauth->error();
        return 0;
    }

    mAccount->setUsername( username );
    mAccount->setOauthToken( token );
    mAccount->setOauthTokenSecret( tokenSecret );
    mAccount->setAlias(kcfg_alias->text());
    saveTimelinesTableState();
    mAccount->writeConfig();
    return mAccount;
}

void TwitterEditAccountWidget::slotRegisterNewAccount()
{
    Choqok::openUrl( KUrl("http://twitter.com/signup") );
}

void TwitterEditAccountWidget::authorizeUser()
{
    kDebug();
    qoauth = new QOAuth::Interface;
    qoauth->setManager(new KIO::Integration::AccessManager(this));
    // set the consumer key and secret
    qoauth->setConsumerKey( "VyXMf0O7CvciiUQjliYtYg" );
    qoauth->setConsumerSecret( "uD2HvsOBjzt1Vs6SnouFtuxDeHmvOOVwmn3fBVyCw0" );
    // set a timeout for requests (in msecs)
    qoauth->setRequestTimeout( 20000 );

    QOAuth::ParamMap otherArgs;
    otherArgs.insert( "oauth_callback", "oob" );

    // send a request for an unauthorized token
    QOAuth::ParamMap reply =
        qoauth->requestToken( "https://api.twitter.com/oauth/request_token",
                              QOAuth::GET, QOAuth::HMAC_SHA1 );

    // if no error occurred, read the received token and token secret
    if ( qoauth->error() == QOAuth::NoError ) {
        token = reply.value( QOAuth::tokenParameterName() );
        tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        kDebug()<<"token: "<<token << " tokenSecret: "<<tokenSecret;
        QUrl url("https://api.twitter.com/oauth/authorize");
        url.addQueryItem("oauth_token", token);
        Choqok::openUrl(url);
    } else {
        kDebug()<<"ERROR: " <<qoauth->error();
        //TODO add Error management
    }
}

void TwitterEditAccountWidget::loadTimelinesTableState()
{
    foreach(const QString &timeline, mAccount->microblog()->timelineNames()){
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        timelinesTable->setItem(newRow, 0, new QTableWidgetItem(timeline));

        QCheckBox *enable = new QCheckBox ( timelinesTable );
        enable->setChecked ( mAccount->timelineNames().contains(timeline) );
        timelinesTable->setCellWidget ( newRow, 1, enable );
    }
}

void TwitterEditAccountWidget::saveTimelinesTableState()
{
    QStringList timelines;
    int rowCount = timelinesTable->rowCount();
    for(int i=0; i<rowCount; ++i){
        QCheckBox *enable = qobject_cast<QCheckBox*>(timelinesTable->cellWidget(i, 1));
        if(enable && enable->isChecked())
            timelines<<timelinesTable->item(i, 0)->text();
    }
    timelines.removeDuplicates();
    mAccount->setTimelineNames(timelines);
}

#include "twittereditaccount.moc"
