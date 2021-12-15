/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOK_CHOQOKTOOLS_H
#define CHOQOK_CHOQOKTOOLS_H

#include <QColor>
#include <QUrl>

#include "choqok_export.h"

namespace Choqok
{

void CHOQOK_EXPORT openUrl(const QUrl &url);

QString CHOQOK_EXPORT getColorString(const QColor &color);

}

#endif // CHOQOK_CHOQOKTOOLS_H
