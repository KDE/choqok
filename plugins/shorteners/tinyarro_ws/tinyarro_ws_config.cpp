/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Andrey Esin <gmlastik@gmail.com>

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

#include "tinyarro_ws_config.h"

#include <QComboBox>
#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KSharedConfig>

#include "passwordmanager.h"

#include "tinyarro_ws_settings.h"

K_PLUGIN_FACTORY_WITH_JSON(Tinyarro_ws_ConfigFactory, "choqok_tinyarro_ws_config.json",
                           registerPlugin < Tinyarro_ws_Config > ();)

Tinyarro_ws_Config::Tinyarro_ws_Config(QWidget *parent, const QVariantList &):
    KCModule(KAboutData::pluginData(QLatin1String("kcm_choqok_tinyarro_ws")), parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mTinyarro_ws_Ctl"));
    ui.setupUi(wd);
    addConfig(Tinyarro_ws_Settings::self(), wd);
    layout->addWidget(wd);
    QString domain = QLatin1String(".ws");
    hostList.insert(QChar(0x27A8) + domain, QLatin1String("xn--ogi.ws"));
    hostList.insert(QChar(0x27AF) + domain, QLatin1String("xn--vgi.ws"));
    hostList.insert(QChar(0x2794) + domain, QLatin1String("xn--3fi.ws"));
    hostList.insert(QChar(0x279E) + domain, QLatin1String("xn--egi.ws"));
    hostList.insert(QChar(0x27BD) + domain, QLatin1String("xn--9gi.ws"));
    hostList.insert(QChar(0x27B9) + domain, QLatin1String("xn--5gi.ws"));
    hostList.insert(QChar(0x2729) + domain, QLatin1String("xn--1ci.ws"));
    hostList.insert(QChar(0x273F) + domain, QLatin1String("xn--odi.ws"));
    hostList.insert(QChar(0x2765) + domain, QLatin1String("xn--rei.ws"));
    hostList.insert(QChar(0x203A) + domain, QLatin1String("xn--cwg.ws"));
    hostList.insert(QChar(0x2318) + domain, QLatin1String("xn--bih.ws"));
    hostList.insert(QChar(0x203D) + domain, QLatin1String("xn--fwg.ws"));
    hostList.insert(QChar(0x2601) + domain, QLatin1String("xn--l3h.ws"));
    hostList.insert(QLatin1String("ta.gd"),                  QLatin1String("ta.gd"));
    hostList.insert(i18n("Random host"),    QLatin1String("Random"));

    for (const QString &host: hostList.keys()) {
        ui.kcfg_tinyarro_ws_host->addItem(host);
    }

    connect(ui.kcfg_tinyarro_ws_host, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &Tinyarro_ws_Config::emitChanged);
}

Tinyarro_ws_Config::~Tinyarro_ws_Config()
{
}

void Tinyarro_ws_Config::load()
{
    KCModule::load();
    KConfigGroup grp(KSharedConfig::openConfig(), "Tinyarro.ws Shortener");
    ui.kcfg_tinyarro_ws_host->setCurrentIndex(grp.readEntry("tinyarro_ws_host", "0").toInt());
}

void Tinyarro_ws_Config::save()
{
    KCModule::save();
    KConfigGroup grp(KSharedConfig::openConfig(), "Tinyarro.ws Shortener");
    grp.writeEntry("tinyarro_ws_host", ui.kcfg_tinyarro_ws_host->currentIndex());
    grp.writeEntry("tinyarro_ws_host_punny", hostList[ui.kcfg_tinyarro_ws_host->currentText()]);
}

void Tinyarro_ws_Config::emitChanged()
{
    Q_EMIT changed(true);
}

#include "tinyarro_ws_config.moc"
