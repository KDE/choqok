/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013  Andrea Scarpino <scarpino@kde.org>

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

#include "pumpioeditaccountwidget.h"

#include <QCheckBox>
#include <QJsonDocument>
#include <QEventLoop>
#include <QUrl>

#include <KInputDialog>
#include <KIO/AccessManager>
#include <KIO/Job>
#include <KIO/StoredTransferJob>
#include <KMessageBox>

#include "choqoktools.h"
#include "accountmanager.h"

#include "pumpioaccount.h"
#include "pumpiodebug.h"
#include "pumpiomicroblog.h"

PumpIOEditAccountWidget::PumpIOEditAccountWidget(PumpIOMicroBlog* microblog,
                                                 PumpIOAccount* account,
                                                 QWidget* parent):
    ChoqokEditAccountWidget(account, parent)
    , m_account(account)
{
    setupUi(this);

    connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));

    if (m_account) {
        kcfg_alias->setText(m_account->alias());
        kcfg_webfingerid->setText(m_account->webfingerID());
        isAuthenticated();
    } else {
        QString newAccountAlias = microblog->serviceName();
        const QString servName = newAccountAlias;
        int counter = 1;
        while (Choqok::AccountManager::self()->findAccount(newAccountAlias)) {
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
            counter++;
        }
        m_account = new PumpIOAccount(microblog, newAccountAlias);
        setAccount(m_account);
        kcfg_alias->setText(newAccountAlias);
    }

    loadTimelinesTable();
}

PumpIOEditAccountWidget::~PumpIOEditAccountWidget()
{
}

Choqok::Account* PumpIOEditAccountWidget::apply()
{
    m_account->setAlias(kcfg_alias->text());
    m_account->setUsername(kcfg_webfingerid->text().split('@')[0]);
    m_account->writeConfig();
    saveTimelinesTable();
    return m_account;
}

void PumpIOEditAccountWidget::authorizeUser()
{
    qCDebug(CHOQOK);
    m_qoauth = new QOAuth::Interface(new KIO::Integration::AccessManager(this), this);
    if (m_account->consumerKey().isEmpty() || m_account->consumerSecret().isEmpty()) {
        registerClient();
    }
    m_qoauth->setConsumerKey(m_account->consumerKey().toLocal8Bit());
    m_qoauth->setConsumerSecret(m_account->consumerSecret().toLocal8Bit());

    QOAuth::ParamMap oAuthParams;
    oAuthParams.insert("oauth_callback", "oob");
    QOAuth::ParamMap oAuthRequest = m_qoauth->requestToken(m_account->host() + "/oauth/request_token",
                                                    QOAuth::GET, QOAuth::HMAC_SHA1, oAuthParams);

    if (m_qoauth->error() == QOAuth::NoError) {
        const QString token = oAuthRequest.value(QOAuth::tokenParameterName());
        const QString tokenSecret = oAuthRequest.value(QOAuth::tokenSecretParameterName());

        QUrl oAuthAuthorizeURL(m_account->host() + "/oauth/authorize");
        oAuthAuthorizeURL.addQueryItem("oauth_token", token);
        Choqok::openUrl(oAuthAuthorizeURL);
        QString verifier = KInputDialog::getText( i18n("PIN"),
                           i18n("Enter the verifier code received from %1", m_account->host()));

        QOAuth::ParamMap oAuthVerifierParams;
        oAuthVerifierParams.insert("oauth_verifier", verifier.toUtf8());
        QOAuth::ParamMap oAuthVerifierRequest = m_qoauth->accessToken(
                                                m_account->host() + "/oauth/access_token",
                                                QOAuth::POST, token.toLocal8Bit(),
                                                tokenSecret.toLocal8Bit(),
                                                QOAuth::HMAC_SHA1, oAuthVerifierParams);
        if (m_qoauth->error() == QOAuth::NoError) {
            m_account->setToken(oAuthVerifierRequest.value(QOAuth::tokenParameterName()));
            m_account->setTokenSecret(oAuthVerifierRequest.value(QOAuth::tokenSecretParameterName()));
            if (isAuthenticated()) {
                KMessageBox::information(this, i18n("Choqok is authorized successfully."),
                                        i18n("Authorized"));
            }
        } else {
            qCDebug(CHOQOK) << "QOAuth error: " + Choqok::qoauthErrorText(m_qoauth->error());
        }
    } else {
        qCDebug(CHOQOK) << "QOAuth error: " + Choqok::qoauthErrorText(m_qoauth->error());
    }
}

bool PumpIOEditAccountWidget::validateData()
{
    if (kcfg_webfingerid->text().isEmpty() || !kcfg_webfingerid->text().contains('@')
        || !isAuthenticated()) {
        return false;
    } else {
        return true;
    }
}

bool PumpIOEditAccountWidget::isAuthenticated()
{
    if (m_account->token().isEmpty() || m_account->tokenSecret().isEmpty()) {
        return false;
    } else {
        kcfg_authorize->setIcon(QIcon::fromTheme("object-unlocked"));
        kcfg_authenticateLed->setState(KLed::On);
        kcfg_authenticateStatus->setText(i18n("Authenticated"));
        return true;
    }
}

void PumpIOEditAccountWidget::loadTimelinesTable()
{
     Q_FOREACH (const QString &timeline, m_account->microblog()->timelineNames()) {
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        timelinesTable->setItem(newRow, 0, new QTableWidgetItem(timeline));

        QCheckBox *enable = new QCheckBox(timelinesTable);
        enable->setChecked(m_account->timelineNames().contains(timeline));
        timelinesTable->setCellWidget(newRow, 1, enable);
    }
}

void PumpIOEditAccountWidget::registerClient()
{
    if (kcfg_webfingerid->text().contains('@')) {
        m_account->setHost("https://" + kcfg_webfingerid->text().split('@')[1]);
        QUrl url(m_account->host() + "/api/client/register");
        QByteArray data("{"
                        " \"type\": \"client_associate\", "
                        " \"application_type\": \"native\", "
                        " \"application_name\": \"Choqok\" "
                        "}");
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        if ( !job ) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        job->addMetaData("content-type", "Content-Type: application/json");
        QEventLoop loop;
        connect(job, SIGNAL(result(KJob*)), &loop, SLOT(quit()));
        job->start();
        loop.exec();

        if (job->error()) {
            qCDebug(CHOQOK) << "An error occurred in Job";
            return;
        } else {
            KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *>(job);

            const QJsonDocument json = QJsonDocument::fromJson(stj->data());
            if (!json.isNull()) {
                const QVariantMap result = json.toVariant().toMap();
                m_account->setConsumerKey(result["client_id"].toString());
                m_account->setConsumerSecret(result["client_secret"].toString());
            } else {
                qCDebug(CHOQOK) << "Cannot parse JSON reply";
            }
        }
    } else {
        qCDebug(CHOQOK) << "webfingerID is not valid";
    }
}

void PumpIOEditAccountWidget::saveTimelinesTable()
{
    QStringList timelines;
    for (int i = 0; i < timelinesTable->rowCount(); ++i) {
        QCheckBox *enable = qobject_cast<QCheckBox*>(timelinesTable->cellWidget(i, 1));
        if (enable && enable->isChecked()) {
            timelines.append(timelinesTable->item(i, 0)->text());
        }
    }
    m_account->setTimelineNames(timelines);
}
