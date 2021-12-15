/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef UR1_CA_H
#define UR1_CA_H

#include <QVariantList>

#include "shortener.h"

/**
@author Bhaskar Kandiyal \<bkandiyal@gmail.com\>
*/
class Ur1_ca : public Choqok::Shortener
{
    Q_OBJECT
public:
    Ur1_ca(QObject *parent, const QVariantList &args);
    ~Ur1_ca();

    virtual QString shorten(const QString &url) override;
};

#endif
