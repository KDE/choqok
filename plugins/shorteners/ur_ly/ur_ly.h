/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010 Scott Banwart <sbanwart@rogue-technology.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UR_LY_H
#define UR_LY_H

#include <QVariantList>
#include <QUrlQuery>

#include "shortener.h"

/**
@author Scott Banwart \<sbanwart@rogue-technology.com\>
*/
class Ur_ly : public Choqok::Shortener
{
    Q_OBJECT
public:
    Ur_ly(QObject *parent, const QVariantList &args);
    ~Ur_ly();

    virtual QString shorten(const QString &url);
};

#endif
