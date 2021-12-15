/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GOO_GL_H
#define GOO_GL_H

#include <QVariantList>

#include "shortener.h"

/**
  @author Andrey Esin \<gmlastik@gmail.com\>
*/

class Goo_gl : public Choqok::Shortener
{
    Q_OBJECT
public:
    Goo_gl(QObject *parent, const QVariantList &args);
    ~Goo_gl();
    virtual QString shorten(const QString &url) override;
};

#endif //GOO_GL_H
