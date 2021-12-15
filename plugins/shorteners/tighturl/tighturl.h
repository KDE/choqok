/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TIGHTURL_H
#define TIGHTURL_H

#include <QVariantList>
#include <QUrlQuery>

#include "shortener.h"
/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class TightUrl : public Choqok::Shortener
{
    Q_OBJECT
public:
    TightUrl(QObject *parent, const QVariantList &args);
    ~TightUrl();

    virtual QString shorten(const QString &url) override;
};

#endif
