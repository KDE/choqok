/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL

*/

#include "twitgooconfig.h"

#include <QCheckBox>
#include <QComboBox>
#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

#include "accountmanager.h"

#include "twitgoosettings.h"

K_PLUGIN_CLASS_WITH_JSON(TwitgooConfig, "choqok_twitgoo_config.json")

TwitgooConfig::TwitgooConfig(QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mTwitgooCtl"));
    ui.setupUi(wd);
    addConfig(TwitgooSettings::self(), wd);
    layout->addWidget(wd);
    connect(ui.cfg_accountsList, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &TwitgooConfig::emitChanged);
    connect(ui.cfg_directLink, &QCheckBox::stateChanged, this, &TwitgooConfig::emitChanged);
}

TwitgooConfig::~TwitgooConfig()
{
}

void TwitgooConfig::load()
{
    KCModule::load();
    QList<Choqok::Account *> list = Choqok::AccountManager::self()->accounts();
    for (Choqok::Account *acc: list) {
        if (acc->inherits("TwitterAccount")) {
            ui.cfg_accountsList->addItem(acc->alias());
        }
    }
    TwitgooSettings::self()->load();
    ui.cfg_accountsList->setCurrentText(TwitgooSettings::alias());
    ui.cfg_directLink->setChecked(TwitgooSettings::directLink());
}

void TwitgooConfig::save()
{
    if (ui.cfg_accountsList->currentIndex() > -1) {
        TwitgooSettings::setAlias(ui.cfg_accountsList->currentText());
        //qDebug() << TwitgooSettings::alias();
    } else {
        TwitgooSettings::setAlias(QString());
        KMessageBox::error(this, i18n("You have to configure at least one Twitter account to use this plugin."));
    }
    TwitgooSettings::setDirectLink(ui.cfg_directLink->isChecked());
    TwitgooSettings::self()->save();
    KCModule::save();
}

void TwitgooConfig::emitChanged()
{
    Q_EMIT changed(true);
}

#include "twitgooconfig.moc"
