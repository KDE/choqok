/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2016 Andrea Scarpino <scarpino@kde.org>

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

#include "friendicaeditaccount.h"

#include <QJsonDocument>
#include <QLineEdit>

#include <KIO/StoredTransferJob>
#include <KJobWidgets>

#include "accountmanager.h"
#include "choqoktools.h"

#include "gnusocialapiaccount.h"

#include "friendicadebug.h"
#include "friendicamicroblog.h"

FriendicaEditAccountWidget::FriendicaEditAccountWidget(FriendicaMicroBlog *microblog,
        GNUSocialApiAccount *account, QWidget *parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account), progress(nullptr), isAuthenticated(false)
{
    setupUi(this);
//     setAuthenticated(false);
//     connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));
//     connect(kcfg_authMethod, SIGNAL(currentIndexChanged(int)), SLOT(slotAuthMethodChanged(int)));
//     slotAuthMethodChanged(kcfg_authMethod->currentIndex());
    connect(kcfg_host, &QLineEdit::editingFinished, this,
            &FriendicaEditAccountWidget::slotCheckHostUrl);
    if (mAccount) {
        kcfg_alias->setText(mAccount->alias());
        kcfg_host->setText(mAccount->host());
        kcfg_api->setText(mAccount->api());
//         kcfg_oauthUsername->setText( mAccount->username() );
        kcfg_basicUsername->setText(mAccount->username());
        kcfg_basicPassword->setText(mAccount->password());
        kcfg_changeExclamationMark->setChecked(mAccount->isChangeExclamationMark());
        kcfg_changeToString->setText(mAccount->changeExclamationMarkToText());
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
        while (Choqok::AccountManager::self()->findAccount(newAccountAlias)) {
            newAccountAlias = QStringLiteral("%1%2").arg(servName).arg(counter);
            counter++;
        }
        setAccount(mAccount = new GNUSocialApiAccount(microblog, newAccountAlias));
        kcfg_alias->setText(newAccountAlias);
        const QRegExp userRegExp(QLatin1String("([a-z0-9]){1,64}"), Qt::CaseInsensitive);
        QValidator *userVal = new QRegExpValidator(userRegExp, nullptr);
        kcfg_basicUsername->setValidator(userVal);
    }
    loadTimelinesTableState();
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

FriendicaEditAccountWidget::~FriendicaEditAccountWidget()
{
}

bool FriendicaEditAccountWidget::validateData()
{
//     if( kcfg_authMethod->currentIndex()==0 ) {//OAuth
//         if(kcfg_alias->text().isEmpty() || kcfg_oauthUsername->text().isEmpty() || !isAuthenticated)
//             return false;
//     } else {//Basic
    if (kcfg_alias->text().isEmpty() || kcfg_basicUsername->text().isEmpty() ||
            kcfg_basicPassword->text().isEmpty()) {
        return false;
    }
//     }
    return true;
}

Choqok::Account *FriendicaEditAccountWidget::apply()
{
    qCDebug(CHOQOK);
    /*if(kcfg_authMethod->currentIndex() == 0){
        mAccount->setUsername( kcfg_oauthUsername->text() );
        mAccount->setOauthToken( token );
        mAccount->setOauthConsumerKey( oauthConsumerKey );
        mAccount->setOauthConsumerSecret( oauthConsumerSecret );
        mAccount->setOauthTokenSecret( tokenSecret );
        mAccount->setUsingOAuth(true);
    } else*/ {
        mAccount->setUsername(kcfg_basicUsername->text());
        mAccount->setPassword(kcfg_basicPassword->text());
        mAccount->setUsingOAuth(false);
    }
    mAccount->setHost(kcfg_host->text());
    mAccount->setApi(kcfg_api->text());
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setChangeExclamationMark(kcfg_changeExclamationMark->isChecked());
    mAccount->setChangeExclamationMarkToText(kcfg_changeToString->text());
    saveTimelinesTableState();
    setTextLimit();
    mAccount->writeConfig();
    return mAccount;
}

// void FriendicaEditAccountWidget::authorizeUser()
// {
//     qCDebug(CHOQOK);
//     slotCheckHostUrl();
//     if(QUrl(kcfg_host->text()).host()!="identi.ca"){
//         KMessageBox::sorry(this, i18n("Sorry, OAuth Method just works with Identi.ca server. You have to use basic authentication for other StatusNet servers."));
//         kcfg_authMethod->setCurrentIndex(1);
//         return;
//     }
//     qoauth = new QOAuth::Interface(new KIO::Integration::AccessManager(this), this);
//     //TODO change this to have support for self hosted StatusNets
//     qoauth->setConsumerKey( oauthConsumerKey );
//     qoauth->setConsumerSecret( oauthConsumerSecret );
//     qoauth->setRequestTimeout( 10000 );
//
//     // send a request for an unauthorized token
//     QString oauthReqTokenUrl = QString("%1/%2/oauth/request_token").arg(kcfg_host->text()).arg(kcfg_api->text());
// //     qCDebug(CHOQOK)<<oauthReqTokenUrl;
//     QOAuth::ParamMap params;
//     params.insert("oauth_callback", "oob");
//     QOAuth::ParamMap reply =
//         qoauth->requestToken( oauthReqTokenUrl, QOAuth::GET, QOAuth::HMAC_SHA1, params );
//     setAuthenticated(false);
//     kcfg_authorize->setIcon(QIcon::fromTheme("object-locked"));
//
//     // if no error occurred, read the received token and token secret
//     if ( qoauth->error() == QOAuth::NoError ) {
//         token = reply.value( QOAuth::tokenParameterName() );
//         tokenSecret = reply.value( QOAuth::tokenSecretParameterName() );
//         qCDebug(CHOQOK)<<"token: "<<token;
//         QUrl url(QString("%1/%2/oauth/authorize").arg(kcfg_host->text()).arg(kcfg_api->text()));
//         url.addQueryItem( QOAuth::tokenParameterName(), token );
//         url.addQueryItem( "oauth_token", token );
//         Choqok::openUrl(url);
//         kcfg_authorize->setEnabled(false);
//         getPinCode();
//     } else {
//         qCDebug(CHOQOK)<<"ERROR:" <<qoauth->error()<<Choqok::qoauthErrorText(qoauth->error());
//         KMessageBox::detailedError(this, i18n("Authentication Error"),
//                                    Choqok::qoauthErrorText(qoauth->error()));
//     }
// }
//
// void FriendicaEditAccountWidget::getPinCode()
// {
//     isAuthenticated = false;
//     while(!isAuthenticated){
//         QString verifier = KInputDialog::getText( i18n("Security code"),
//                                                   i18nc("Security code received from StatusNet",
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
//             qCDebug(CHOQOK)<<"token: "<<token;
//             setAuthenticated(true);
//             KMessageBox::information(this, i18n("Choqok is authorized successfully."),
//                                      i18n("Authorized"));
//         } else {
//             setAuthenticated(false);
//             qCDebug(CHOQOK)<<"ERROR: "<<qoauth->error()<<' '<<Choqok::qoauthErrorText(qoauth->error());
//             KMessageBox::detailedError(this, i18n("Authentication Error"),
//             Choqok::qoauthErrorText(qoauth->error()));
//         }
//     }
// }

void FriendicaEditAccountWidget::setTextLimit()
{
    QString url = mAccount->host() + QLatin1Char('/') + mAccount->api() + QLatin1String("/statusnet/config.json");
    KIO::StoredTransferJob *job = KIO::storedGet(QUrl(url), KIO::Reload, KIO::HideProgressInfo);
    job->exec();
    if (job->error()) {
        qCCritical(CHOQOK) << "Job error:" << job->errorString();
        return;
    }

    const QJsonDocument json = QJsonDocument::fromJson(job->data());
    if (!json.isNull()) {
        const QVariantMap siteInfos = json.toVariant().toMap()[QLatin1String("site")].toMap();
        bool ok;
        mAccount->setPostCharLimit(siteInfos[QLatin1String("textlimit")].toUInt(&ok));
        if (!ok) {
            qCDebug(CHOQOK) << "Cannot parse text limit value";
            mAccount->setPostCharLimit(140);
        }
    } else {
        qCDebug(CHOQOK) << "Cannot parse JSON reply";
    }
}

void FriendicaEditAccountWidget::loadTimelinesTableState()
{
    for (const QString &timeline: mAccount->microblog()->timelineNames()) {
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        Choqok::TimelineInfo *info = mAccount->microblog()->timelineInfo(timeline);
        QTableWidgetItem *item = new QTableWidgetItem(info->name);
        item->setData(32, timeline);
        item->setToolTip(info->description);
        timelinesTable->setItem(newRow, 0, item);

        QCheckBox *enable = new QCheckBox(timelinesTable);
        enable->setChecked(mAccount->timelineNames().contains(timeline));
        timelinesTable->setCellWidget(newRow, 1, enable);
    }
}

void FriendicaEditAccountWidget::saveTimelinesTableState()
{
    QStringList timelines;
    int rowCount = timelinesTable->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QCheckBox *enable = qobject_cast<QCheckBox *>(timelinesTable->cellWidget(i, 1));
        if (enable && enable->isChecked()) {
            timelines << timelinesTable->item(i, 0)->data(32).toString();
        }
    }
    timelines.removeDuplicates();
    mAccount->setTimelineNames(timelines);
}

// void FriendicaEditAccountWidget::slotAuthMethodChanged(int index)
// {
//     if(index == 0){
//         kcfg_BasicBox->hide();
//         kcfg_OAuthBox->show();
//     } else {
//         kcfg_BasicBox->show();
//         kcfg_OAuthBox->hide();
//     }
// }

// void FriendicaEditAccountWidget::setAuthenticated(bool authenticated)
// {
//     isAuthenticated = authenticated;
//     if(authenticated){
//         kcfg_authorize->setIcon(QIcon::fromTheme("object-unlocked"));
//         kcfg_authenticateLed->on();
//         kcfg_authenticateStatus->setText(i18n("Authenticated"));
//     } else {
//         kcfg_authorize->setIcon(QIcon::fromTheme("object-locked"));
//         kcfg_authenticateLed->off();
//         kcfg_authenticateStatus->setText(i18n("Not Authenticated"));
//     }
// }

void FriendicaEditAccountWidget::slotCheckHostUrl()
{
    if (!kcfg_host->text().isEmpty() && !kcfg_host->text().startsWith(QLatin1String("http"),
            Qt::CaseInsensitive) && !kcfg_host->text().startsWith(QLatin1String("https"))) {
        kcfg_host->setText(kcfg_host->text().prepend(QLatin1String("https://")));
    }
}

