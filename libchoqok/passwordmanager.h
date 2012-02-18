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
#ifndef PASSWORDMANAGER_H
#define PASSWORDMANAGER_H

#include <QtCore/QObject>
#include <QtGui/qwindowdefs.h>
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
    QString readPassword(const QString& alias);
    bool writePassword(const QString &alias, const QString &password);
    bool removePassword(const QString& alias);

private:
    PasswordManager();
    class Private;
    Private * const d;
    static PasswordManager *mSelf;
};
}

#endif
