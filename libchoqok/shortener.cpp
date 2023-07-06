/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "shortener.h"

namespace Choqok
{

Shortener::Shortener(const QString &componentName, QObject *parent)
    : Plugin(componentName, parent)
{
}

Shortener::~Shortener()
{}

QString Shortener::shorten(const QString &url)
{
    return url;
}
}

#include "moc_shortener.cpp"
