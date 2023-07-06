/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pumpioeditaccountwidget.h"

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

#include "pumpioaccount.h"
#include "pumpiodebug.h"
#include "pumpiomicroblog.h"
#include "pumpiooauth.h"

PumpIOEditAccountWidget::PumpIOEditAccountWidget(PumpIOMicroBlog *microblog,
        PumpIOAccount *account,
        QWidget *parent):
    ChoqokEditAccountWidget(account, parent)
    , m_account(account)
{
    setupUi(this);

    connect(kcfg_authorize, &QPushButton::clicked, this, &PumpIOEditAccountWidget::authorizeUser);

    if (m_account) {
        kcfg_alias->setText(m_account->alias());
        kcfg_webfingerid->setText(m_account->webfingerID());
        setAuthenticated(!m_account->token().isEmpty() && !m_account->tokenSecret().isEmpty());
    } else {
        setAuthenticated(false);
        QString newAccountAlias = microblog->serviceName();
        const QString servName = newAccountAlias;
        int counter = 1;
        while (Choqok::AccountManager::self()->findAccount(newAccountAlias)) {
            newAccountAlias = QStringLiteral("%1%2").arg(servName).arg(counter);
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

Choqok::Account *PumpIOEditAccountWidget::apply()
{
    m_account->setAlias(kcfg_alias->text());
    m_account->setUsername(kcfg_webfingerid->text().split(QLatin1Char('@'))[0]);
    m_account->setToken(m_account->oAuth()->token());
    m_account->setTokenSecret(m_account->oAuth()->tokenSecret());
    m_account->writeConfig();
    saveTimelinesTable();
    return m_account;
}

void PumpIOEditAccountWidget::authorizeUser()
{
    qCDebug(CHOQOK);
    if (kcfg_webfingerid->text().isEmpty() || !kcfg_webfingerid->text().contains(QLatin1Char('@'))) {
        return;
    }
    if (m_account->consumerKey().isEmpty() || m_account->consumerSecret().isEmpty()) {
        registerClient();
    }

    m_account->oAuth()->grant();

    connect(m_account->oAuth(), &QAbstractOAuth::authorizeWithBrowser, &Choqok::openUrl);
    connect(m_account->oAuth(), &QAbstractOAuth::statusChanged, this, &PumpIOEditAccountWidget::getPinCode);
}

void PumpIOEditAccountWidget::getPinCode()
{
    isAuthenticated = false;
    if (m_account->oAuth()->status() == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        QString verifier = QInputDialog::getText(this, i18n("PIN"),
                           i18n("Enter the verifier code received from %1", m_account->host()));
        if (verifier.isEmpty()) {
            return;
        }

        m_account->oAuth()->continueGrantWithVerifier(verifier);
    } else if (m_account->oAuth()->status() == QAbstractOAuth::Status::Granted) {
        setAuthenticated(true);
        KMessageBox::information(this, i18n("Choqok is authorized successfully."), i18n("Authorized"));
    } else {
        KMessageBox::detailedError(this, i18n("Authorization Error"), i18n("OAuth authorization error"));
    }
}

bool PumpIOEditAccountWidget::validateData()
{
    if (kcfg_alias->text().isEmpty() || kcfg_webfingerid->text().isEmpty() ||
            !kcfg_webfingerid->text().contains(QLatin1Char('@')) ||
            !isAuthenticated) {
        return false;
    } else {
        return true;
    }
}

void PumpIOEditAccountWidget::setAuthenticated(bool authenticated)
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

void PumpIOEditAccountWidget::loadTimelinesTable()
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

void PumpIOEditAccountWidget::registerClient()
{
    if (kcfg_webfingerid->text().contains(QLatin1Char('@'))) {
        m_account->setHost(QLatin1String("https://") + kcfg_webfingerid->text().split(QLatin1Char('@'))[1]);

        m_account->oAuth()->setTemporaryCredentialsUrl(QUrl(m_account->host() + QLatin1String("/oauth/request_token")));
        m_account->oAuth()->setAuthorizationUrl(QUrl(m_account->host() + QLatin1String("/oauth/authorize")));
        m_account->oAuth()->setTokenCredentialsUrl(QUrl(m_account->host() + QLatin1String("/oauth/access_token")));

        QUrl url(m_account->host() + QLatin1String("/api/client/register"));
        QByteArray data("{"
                        " \"type\": \"client_associate\", "
                        " \"application_type\": \"native\", "
                        " \"application_name\": \"Choqok\" "
                        "}");
        KIO::StoredTransferJob *job = KIO::storedHttpPost(data, url, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http POST request!";
            return;
        }
        job->addMetaData(QLatin1String("content-type"), QLatin1String("Content-Type: application/json"));
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
                m_account->oAuth()->setClientSharedSecret(m_account->consumerSecret());
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
        QCheckBox *enable = qobject_cast<QCheckBox *>(timelinesTable->cellWidget(i, 1));
        if (enable && enable->isChecked()) {
            timelines.append(timelinesTable->item(i, 0)->text());
        }
    }
    m_account->setTimelineNames(timelines);
}

#include "moc_pumpioeditaccountwidget.cpp"
