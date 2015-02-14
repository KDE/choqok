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

#include "laconicaeditaccount.h"

#include <QDomDocument>
#include <QProgressBar>

#include <KDebug>
#include <KInputDialog>
#include <KIO/AccessManager>
#include <KIO/Job>
#include <KIO/JobClasses>
#include <KIO/NetAccess>
#include <KMessageBox>
#include <KToolInvocation>

#include <QtOAuth/QtOAuth>
#include <QtOAuth/qoauth_namespace.h>

#include <qjson/parser.h>

#include "accountmanager.h"
#include "choqoktools.h"

#include "laconicaaccount.h"
#include "laconicamicroblog.h"

LaconicaEditAccountWidget::LaconicaEditAccountWidget(LaconicaMicroBlog *microblog,
                                                    LaconicaAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account), progress(0), isAuthenticated(false)
{
    setupUi(this);
//     setAuthenticated(false);
    oauthConsumerKey = "747d09d8e7b9417f5835f04510cb86ed";//Identi.ca tokens
    oauthConsumerSecret = "57605f8507a041525a2d5c0abef15b20";
//     connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));
//     connect(kcfg_authMethod, SIGNAL(currentIndexChanged(int)), SLOT(slotAuthMethodChanged(int)));
//     slotAuthMethodChanged(kcfg_authMethod->currentIndex());
    connect(kcfg_host, SIGNAL(editingFinished()), SLOT(slotCheckHostUrl()));
    if(mAccount) {
        kcfg_alias->setText( mAccount->alias() );
        kcfg_host->setText( mAccount->host() );
        kcfg_api->setText( mAccount->api() );
//         kcfg_oauthUsername->setText( mAccount->username() );
        kcfg_basicUsername->setText( mAccount->username() );
        kcfg_basicPassword->setText( mAccount->password() );
        kcfg_changeExclamationMark->setChecked( mAccount->isChangeExclamationMark() );
        kcfg_changeToString->setText( mAccount->changeExclamationMarkToText() );
//         if(mAccount->usingOAuth()){
//             if( !mAccount->oauthConsumerKey().isEmpty() &&
//                 !mAccount->oauthConsumerSecret().isEmpty() &&
//                 !mAccount->oauthToken().isEmpty() &&
//                 !mAccount->oauthTokenSecret().isEmpty() ) {
//                 setAuthenticated(true);
//                 oauthConsumerKey = mAccount->oauthConsumerKey();
//                 oauthConsumerSecret = mAccount->oauthConsumerSecret();
//                 token = mAccount->oauthToken();
//                 tokenSecret = mAccount->oauthTokenSecret();
//             } else {
//                 setAuthenticated(false);
//             }
//              kcfg_authMethod->setCurrentIndex(0);
//         } else {
//             kcfg_authMethod->setCurrentIndex(1);
//         }
    } else {
//         kcfg_authMethod->setCurrentIndex(0);
        QString newAccountAlias = microblog->serviceName();
        QString servName = newAccountAlias;
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias)){
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
            counter++;
        }
        setAccount( mAccount = new LaconicaAccount(microblog, newAccountAlias) );
        kcfg_alias->setText( newAccountAlias );
        const QRegExp userRegExp("([a-z0-9]){1,64}", Qt::CaseInsensitive);
        QValidator *userVal = new QRegExpValidator(userRegExp, 0);
        kcfg_basicUsername->setValidator(userVal);
    }
    loadTimelinesTableState();
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

LaconicaEditAccountWidget::~LaconicaEditAccountWidget()
{
}

bool LaconicaEditAccountWidget::validateData()
{
//     if( kcfg_authMethod->currentIndex()==0 ) {//OAuth
//         if(kcfg_alias->text().isEmpty() || kcfg_oauthUsername->text().isEmpty() || !isAuthenticated)
//             return false;
//     } else {//Basic
        if(kcfg_alias->text().isEmpty() || kcfg_basicUsername->text().isEmpty() ||
            kcfg_basicPassword->text().isEmpty())
            return false;
//     }
    return true;
}

Choqok::Account* LaconicaEditAccountWidget::apply()
{
    kDebug();
    /*if(kcfg_authMethod->currentIndex() == 0){
        mAccount->setUsername( kcfg_oauthUsername->text() );
        mAccount->setOauthToken( token );
        mAccount->setOauthConsumerKey( oauthConsumerKey );
        mAccount->setOauthConsumerSecret( oauthConsumerSecret );
        mAccount->setOauthTokenSecret( tokenSecret );
        mAccount->setUsingOAuth(true);
    } else*/ {
        mAccount->setUsername( kcfg_basicUsername->text() );
        mAccount->setPassword( kcfg_basicPassword->text() );
        mAccount->setUsingOAuth(false);
    }
    mAccount->setHost( kcfg_host->text() );
    mAccount->setApi( kcfg_api->text() );
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setChangeExclamationMark(kcfg_changeExclamationMark->isChecked());
    mAccount->setChangeExclamationMarkToText(kcfg_changeToString->text());
    saveTimelinesTableState();
    setTextLimit();
    mAccount->writeConfig();
    return mAccount;
}

// void LaconicaEditAccountWidget::authorizeUser()
// {
//     kDebug();
//     slotCheckHostUrl();
//     if(KUrl(kcfg_host->text()).host()!="identi.ca"){
//         KMessageBox::sorry(this, i18n("Sorry, OAuth Method just works with Identi.ca server. You have to use basic authentication for other StatusNet servers."));
//         kcfg_authMethod->setCurrentIndex(1);
//         return;
//     }
//     qoauth = new QOAuth::Interface(new KIO::AccessManager(this), this);//TODO KDE 4.5 Change to use new class.
//     //TODO change this to have support for self hosted StatusNets
//     qoauth->setConsumerKey( oauthConsumerKey );
//     qoauth->setConsumerSecret( oauthConsumerSecret );
//     qoauth->setRequestTimeout( 10000 );
// 
//     // send a request for an unauthorized token
//     QString oauthReqTokenUrl = QString("%1/%2/oauth/request_token").arg(kcfg_host->text()).arg(kcfg_api->text());
// //     kDebug()<<oauthReqTokenUrl;
//     QOAuth::ParamMap params;
//     params.insert("oauth_callback", "oob");
//     QOAuth::ParamMap reply =
//         qoauth->requestToken( oauthReqTokenUrl, QOAuth::GET, QOAuth::HMAC_SHA1, params );
//     setAuthenticated(false);
//     kcfg_authorize->setIcon(KIcon("object-locked"));
// 
//     // if no error occurred, read the received token and token secret
//     if ( qoauth->error() == QOAuth::NoError ) {
//         token = reply.value( QOAuth::tokenParameterName() );
//         tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
//         kDebug()<<"token: "<<token;
//         QUrl url(QString("%1/%2/oauth/authorize").arg(kcfg_host->text()).arg(kcfg_api->text()));
//         url.addQueryItem( QOAuth::tokenParameterName(), token );
//         url.addQueryItem( "oauth_token", token );
//         Choqok::openUrl(url);
//         kcfg_authorize->setEnabled(false);
//         getPinCode();
//     } else {
//         kDebug()<<"ERROR: " <<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
//         KMessageBox::detailedError(this, i18n("Authentication Error"),
//                                    Choqok::qoauthErrorText(qoauth->error()));
//     }
// }
// 
// void LaconicaEditAccountWidget::getPinCode()
// {
//     isAuthenticated = false;
//     while(!isAuthenticated){
//         QString verifier = KInputDialog::getText( i18n("Security code"),
//                                                   i18nc("Security code recieved from StatusNet",
//                                                         "Enter security code:"));
//         if(verifier.isEmpty())
//             return;
//         QOAuth::ParamMap otherArgs;
//         otherArgs.insert( "oauth_verifier", verifier.toUtf8() );
// 
//         QOAuth::ParamMap reply =
//         qoauth->accessToken( QString("%1/%2/oauth/access_token").arg(kcfg_host->text()).arg(kcfg_api->text()),
//                              QOAuth::GET, token, tokenSecret, QOAuth::HMAC_SHA1, otherArgs );
//         // if no error occurred, read the Access Token (and other arguments, if applicable)
//         if ( qoauth->error() == QOAuth::NoError ) {
//             sender()->deleteLater();
//             kcfg_authorize->setEnabled(true);
//             token = reply.value( QOAuth::tokenParameterName() );
//             tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
//             kDebug()<<"token: "<<token;
//             setAuthenticated(true);
//             KMessageBox::information(this, i18n("Choqok is authorized successfully."),
//                                      i18n("Authorized"));
//         } else {
//             setAuthenticated(false);
//             kDebug()<<"ERROR: "<<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
//             KMessageBox::detailedError(this, i18n("Authentication Error"),
//             Choqok::qoauthErrorText(qoauth->error()));
//         }
//     }
// }

void LaconicaEditAccountWidget::setTextLimit()
{
    QByteArray jobData;
    QString url = mAccount->host() + "/" + mAccount->api() + "/statusnet/config.json";
    KIO::TransferJob *job = KIO::get(KUrl(url), KIO::Reload, KIO::HideProgressInfo);
    if ( !KIO::NetAccess::synchronousRun(job, 0, &jobData) ) {
        kError()<<"Job error: " << job->errorString();
        return;
    }

    bool ok;
    QJson::Parser parser;
    QVariantMap siteInfos = parser.parse(jobData, &ok).toMap()["site"].toMap();

    if (ok) {
        mAccount->setPostCharLimit(siteInfos["textlimit"].toUInt(&ok));
    } else {
        kDebug() << "Cannot parse JSON reply";
    }

    if (!ok) {
        kDebug() << "Cannot parse text limit value";
        mAccount->setPostCharLimit(140);
    }
}

void LaconicaEditAccountWidget::loadTimelinesTableState()
{
    Q_FOREACH (const QString &timeline, mAccount->microblog()->timelineNames()) {
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

// void LaconicaEditAccountWidget::slotAuthMethodChanged(int index)
// {
//     if(index == 0){
//         kcfg_BasicBox->hide();
//         kcfg_OAuthBox->show();
//     } else {
//         kcfg_BasicBox->show();
//         kcfg_OAuthBox->hide();
//     }
// }

// void LaconicaEditAccountWidget::setAuthenticated(bool authenticated)
// {
//     isAuthenticated = authenticated;
//     if(authenticated){
//         kcfg_authorize->setIcon(KIcon("object-unlocked"));
//         kcfg_authenticateLed->on();
//         kcfg_authenticateStatus->setText(i18n("Authenticated"));
//     } else {
//         kcfg_authorize->setIcon(KIcon("object-locked"));
//         kcfg_authenticateLed->off();
//         kcfg_authenticateStatus->setText(i18n("Not Authenticated"));
//     }
// }

void LaconicaEditAccountWidget::slotCheckHostUrl()
{
    if( !kcfg_host->text().isEmpty() && !kcfg_host->text().startsWith(QLatin1String("http"),
                                                                      Qt::CaseInsensitive) )
        kcfg_host->setText(kcfg_host->text().prepend("http://"));
}

#include "laconicaeditaccount.moc"
