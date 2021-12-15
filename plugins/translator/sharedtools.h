/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SHAREDTOOLS_H
#define SHAREDTOOLS_H

#include <QObject>
#include <QMap>
#include <QStringList>

class SharedTools : public QObject
{
	Q_OBJECT
public:
    static SharedTools* self();
    virtual ~SharedTools();
    QMap<QString, QString> missingLangs() const;
    QStringList languageCodes() const;
	QString languageFlag( const QString& languageCode ) const;
private:
    static SharedTools* _self;
    SharedTools();
    QMap<QString, QString> _missingLangs;
    QStringList _languageCodes;
};

#endif // SHAREDTOOLS_H
