/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

K_PLUGIN_CLASS_WITH_JSON(IMStatusConfig, "choqok_imstatus_config.json")

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
