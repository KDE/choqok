/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef APPEARANCECONFIG_H
#define APPEARANCECONFIG_H

#include "ui_appearanceconfig_base.h"

#include <KCModule>

/**
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 */
class AppearanceConfig : public KCModule, public Ui_AppearanceConfig_Base
{
    Q_OBJECT

public:
    AppearanceConfig(QWidget *parent, const QVariantList &args);
    ~AppearanceConfig();
};
#endif
