/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "is_gd_config.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>

#include "notifymanager.h"

#include "is_gd_settings.h"

K_PLUGIN_CLASS_WITH_JSON(Is_gd_Config, "choqok_is_gd_config.json")

Is_gd_Config::Is_gd_Config(QWidget *parent, const QVariantList &):
    KCModule(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mIsGdCtl"));
    wd->setMinimumWidth(200);
    ui.setupUi(wd);
    addConfig(Is_gd_Settings::self(), wd);
    layout->addWidget(wd);
}

Is_gd_Config::~Is_gd_Config()
{
}

void Is_gd_Config::load()
{
    KCModule::load();
}

void Is_gd_Config::save()
{
    KCModule::save();
}

void Is_gd_Config::emitChanged()
{
    Q_EMIT changed(true);
}

#include "is_gd_config.moc"
#include "moc_is_gd_config.cpp"
