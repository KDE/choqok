/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "ocsconfigurewidget.h"

#include <KMessageBox>
#include <KLocalizedString>

#include <Attica/ProviderManager>

#include "accountmanager.h"

#include "ocsaccount.h"
#include "ocsmicroblog.h"
#include "ocsdebug.h"

OCSConfigureWidget::OCSConfigureWidget(OCSMicroblog *microblog, OCSAccount *account, QWidget *parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account), mMicroblog(microblog), providersLoaded(false)
{
    setupUi(this);
    cfg_provider->setCurrentText(i18n("Loading..."));
    if (microblog->isOperational()) {
        slotprovidersLoaded();
    } else {
        connect(microblog, &OCSMicroblog::initialized, this,
                &OCSConfigureWidget::slotprovidersLoaded);
    }
    if (mAccount) {
        cfg_alias->setText(mAccount->alias());
    } else {
        QString newAccountAlias = microblog->serviceName();
        QString servName = newAccountAlias;
        int counter = 1;
        while (Choqok::AccountManager::self()->findAccount(newAccountAlias)) {
            newAccountAlias = QStringLiteral("%1%2").arg(servName).arg(counter);
            counter++;
        }
        setAccount(mAccount = new OCSAccount(microblog, newAccountAlias));
        cfg_alias->setText(newAccountAlias);
    }
}

OCSConfigureWidget::~OCSConfigureWidget()
{

}

bool OCSConfigureWidget::validateData()
{
    if (!providersLoaded) {
      KMessageBox::error(
          choqokMainWindow,
          i18n("You have to wait for providers list to be loaded."));
      return false;
    }
    if (!cfg_alias->text().isEmpty() && cfg_provider->currentIndex() >= 0) {
        return true;
    } else {
        return false;
    }
}

Choqok::Account *OCSConfigureWidget::apply()
{
    mAccount->setAlias(cfg_alias->text());
    mAccount->setProviderUrl(cfg_provider->itemData(cfg_provider->currentIndex()).toUrl());
    mAccount->writeConfig();
    return mAccount;
}

void OCSConfigureWidget::slotprovidersLoaded()
{
    qCDebug(CHOQOK);
    cfg_provider->removeItem(0);
    providersLoaded = true;
    QList <Attica::Provider> providerList = mMicroblog->providerManager()->providers();
    int selectedIndex = 0;
    for (const Attica::Provider &p: providerList) {
        qCDebug(CHOQOK) << p.baseUrl();
        cfg_provider->addItem(p.name(), p.baseUrl());
        if (mAccount && mAccount->providerUrl() == p.baseUrl()) {
            selectedIndex = cfg_provider->count() - 1;
        }
    }
    cfg_provider->setCurrentIndex(selectedIndex);
}

#include "moc_ocsconfigurewidget.cpp"
