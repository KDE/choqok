/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "appearanceconfig.h"

#include <QWidget>

#include <KAboutData>
#include <KPluginFactory>

#include "appearancedebug.h"
#include "choqokappearancesettings.h"

K_PLUGIN_FACTORY_WITH_JSON(ChoqokAppearanceConfigFactory, "choqok_appearanceconfig.json",
                           registerPlugin<AppearanceConfig>();)

AppearanceConfig::AppearanceConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    setupUi(this);

    addConfig(Choqok::AppearanceSettings::self(), this);

    load();
}

AppearanceConfig::~AppearanceConfig()
{
}

#include "appearanceconfig.moc"
#include "moc_appearanceconfig.cpp"
