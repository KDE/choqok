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

#include "imstatusconfig.h"

#include <QCheckBox>
#include <QComboBox>
#include <QPlainTextEdit>
#include <QVBoxLayout>

#include <KAboutData>
#include <KPluginFactory>

#include "imqdbus.h"
#include "imstatussettings.h"

K_PLUGIN_FACTORY_WITH_JSON(IMStatusConfigFactory, "choqok_imstatus_config.json",
                           registerPlugin < IMStatusConfig > ();)

IMStatusConfig::IMStatusConfig(QWidget *parent, const QVariantList &args) :
    KCModule(parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mIMStatusCtl"));
    ui.setupUi(wd);
    addConfig(IMStatusSettings::self(), wd);
    layout->addWidget(wd);
    setButtons(KCModule::Apply);
    connect(ui.cfg_imclient, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &IMStatusConfig::emitChanged);
    connect(ui.cfg_repeat, &QCheckBox::stateChanged, this, &IMStatusConfig::emitChanged);
    connect(ui.cfg_reply, &QCheckBox::stateChanged, this, &IMStatusConfig::emitChanged);
    connect(ui.cfg_templtate, &QPlainTextEdit::textChanged, this, &IMStatusConfig::emitChanged);
    imList = IMQDBus::scanForIMs();
    ui.cfg_imclient->addItems(imList);
}

IMStatusConfig::~IMStatusConfig()
{
}

void IMStatusConfig::load()
{
    KCModule::load();
    KConfigGroup grp(KSharedConfig::openConfig(), "IMStatus");
    IMStatusSettings::self()->load();
    ui.cfg_imclient->setCurrentIndex(imList.indexOf(IMStatusSettings::imclient()));
    ui.cfg_templtate->setPlainText(IMStatusSettings::templtate().isEmpty() ?
                                   QLatin1String("%username%: \"%status%\" at %time% from %client% (%url%)") : IMStatusSettings::templtate());
    ui.cfg_reply->setChecked(IMStatusSettings::reply());
    ui.cfg_repeat->setChecked(IMStatusSettings::repeat());
}

void IMStatusConfig::save()
{
    KCModule::save();
    IMStatusSettings::setImclient(ui.cfg_imclient->currentText());
    IMStatusSettings::setTempltate(ui.cfg_templtate->toPlainText());
    IMStatusSettings::setReply(ui.cfg_reply->isChecked());
    IMStatusSettings::setRepeat(ui.cfg_repeat->isChecked());
    IMStatusSettings::self()->save();
}

void IMStatusConfig::emitChanged()
{
    Q_EMIT changed(true);
}

#include "imstatusconfig.moc"
