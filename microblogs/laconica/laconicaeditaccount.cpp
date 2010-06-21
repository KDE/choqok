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
#include <QProgressBar>
#include <accountmanager.h>
#include <QtOAuth/interface.h>
#include <QtOAuth/qoauth_namespace.h>
#include <choqoktools.h>
#include <kio/accessmanager.h>

LaconicaEditAccountWidget::LaconicaEditAccountWidget(LaconicaMicroBlog *microblog,
                                                    LaconicaAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account), progress(0), isAuthorized(false)
{
    setupUi(this);
    connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));
    if(mAccount) {
        kcfg_username->setText( mAccount->username() );
        kcfg_alias->setText( mAccount->alias() );
        kcfg_secure->setChecked( mAccount->useSecureConnection() );
        kcfg_host->setText( mAccount->host() );
        kcfg_api->setText( mAccount->api() );
        kcfg_changeExclamationMark->setChecked( mAccount->isChangeExclamationMark() );
        kcfg_changeToString->setText( mAccount->changeExclamationMarkToText() );
    } else {
        QString newAccountAlias = microblog->serviceName();
        QString servName = newAccountAlias;
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias)){
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
            counter++;
        }
        setAccount( mAccount = new LaconicaAccount(microblog, newAccountAlias) );
        kcfg_alias->setText( newAccountAlias );
        const QRegExp userRegExp("([a-zA-Z0-9_]){1,64}");
        QValidator *userVal = new QRegExpValidator(userRegExp, 0);
        kcfg_username->setValidator(userVal);
    }
    loadTimelinesTableState();
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

LaconicaEditAccountWidget::~LaconicaEditAccountWidget()
{
}

bool LaconicaEditAccountWidget::validateData()
{
    if(kcfg_alias->text().isEmpty() || kcfg_username->text().isEmpty() || !isAuthorized)
        return false;
    else
        return true;
}

Choqok::Account* LaconicaEditAccountWidget::apply()
{
    kDebug();
//     QOAuth::ParamMap otherArgs;
//     otherArgs.insert( "oauth_verifier", kcfg_password->text().toUtf8() );
//     otherArgs.insert( "misc_arg2", "value2" );

    // send a request to exchange Request Token for an Access Token
    QOAuth::ParamMap reply =
        qoauth->accessToken( QString("http://%1/%2/oauth/access_token").arg(kcfg_host->text()).arg(kcfg_api->text()), QOAuth::GET, token, tokenSecret, QOAuth::HMAC_SHA1 );

    // if no error occurred, read the Access Token (and other arguments, if applicable)
    if ( qoauth->error() == QOAuth::NoError ) {
        token = reply.value( QOAuth::tokenParameterName() );
        tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        kDebug()<<"token: "<<token<<" tokenSecret: "<<tokenSecret;
    } else {
        kDebug()<<"ERROR: "<<qoauth->error();
        return 0;
    }

    mAccount->setUsername( kcfg_username->text() );
    mAccount->setOauthToken( token );
    mAccount->setOauthTokenSecret( tokenSecret );
    mAccount->setHost( kcfg_host->text() );
    mAccount->setApi( kcfg_api->text() );
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setUseSecureConnection(kcfg_secure->isChecked());
    mAccount->setChangeExclamationMark(kcfg_changeExclamationMark->isChecked());
    mAccount->setChangeExclamationMarkToText(kcfg_changeToString->text());
    saveTimelinesTableState();
    mAccount->writeConfig();
    return mAccount;
}

void LaconicaEditAccountWidget::authorizeUser()
{
    kDebug();
    qoauth = new QOAuth::Interface;
    qoauth->setManager(new KIO::Integration::AccessManager(this));
    // set the consumer key and secret
    //TODO change this to have support for self hosted StatusNets
    qoauth->setConsumerKey( "747d09d8e7b9417f5835f04510cb86ed" );
    qoauth->setConsumerSecret( "57605f8507a041525a2d5c0abef15b20" );
    // set a timeout for requests (in msecs)
    qoauth->setRequestTimeout( 10000 );

    // send a request for an unauthorized token
    QOAuth::ParamMap reply =
        qoauth->requestToken( QString("https://%1/%2/oauth/request_token").arg(kcfg_host->text()).arg(kcfg_api->text()), QOAuth::GET, QOAuth::HMAC_SHA1 );

    // if no error occurred, read the received token and token secret
    if ( qoauth->error() == QOAuth::NoError ) {
        token = reply.value( QOAuth::tokenParameterName() );
        tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
        kDebug()<<"token: "<<token << " tokenSecret: "<<tokenSecret;
        QUrl url(QString("http://%1/%2/oauth/authorize").arg(kcfg_host->text()).arg(kcfg_api->text()));
        url.addQueryItem( QOAuth::tokenParameterName(), token );
        url.addQueryItem( "oauth_token", token );
        Choqok::openUrl(url);
        isAuthorized = true;
    } else {
        kDebug()<<"ERROR: " <<qoauth->error()<<" reply:"<<reply;
        //TODO add Error management
    }
}

void LaconicaEditAccountWidget::loadTimelinesTableState()
{
    foreach(const QString &timeline, mAccount->microblog()->timelineNames()){
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        Choqok::TimelineInfo *info = mAccount->microblog()->timelineInfo(timeline);
        QTableWidgetItem *item = new QTableWidgetItem(info->name);
        item->setData(32, timeline);
        item->setToolTip(info->description);
        timelinesTable->setItem(newRow, 0, item);

        QCheckBox *enable = new QCheckBox ( timelinesTable );
        enable->setChecked ( mAccount->timelineNames().contains(timeline) );
        timelinesTable->setCellWidget ( newRow, 1, enable );
    }
}

void LaconicaEditAccountWidget::saveTimelinesTableState()
{
    QStringList timelines;
    int rowCount = timelinesTable->rowCount();
    for(int i=0; i<rowCount; ++i){
        QCheckBox *enable = qobject_cast<QCheckBox*>(timelinesTable->cellWidget(i, 1));
        if(enable && enable->isChecked())
            timelines<<timelinesTable->item(i, 0)->data(32).toString();
    }
    timelines.removeDuplicates();
    mAccount->setTimelineNames(timelines);
}

#include "laconicaeditaccount.moc"
