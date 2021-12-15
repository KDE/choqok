/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef URLUTILS_H
#define URLUTILS_H

#include <QObject>
#include <QStringList>

#include "choqok_export.h"

class CHOQOK_EXPORT UrlUtils : public QObject
{
    Q_OBJECT

public:
    UrlUtils();
    ~UrlUtils();

    static QStringList detectUrls(const QString &text);
    static QString detectEmails(const QString &text);

private:
    static const QRegExp mUrlRegExp;
    static const QRegExp mEmailRegExp;
};

#endif // URLUTILS_H
