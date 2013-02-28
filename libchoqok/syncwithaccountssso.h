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

#ifndef SYNC_WITH_ACCOUNTS_SSH_H
#define SYNC_WITH_ACCOUNTS_SSH_H

#include <KJob>
#include <Accounts/Account>

namespace SignOn
{
    class Error;
    class SessionData;
}
namespace Accounts
{
    class Manager;
}
class SyncWithAccountsSSO : public KJob
{
    Q_OBJECT
    public:
        SyncWithAccountsSSO(const QString &type, QObject* parent = 0);
        virtual ~SyncWithAccountsSSO();

        virtual void start();

    private Q_SLOTS:
        void realStart();
        void sessionResponse(const SignOn::SessionData &data);
        void sessionError(const SignOn::Error &error);

    Q_SIGNALS:
        void accountToBeSync(const QString &alias, const QVariantMap &info);

    private:
        void syncAccount(Accounts::Account *acc);
        bool isAccountSSOConfigured(const Accounts::AccountId &id);

        QString m_type;
        Accounts::Manager *m_manager;
};

#endif //SYNC_WITH_ACCOUNTS_SSH_H