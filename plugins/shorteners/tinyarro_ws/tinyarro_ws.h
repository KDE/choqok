/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TINYARROWS_WS_H
#define TINYARROWS_WS_H

#include <QVariantList>
#include <QUrlQuery>

#include "shortener.h"

/**
  @author Andrey Esin \<gmlastik@gmail.com\>
*/
class Tinyarro_ws : public Choqok::Shortener
{
    Q_OBJECT

public:
    Tinyarro_ws(QObject *parent, const QVariantList &args);
    ~Tinyarro_ws();

    virtual QString shorten(const QString &url) override;
};

#endif
