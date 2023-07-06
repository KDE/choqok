/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "yourlsconfig.h"

#include <QLineEdit>
#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>

#include "passwordmanager.h"

#include "yourlssettings.h"

K_PLUGIN_CLASS_WITH_JSON(YourlsConfig, "choqok_yourls_config.json")

YourlsConfig::YourlsConfig(QWidget *parent, const QVariantList &):
    KCModule(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mYourlsCtl"));
    ui.setupUi(wd);
    addConfig(YourlsSettings::self(), wd);
    layout->addWidget(wd);
    connect(ui.kcfg_username, &QLineEdit::textChanged, this, &YourlsConfig::emitChanged);
    connect(ui.cfg_password, &QLineEdit::textChanged, this, &YourlsConfig::emitChanged);
}

YourlsConfig::~YourlsConfig()
{

}

void YourlsConfig::load()
{
    KCModule::load();
    ui.cfg_password->setText(Choqok::PasswordManager::self()->readPassword(QStringLiteral("yourls_%1")
                             .arg(ui.kcfg_username->text())));
}

void YourlsConfig::save()
{
    KCModule::save();
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("yourls_%1").arg(ui.kcfg_username->text()),
            ui.cfg_password->text());
}

void YourlsConfig::emitChanged()
{
    Q_EMIT changed(true);
    disconnect(ui.kcfg_username, &QLineEdit::textChanged, this, &YourlsConfig::emitChanged);
    disconnect(ui.cfg_password, &QLineEdit::textChanged, this, &YourlsConfig::emitChanged);
}

#include "moc_yourlsconfig.cpp"
#include "yourlsconfig.moc"
