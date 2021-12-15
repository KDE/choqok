/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QObject>

#include "choqok_export.h"

namespace Choqok
{
/**
@brief Singleton class to manage passwords
Read: @ref readPassword()
Write: @ref writePassword()
Remove: @ref removePassword()

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT PasswordManager : public QObject
{
    Q_OBJECT
public:
    ~PasswordManager();
    static PasswordManager *self();
    QString readPassword(const QString &alias);
    bool writePassword(const QString &alias, const QString &password);
    bool removePassword(const QString &alias);

private:
    PasswordManager();
    class Private;
    Private *const d;
    static PasswordManager *mSelf;
};
}

#endif
