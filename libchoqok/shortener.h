/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef SHORTENER_H
#define SHORTENER_H

#include <QString>

#include "plugin.h"

namespace Choqok
{
/**
@brief The base class for a Shortener plugin main class.

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT Shortener : public Plugin
{
    Q_OBJECT
public:
    virtual ~Shortener();
    /**
        Shorten the @p url and return the shortened URL
    */
    virtual QString shorten(const QString &url);

protected:
    Shortener(const QString &componentName, QObject *parent);
};
}//End Namespace Choqok
#endif
