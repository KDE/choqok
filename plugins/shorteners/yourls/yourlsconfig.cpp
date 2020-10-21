/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, see http://www.gnu.org/licenses/

*/

#include "yourlsconfig.h"

#include <QLineEdit>
#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>

#include "passwordmanager.h"

#include "yourlssettings.h"

K_PLUGIN_FACTORY_WITH_JSON(YourlsConfigFactory, "choqok_yourls_config.json",
                           registerPlugin < YourlsConfig > ();)

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

#include "yourlsconfig.moc"
