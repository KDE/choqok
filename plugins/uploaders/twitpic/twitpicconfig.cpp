/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitpicconfig.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>
#include <KLocalizedString>
#include <KMessageBox>

#include "accountmanager.h"

#include "twitpicsettings.h"

K_PLUGIN_CLASS_WITH_JSON(TwitpicConfig, "choqok_twitpic_config.json")

TwitpicConfig::TwitpicConfig(QWidget *parent, const QVariantList &):
    KCModule(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mTwitpicCtl");
    ui.setupUi(wd);
    addConfig(TwitpicSettings::self(), wd);
    layout->addWidget(wd);
    connect(ui.accountsList, SIGNAL(currentIndexChanged(int)), SLOT(emitChanged()));
}

TwitpicConfig::~TwitpicConfig()
{
    //qDebug() << TwitpicSettings::alias();
}

void TwitpicConfig::load()
{
    KCModule::load();
    QList<Choqok::Account *> list = Choqok::AccountManager::self()->accounts();
    for (Choqok::Account *acc: list) {
        if (acc->inherits("TwitterAccount")) {
            ui.accountsList->addItem(acc->alias());
        }
    }
    TwitpicSettings::self()->load();
    ui.accountsList->setCurrentText(TwitpicSettings::alias());
}

void TwitpicConfig::save()
{
    //qDebug() << ui.accountsList->currentIndex();
    if (ui.accountsList->currentIndex() > -1) {
        TwitpicSettings::setAlias(ui.accountsList->currentText());
        //qDebug() << TwitpicSettings::alias();
    } else {
        TwitpicSettings::setAlias(QString());
        KMessageBox::error(this, i18n("You have to configure at least one Twitter account to use this plugin."));
    }
    TwitpicSettings::self()->save();
    KCModule::save();
}

void TwitpicConfig::emitChanged()
{
    Q_EMIT changed(true);
}

#include "twitpicconfig.moc"
