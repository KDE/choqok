/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef BIT_LY_H
#define BIT_LY_H

#include <QVariantList>
#include <QUrlQuery>

#include "shortener.h"

/**
  @author Andrey Esin \<gmlastik@gmail.com\>
*/

class Bit_ly : public Choqok::Shortener
{
    Q_OBJECT
public:
    Bit_ly(QObject *parent, const QVariantList &args);
    ~Bit_ly();

    virtual QString shorten(const QString &url) override;
};

#endif //BIT_LY_H
