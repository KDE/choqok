/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "uploader.h"

namespace Choqok
{

Choqok::Uploader::Uploader(const QString &componentName, QObject *parent)
    : Plugin(componentName, parent)
{

}

Choqok::Uploader::~Uploader()
{}

}

