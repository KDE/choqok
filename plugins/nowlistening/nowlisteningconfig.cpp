/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2009-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "nowlisteningconfig.h"

#include <QPlainTextEdit>
#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>

#include "nowlisteningsettings.h"

K_PLUGIN_CLASS_WITH_JSON(NowListeningConfig, "choqok_nowlistening_config.json")

NowListeningConfig::NowListeningConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mNowListeningCtl"));
    ui.setupUi(wd);
    addConfig(NowListeningSettings::self(), wd);
    layout->addWidget(wd);
    setButtons(KCModule::Apply | KCModule::Default);
    connect(ui.kcfg_templateString, &QPlainTextEdit::textChanged,
            this, &NowListeningConfig::emitChanged);
}

NowListeningConfig::~NowListeningConfig()
{

}

void NowListeningConfig::defaults()
{
    KCModule::defaults();
}

void NowListeningConfig::load()
{
    KCModule::load();
}

void NowListeningConfig::save()
{
    KCModule::save();
}

void NowListeningConfig::emitChanged()
{
    Q_EMIT changed(true);
    disconnect(ui.kcfg_templateString, &QPlainTextEdit::textChanged,
               this, &NowListeningConfig::emitChanged);
}

#include "moc_nowlisteningconfig.cpp"
#include "nowlisteningconfig.moc"
