/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
