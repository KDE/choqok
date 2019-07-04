/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017  Andrea Scarpino <scarpino@kde.org>

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

#include "mastodoneditaccountwidget.h"

#include <QCoreApplication>
#include <QCheckBox>
#include <QInputDialog>
#include <QJsonDocument>
#include <QEventLoop>
#include <QPushButton>
#include <QUrl>

#include <KIO/AccessManager>
#include <KIO/StoredTransferJob>
#include <KMessageBox>

#include "choqoktools.h"
#include "accountmanager.h"

#include "mastodonaccount.h"
#include "mastodondebug.h"
#include "mastodonmicroblog.h"
#include "mastodonoauth.h"

MastodonEditAccountWidget::MastodonEditAccountWidget(MastodonMicroBlog *microblog,
         MastodonAccount *account,
         QWidget *parent):
     ChoqokEditAccountWidget(account, parent)
     , m_account(account)
{
    setupUi(this);

    connect(kcfg_authorize, &QPushButton::clicked, this, &MastodonEditAccountWidget::authorizeUser);

    if (m_account) {
        kcfg_alias->setText(m_account->alias());
        kcfg_acct->setText(m_account->acct());
        setAuthenticated(!m_account->tokenSecret().isEmpty());
    } else {
        setAuthenticated(false);
        QString newAccountAlias = microblog->serviceName();
        const QString servName = newAccountAlias;
        int counter = 1;
        while (Choqok::AccountManager::self()->findAccount(newAccountAlias)) {
            newAccountAlias = QStringLiteral("%1%2").arg(servName).arg(counter);
            counter++;
        }
        m_account = new MastodonAccount(microblog, newAccountAlias);
        setAccount(m_account);
        kcfg_alias->setText(newAccountAlias);
    }

    loadTimelinesTable();
}

MastodonEditAccountWidget::~MastodonEditAccountWidget()
{
}

Choqok::Account *MastodonEditAccountWidget::apply()
{
    m_account->setAlias(kcfg_alias->text());
    m_account->setAcct(kcfg_acct->text());
    m_account->setTokenSecret(m_account->oAuth()->token());
    m_account->writeConfig();
    saveTimelinesTable();
    return m_account;
}

void MastodonEditAccountWidget::authorizeUser()
{
    qCDebug(CHOQOK);
    if (kcfg_acct->text().isEmpty() || !kcfg_acct->text().contains(QLatin1Char('@'))) {
        return;
    }
    if (m_account->consumerKey().isEmpty() || m_account->consumerSecret().isEmpty()) {
        registerClient();
    }

    connect(m_account->oAuth(), &QAbstractOAuth::authorizeWithBrowser, &Choqok::openUrl);
    connect(m_account->oAuth(), &QAbstractOAuth::statusChanged, this, &MastodonEditAccountWidget::gotToken);

    m_account->oAuth()->grant();

    QString verifier = QInputDialog::getText(this, i18n("coe"),
                       i18n("Enter the code received from %1", m_account->host()));
    if (verifier.isEmpty()) {
        return;
    }

    m_account->oAuth()->getToken(verifier);
}

void MastodonEditAccountWidget::gotToken()
{
    isAuthenticated = false;
    if (m_account->oAuth()->status() == QAbstractOAuth::Status::Granted) {
        setAuthenticated(true);
        KMessageBox::information(this, i18n("Choqok is authorized successfully."), i18n("Authorized"));
    } else {
        KMessageBox::detailedError(this, i18n("Authorization Error"), i18n("OAuth authorization error"));
    }
}

bool MastodonEditAccountWidget::validateData()
{
    if (kcfg_alias->text().isEmpty() || kcfg_acct->text().isEmpty() ||
            !kcfg_acct->text().contains(QLatin1Char('@')) ||
            !isAuthenticated) {
        return false;
    } else {
        return true;
    }
}

void MastodonEditAccountWidget::setAuthenticated(bool authenticated)
{
    isAuthenticated = authenticated;
    if (authenticated) {
        kcfg_authorize->setIcon(QIcon::fromTheme(QLatin1String("object-unlocked")));
        kcfg_authenticateLed->on();
        kcfg_authenticateStatus->setText(i18n("Authenticated"));
    } else {
        kcfg_authorize->setIcon(QIcon::fromTheme(QLatin1String("object-locked")));
        kcfg_authenticateLed->off();
        kcfg_authenticateStatus->setText(i18n("Not Authenticated"));
    }
}

void MastodonEditAccountWidget::loadTimelinesTable()
{
    for (const QString &timeline: m_account->microblog()->timelineNames()) {
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        timelinesTable->setItem(newRow, 0, new QTableWidgetItem(timeline));

        QCheckBox *enable = new QCheckBox(timelinesTable);
        enable->setChecked(m_account->timelineNames().contains(timeline));
        timelinesTable->setCellWidget(newRow, 1, enable);
    }
}

void MastodonEditAccountWidget::registerClient()
{
    if (kcfg_acct->text().contains(QLatin1Char('@'))) {
        m_account->setUsername(kcfg_acct->text().split(QLatin1Char('@'))[0]);
        m_account->setHost(QLatin1String("https://") + kcfg_acct->text().split(QLatin1Char('@'))[1]);

        m_account->oAuth()->setAccessTokenUrl(QUrl(m_account->host() + QLatin1String("/oauth/token")));
        m_account->oAuth()->setAuthorizationUrl(QUrl(m_account->host() + QLatin1String("/oauth/authorize")));

        QUrl url(m_account->host() + QLatin1String("/api/v1/apps"));
        QByteArray data;
        data += "client_name=" + QCoreApplication::applicationName().toLatin1();
        data += "&redirect_uris=" + QUrl::toPercentEncoding(QLatin1String("urn:ietf:wg:oauth:2.0:oob"));
        data += "&scopes=" + QUrl::toPercentEncoding(QLatin1String("read write follow"));
        data += "&website=" + QUrl::toPercentEncoding(QLatin1String("https://choqok.kde.org/"));

        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/x-www-form-urlencoded"));
        QEventLoop loop;
        connect(job, &KIO::StoredTransferJob::result, &loop, &QEventLoop::quit);
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
                m_account->setConsumerKey(result[QLatin1String("client_id")].toString());
                m_account->setConsumerSecret(result[QLatin1String("client_secret")].toString());
                m_account->oAuth()->setClientIdentifier(m_account->consumerKey());
                m_account->oAuth()->setClientIdentifierSharedKey(m_account->consumerSecret());
            } else {
                qCDebug(CHOQOK) << "Cannot parse JSON reply";
            }
        }
    } else {
        qCDebug(CHOQOK) << "username is not valid";
    }
}

void MastodonEditAccountWidget::saveTimelinesTable()
{
    QStringList timelines;
    for (int i = 0; i < timelinesTable->rowCount(); ++i) {
        QCheckBox *enable = qobject_cast<QCheckBox *>(timelinesTable->cellWidget(i, 1));
        if (enable && enable->isChecked()) {
            timelines.append(timelinesTable->item(i, 0)->text());
        }
    }
    //m_account->setTimelineNames(timelines);
}
