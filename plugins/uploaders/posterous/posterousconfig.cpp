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

#include "posterousconfig.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KLocalizedString>
#include <KPluginFactory>
#include <KMessageBox>

#include "accountmanager.h"
#include "passwordmanager.h"

#include "posteroussettings.h"

K_PLUGIN_FACTORY_WITH_JSON(PosterousConfigFactory, "choqok_posterous_config.json",
                           registerPlugin < PosterousConfig > ();)

PosterousConfig::PosterousConfig(QWidget *parent, const QVariantList &) :
    KCModule(KAboutData::pluginData("kcm_choqok_posterous"), parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mPosterousCtl");
    ui.setupUi(wd);
    addConfig(PosterousSettings::self(), wd);
    layout->addWidget(wd);
    ui.cfg_password->setEchoMode(QLineEdit::Password);
    connect(ui.cfg_basic, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_login, SIGNAL(textChanged(QString)), SLOT(emitChanged()));
    connect(ui.cfg_password, SIGNAL(textChanged(QString)), SLOT(emitChanged()));
    connect(ui.cfg_oauth, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_accountsList, SIGNAL(currentIndexChanged(int)), SLOT(emitChanged()));
}

PosterousConfig::~PosterousConfig()
{

}

void PosterousConfig::load()
{
    KCModule::load();
    QList<Choqok::Account *> list = Choqok::AccountManager::self()->accounts();
    Q_FOREACH (Choqok::Account *acc, list) {
        if (acc->inherits("TwitterAccount")) {
            ui.cfg_accountsList->addItem(acc->alias());
        }
    }
    PosterousSettings::self()->readConfig();
    ui.cfg_basic->setChecked(PosterousSettings::basic());
    ui.cfg_login->setText(PosterousSettings::login());
    ui.cfg_password->setText(Choqok::PasswordManager::self()->readPassword(QString("posterous_%1")
                             .arg(ui.cfg_login->text())));
    ui.cfg_oauth->setChecked(PosterousSettings::oauth());
    ui.cfg_accountsList->setCurrentItem(PosterousSettings::alias());
    emitChanged();
}

void PosterousConfig::save()
{
    if (ui.cfg_accountsList->currentIndex() > -1) {
        PosterousSettings::setAlias(ui.cfg_accountsList->currentText());
        //qDebug() << PosterousSettings::alias();
    } else {
        PosterousSettings::setAlias(QString());
        KMessageBox::error(this, i18n("You have to configure at least one Twitter account to use this plugin."));
    }

    PosterousSettings::setBasic(ui.cfg_basic->isChecked());
    PosterousSettings::setLogin(ui.cfg_login->text());
    Choqok::PasswordManager::self()->writePassword(QString("posterous_%1").arg(ui.cfg_login->text()),
            ui.cfg_password->text());
    PosterousSettings::setOauth(ui.cfg_oauth->isChecked());
    PosterousSettings::self()->writeConfig();
    KCModule::save();
}

void PosterousConfig::emitChanged()
{
    ui.cfg_login->setEnabled(ui.cfg_basic->isChecked());
    ui.cfg_password->setEnabled(ui.cfg_basic->isChecked());
    ui.cfg_accountsList->setEnabled(ui.cfg_oauth->isChecked());
    Q_EMIT changed(true);
}

#include "posterousconfig.moc"
