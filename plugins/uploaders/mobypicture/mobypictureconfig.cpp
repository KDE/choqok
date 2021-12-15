/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "mobypictureconfig.h"

#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>
#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

#include "mobypicturesettings.h"

#include "accountmanager.h"
#include "passwordmanager.h"

K_PLUGIN_FACTORY_WITH_JSON(MobypictureConfigFactory, "choqok_mobypicture_config.json",
                           registerPlugin < MobypictureConfig > ();)

MobypictureConfig::MobypictureConfig(QWidget *parent, const QVariantList &)
    : KCModule(parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mMobypictureCtl"));
    ui.setupUi(wd);
    addConfig(MobypictureSettings::self(), wd);
    layout->addWidget(wd);
    ui.cfg_pin->setEchoMode(QLineEdit::Password);
    connect(ui.cfg_basic, &QRadioButton::clicked, this, &MobypictureConfig::emitChanged);
    connect(ui.cfg_login, &QLineEdit::textChanged, this, &MobypictureConfig::emitChanged);
    connect(ui.cfg_pin, &QLineEdit::textChanged, this, &MobypictureConfig::emitChanged);
    connect(ui.cfg_oauth, &QRadioButton::clicked, this, &MobypictureConfig::emitChanged);
    connect(ui.cfg_accountsList, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &MobypictureConfig::emitChanged);
}

MobypictureConfig::~MobypictureConfig()
{
}

void MobypictureConfig::load()
{
    KCModule::load();
    QList<Choqok::Account *> list = Choqok::AccountManager::self()->accounts();
    for (Choqok::Account *acc: list) {
        if (acc->inherits("TwitterAccount")) {
            ui.cfg_accountsList->addItem(acc->alias());
        }
    }
    MobypictureSettings::self()->load();
    ui.cfg_basic->setChecked(MobypictureSettings::basic());
    ui.cfg_login->setText(MobypictureSettings::login());
    ui.cfg_pin->setText(Choqok::PasswordManager::self()->readPassword(QStringLiteral("mobypicture_%1")
                        .arg(ui.cfg_login->text())));
    ui.cfg_oauth->setChecked(MobypictureSettings::oauth());
    ui.cfg_accountsList->setCurrentText(MobypictureSettings::alias());
    emitChanged();
}

void MobypictureConfig::save()
{
    if (ui.cfg_accountsList->currentIndex() > -1) {
        //qDebug() << MobypictureSettings::alias();
        MobypictureSettings::setAlias(ui.cfg_accountsList->currentText());
    } else {
        MobypictureSettings::setAlias(QString());
        KMessageBox::error(this, i18n("You have to configure at least one Twitter account to use this plugin."));
    }

    MobypictureSettings::setBasic(ui.cfg_basic->isChecked());
    MobypictureSettings::setLogin(ui.cfg_login->text());
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("mobypicture_%1").arg(ui.cfg_login->text()),
            ui.cfg_pin->text());
    MobypictureSettings::setOauth(ui.cfg_oauth->isChecked());
    MobypictureSettings::self()->save();
    KCModule::save();
}

void MobypictureConfig::emitChanged()
{
    ui.cfg_login->setEnabled(ui.cfg_basic->isChecked());
    ui.cfg_pin->setEnabled(ui.cfg_basic->isChecked());
    ui.cfg_accountsList->setEnabled(ui.cfg_oauth->isChecked());
    Q_EMIT changed(true);
}

#include "mobypictureconfig.moc"
