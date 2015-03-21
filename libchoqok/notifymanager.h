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

#ifndef NOTIFYMANAGER_H
#define NOTIFYMANAGER_H

#include <KLocalizedString>

#include "choqok_export.h"

namespace Choqok
{

class CHOQOK_EXPORT NotifyManager
{
public:
    ~NotifyManager();

    static void error(const QString &message , const QString &title = i18n("Error"));
    static void success(const QString &message, const QString &title = i18n("Success"));

    static void newPostArrived(const QString &message, const QString &title = i18n("New posts"));
    static void shortening(const QString &message, const QString &title = i18n("Shortening a URL"));

    static void resetNotifyManager();

private:
    NotifyManager();
};
}
#endif // NOTIFYMANAGER_H
