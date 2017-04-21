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

#include "twittereditaccount.h"

#include <QCheckBox>
#include <QInputDialog>

#include <KIO/AccessManager>
#include <KMessageBox>

#include "accountmanager.h"
#include "choqoktools.h"

#include "twitteraccount.h"
#include "twitterdebug.h"
#include "twittermicroblog.h"
#include "twitterapioauth.h"

TwitterEditAccountWidget::TwitterEditAccountWidget(TwitterMicroBlog *microblog,
        TwitterAccount *account, QWidget *parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account)
{
    setupUi(this);
    kcfg_basicAuth->hide();
    connect(kcfg_authorize, SIGNAL(clicked(bool)), SLOT(authorizeUser()));
    if (mAccount) {
        kcfg_alias->setText(mAccount->alias());
        setAuthenticated(!mAccount->oauthToken().isEmpty() && !mAccount->oauthTokenSecret().isEmpty());
    } else {
        setAuthenticated(false);
        QString newAccountAlias = microblog->serviceName();
        QString servName = newAccountAlias;
        int counter = 1;
        while (Choqok::AccountManager::self()->findAccount(newAccountAlias)) {
            newAccountAlias = QStringLiteral("%1%2").arg(servName).arg(counter);
            counter++;
        }
        setAccount(mAccount = new TwitterAccount(microblog, newAccountAlias));
        kcfg_alias->setText(newAccountAlias);
    }
    loadTimelinesTableState();
    kcfg_alias->setFocus(Qt::OtherFocusReason);
}

TwitterEditAccountWidget::~TwitterEditAccountWidget()
{

}

bool TwitterEditAccountWidget::validateData()
{
    if (kcfg_alias->text().isEmpty() || !isAuthenticated) {
        return false;
    } else {
        return true;
    }
}

Choqok::Account *TwitterEditAccountWidget::apply()
{
    qCDebug(CHOQOK);
    mAccount->setAlias(kcfg_alias->text());
    mAccount->setOauthToken(mAccount->oauthInterface()->token().toLatin1());
    mAccount->setOauthTokenSecret(mAccount->oauthInterface()->tokenSecret().toLatin1());
    saveTimelinesTableState();
    mAccount->writeConfig();
    return mAccount;
}

void TwitterEditAccountWidget::authorizeUser()
{
    qCDebug(CHOQOK);

    mAccount->oauthInterface()->grant();

    connect(mAccount->oauthInterface(), &QAbstractOAuth::authorizeWithBrowser, &Choqok::openUrl);
    connect(mAccount->oauthInterface(), &QAbstractOAuth::statusChanged, this, &TwitterEditAccountWidget::getPinCode);
}

void TwitterEditAccountWidget::getPinCode()
{
    isAuthenticated = false;
    if (mAccount->oauthInterface()->status() == QAbstractOAuth::Status::TemporaryCredentialsReceived) {
        QString verifier = QInputDialog::getText(this, i18n("PIN"),
                           i18n("Enter the PIN received from Twitter:"));
        if (verifier.isEmpty()) {
            return;
        }

        mAccount->oauthInterface()->continueGrantWithVerifier(verifier);
    } else if (mAccount->oauthInterface()->status() == QAbstractOAuth::Status::Granted) {
        setAuthenticated(true);
        KMessageBox::information(this, i18n("Choqok is authorized successfully."), i18n("Authorized"));
    } else {
        KMessageBox::detailedError(this, i18n("Authorization Error"), i18n("OAuth authorization error"));
    }
}

void TwitterEditAccountWidget::setAuthenticated(bool authenticated)
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

void TwitterEditAccountWidget::loadTimelinesTableState()
{
    for (const QString &timeline: mAccount->microblog()->timelineNames()) {
        int newRow = timelinesTable->rowCount();
        timelinesTable->insertRow(newRow);
        timelinesTable->setItem(newRow, 0, new QTableWidgetItem(timeline));

        QCheckBox *enable = new QCheckBox(timelinesTable);
        enable->setChecked(mAccount->timelineNames().contains(timeline));
        timelinesTable->setCellWidget(newRow, 1, enable);
    }
}

void TwitterEditAccountWidget::saveTimelinesTableState()
{
    QStringList timelines;
    int rowCount = timelinesTable->rowCount();
    for (int i = 0; i < rowCount; ++i) {
        QCheckBox *enable = qobject_cast<QCheckBox *>(timelinesTable->cellWidget(i, 1));
        if (enable && enable->isChecked()) {
            timelines << timelinesTable->item(i, 0)->text();
        }
    }
    timelines.removeDuplicates();
    mAccount->setTimelineNames(timelines);
}

