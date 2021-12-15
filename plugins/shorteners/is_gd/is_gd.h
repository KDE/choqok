/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Felix Rohrbach <fxrh@gmx.de>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef IS_GD_H
#define IS_GD_H

#include <QVariantList>
#include <QUrlQuery>

#include "shortener.h"

/**
  @author Felix Rohrbach \<fxrh@gmx.de\>
*/

class Is_gd : public Choqok::Shortener
{
    Q_OBJECT
public:
    Is_gd(QObject *parent, const QVariantList &args);
    ~Is_gd();

    virtual QString shorten(const QString &url) override;

};

#endif //IS_GD_H
