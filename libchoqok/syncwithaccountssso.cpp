/*************************************************************************************
 *  Copyright (C) 2013 by Alejandro Fiestas Olivares <afiestas@kde.org>              *
 *                                                                                   *
 *  This program is free software; you can redistribute it and/or                    *
 *  modify it under the terms of the GNU General Public License                      *
 *  as published by the Free Software Foundation; either version 2                   *
 *  of the License, or (at your option) any later version.                           *
 *                                                                                   *
 *  This program is distributed in the hope that it will be useful,                  *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of                   *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the                    *
 *  GNU General Public License for more details.                                     *
 *                                                                                   *
 *  You should have received a copy of the GNU General Public License                *
 *  along with this program; if not, write to the Free Software                      *
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA   *
 *************************************************************************************/

#include "syncwithaccountssso.h"

#include <QtCore/QDebug>
#include <Accounts/Account>
#include <Accounts/Manager>
#include <Accounts/AccountService>
#include <SignOn/AuthSession>
#include <SignOn/Identity>

#include <kglobal.h>
#include <KSharedConfigPtr>
#include <ksharedptr.h>
#include <KSharedConfig>
#include <kcompositejob.h>

SyncWithAccountsSSO::SyncWithAccountsSSO(const QString &type, QObject* parent)
 : KJob(parent)
 , m_type(type)
{

}

SyncWithAccountsSSO::~SyncWithAccountsSSO()
{

}

void SyncWithAccountsSSO::start()
{
    QMetaObject::invokeMethod(this, "realStart");
}

void SyncWithAccountsSSO::realStart()
{
    qDebug() << "Sync";
    m_manager = new Accounts::Manager("microblogging");
    Accounts::AccountIdList list = m_manager->accountList();
    Accounts::Account *acc = 0;
    Accounts::Service service = m_manager->service(m_type);

    Q_FOREACH(const Accounts::AccountId id, list) {
        qDebug() << "checking: " << id;
        if (isAccountSSOConfigured(id)) {
            continue;
        }

        qDebug() << "Already configured";
        acc = m_manager->account(id);

        if (!acc->enabled()) {
            continue;
        }

        qDebug() << "Enabled";
        Accounts::ServiceList services = acc->services("microblogging");
        if (services.isEmpty()) {
            continue;
        }
        qDebug() << "No microblogging";
        if (!services.contains(service)) {
            continue;
        }
        qDebug() << "No service";

        syncAccount(acc);
    }
}

void SyncWithAccountsSSO::sessionResponse(const SignOn::SessionData& data)
{
    qDebug() << "sessionResponse";
    qDebug() << data.toMap();
    QVariantMap map = data.toMap();
    map.insert("consumerToken", sender()->property("consumerToken").toString());
    map.insert("consumerSecret", sender()->property("consumerSecret").toString());
    map.insert("accountId", sender()->property("id").toInt());
    Q_EMIT accountToBeSync(sender()->property("account").toString(), map);
}

void SyncWithAccountsSSO::sessionError(const SignOn::Error& error)
{
    qDebug() << "sessionError: " << error.message();
}

void SyncWithAccountsSSO::syncAccount(Accounts::Account* acc)
{
    QVariant name = acc->value("name");

    Accounts::AccountService *service = new Accounts::AccountService(acc, m_manager->service(m_type));
    Accounts::AuthData authData = service->authData();
    qDebug() << "consumer" << authData.parameters();

    SignOn::Identity* identity = SignOn::Identity::existingIdentity(authData.credentialsId(), this);
    QPointer<SignOn::AuthSession> authSession = identity->createSession(authData.method());
    authSession->setProperty("account", name);
    authSession->setProperty("id", acc->id());
    authSession->setProperty("consumerToken", authData.parameters()["ConsumerKey"]);
    authSession->setProperty("consumerSecret", authData.parameters()["ConsumerSecret"]);
    QObject::connect(authSession, SIGNAL(response(const SignOn::SessionData&)),
                     SLOT(sessionResponse(const SignOn::SessionData&)));
    QObject::connect(authSession, SIGNAL(error(const SignOn::Error&)),
                     SLOT(sessionError(const SignOn::Error&)));


    authSession->process(authData.parameters(), authData.mechanism());
}

bool SyncWithAccountsSSO::isAccountSSOConfigured(const Accounts::AccountId& id)
{
    KSharedConfigPtr config = KGlobal::config();
    const QStringList accountGroups = config->groupList().filter( QRegExp( QString::fromLatin1( "^Account_" ) ) );
    Q_FOREACH(const QString &groupName, accountGroups) {
        KConfigGroup group (config, groupName);
        if (group.readEntry("account-sso", 0) == id) {
            return true;
        }
    }

    return false;
}
