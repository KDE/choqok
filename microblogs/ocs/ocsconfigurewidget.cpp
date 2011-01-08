/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "ocsconfigurewidget.h"
#include <attica/providermanager.h>
#include "ocsaccount.h"
#include "ocsmicroblog.h"
#include <KDebug>
#include <accountmanager.h>
#include <KMessageBox>

OCSConfigureWidget::OCSConfigureWidget(OCSMicroblog *microblog, OCSAccount* account, QWidget* parent)
    : ChoqokEditAccountWidget(account, parent), mAccount(account), mMicroblog(microblog), providersLoaded(false)
{
    setupUi(this);
    cfg_provider->setCurrentItem(i18n("Loading..."), true);
    if( microblog->isOperational() ) {
        slotprovidersLoaded();
    } else {
        connect(microblog, SIGNAL(initialized()), SLOT(slotprovidersLoaded()));
    }
    if(mAccount){
        cfg_alias->setText(mAccount->alias());
    } else {
        QString newAccountAlias = microblog->serviceName();
        QString servName = newAccountAlias;
        int counter = 1;
        while(Choqok::AccountManager::self()->findAccount(newAccountAlias)){
            newAccountAlias = QString("%1%2").arg(servName).arg(counter);
            counter++;
        }
        setAccount( mAccount = new OCSAccount(microblog, newAccountAlias) );
        cfg_alias->setText( newAccountAlias );
    }
}

OCSConfigureWidget::~OCSConfigureWidget()
{

}

bool OCSConfigureWidget::validateData()
{
    if(!providersLoaded){
        KMessageBox::sorry(choqokMainWindow, i18n("You have to wait for providers list to be loaded."));
        return false;
    }
    if( !cfg_alias->text().isEmpty() && cfg_provider->currentIndex() >= 0 )
        return true;
    else
        return false;
}

Choqok::Account* OCSConfigureWidget::apply()
{
    mAccount->setAlias( cfg_alias->text() );
    mAccount->setProviderUrl( cfg_provider->itemData(cfg_provider->currentIndex()).toString() );
    mAccount->writeConfig();
    return mAccount;
}

void OCSConfigureWidget::slotprovidersLoaded()
{
    kDebug();
    cfg_provider->removeItem(0);
    providersLoaded = true;
    QList <Attica::Provider> providerList = mMicroblog->providerManager()->providers();
    int selectedIndex = 0;
    foreach(const Attica::Provider &p, providerList){
        kDebug()<<p.baseUrl();
        cfg_provider->addItem(p.name(), p.baseUrl());
        if(mAccount && mAccount->providerUrl() == p.baseUrl())
            selectedIndex = cfg_provider->count() - 1;
    }
    cfg_provider->setCurrentIndex(selectedIndex);
}

#include "ocsconfigurewidget.moc"
