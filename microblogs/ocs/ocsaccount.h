/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef OCSACCOUNT_H
#define OCSACCOUNT_H

#include "account.h"

#include <Attica/Provider>

class OCSMicroblog;

class OCSAccount : public Choqok::Account
{
    Q_OBJECT
public:
    OCSAccount(OCSMicroblog *parent, const QString &alias);
    ~OCSAccount();

    QUrl providerUrl() const;
    void setProviderUrl(const QUrl &url);

    Attica::Provider provider();

    virtual void writeConfig() override;

protected Q_SLOTS:
    void slotDefaultProvidersLoaded();

private:
    class Private;
    Private *const d;
};

#endif // OCSACCOUNT_H
