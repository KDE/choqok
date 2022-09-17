/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "untinyconfig.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include <untinysettings.h>
#include <QVBoxLayout>

K_PLUGIN_CLASS_WITH_JSON(UnTinyConfig, "choqok_untiny.json")

UnTinyConfig::UnTinyConfig(QWidget* parent, const QVariantList& args):
        KCModule( UnTinyConfigFactory::componentData(), parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mUnTinyCtl");
    ui.setupUi(wd);
    addConfig( UnTinySettings::self(), wd );
    layout->addWidget(wd);
    setButtons(KCModule::Apply | KCModule::Default);
}

UnTinyConfig::~UnTinyConfig()
{

}

void UnTinyConfig::defaults()
{
    KCModule::defaults();
}

void UnTinyConfig::load()
{
    KCModule::load();
}

void UnTinyConfig::save()
{
    KCModule::save();
}

void UnTinyConfig::emitChanged()
{
    Q_EMIT changed(true);
}

#include "untinyconfig.moc"
