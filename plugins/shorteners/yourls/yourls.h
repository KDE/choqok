/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2011 Marcello Ceschia <marcelloceschia@users.sourceforge.net>
    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef YOURLS_H
#define YOURLS_H

#include <QVariantList>
#include <QUrlQuery>

#include "shortener.h"

/**
@author Marcello Ceschia \<marcelloceschia@users.sourceforge.net\>
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class Yourls : public Choqok::Shortener
{
    Q_OBJECT

public:
    Yourls(QObject *parent, const QVariantList &args);
    ~Yourls();

    virtual QString shorten(const QString &url) override;

private Q_SLOTS:
    void reloadConfigs();
private:
    QString password;
};

#endif
