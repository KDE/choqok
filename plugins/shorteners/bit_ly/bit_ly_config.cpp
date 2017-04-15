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

#include "bit_ly_config.h"

#include <QVBoxLayout>

#include <KAboutData>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KSharedConfig>

#include "notifymanager.h"
#include "passwordmanager.h"

#include "bit_ly_settings.h"

K_PLUGIN_FACTORY_WITH_JSON(Bit_ly_ConfigFactory, "choqok_bit_ly_config.json",
                           registerPlugin < Bit_ly_Config > ();)

Bit_ly_Config::Bit_ly_Config(QWidget *parent, const QVariantList &):
    KCModule(KAboutData::pluginData(QLatin1String("kcm_choqok_bit_ly")), parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mBitLYCtl"));
    wd->setMinimumWidth(400);
    ui.setupUi(wd);
    addConfig(Bit_ly_Settings::self(), wd);
    layout->addWidget(wd);

    QRegExp rx(QLatin1String("([a-z0-9_]){4,32}"), Qt::CaseInsensitive);
    QValidator *val0 = new QRegExpValidator(rx, 0);
    ui.kcfg_login->setValidator(val0);
    rx.setPattern(QLatin1String("([a-z0-9_]){1,40}"));
    QValidator *val1 = new QRegExpValidator(rx, 0);
    ui.kcfg_api_key->setValidator(val1);

    ui.help_label->setTextFormat(Qt::RichText);
    ui.help_label->setText(i18nc("The your_api_key part of the URL is a fixed part of the URL "
                                 "and should probably not be changed in localization.",
                                 "You can find your API key <a href=\"http://bit.ly/a/your_api_key\">here</a>"));

    domains << QLatin1String("bit.ly") << QLatin1String("j.mp");
    ui.kcfg_domain->addItems(domains);

    connect(ui.kcfg_login, SIGNAL(textChanged(QString)), SLOT(emitChanged()));
    connect(ui.kcfg_api_key, SIGNAL(textChanged(QString)), SLOT(emitChanged()));
    connect(ui.kcfg_domain, SIGNAL(currentIndexChanged(int)), SLOT(emitChanged()));
    connect(ui.validate_button, SIGNAL(clicked(bool)), SLOT(slotValidate()));
}

Bit_ly_Config::~Bit_ly_Config()
{
}

void Bit_ly_Config::load()
{
    KCModule::load();
    KConfigGroup grp(KSharedConfig::openConfig(), "Bit.ly Shortener");
    ui.kcfg_login->setText(grp.readEntry("login", ""));
    ui.kcfg_domain->setCurrentIndex(domains.indexOf(grp.readEntry("domain", "bit.ly")));
    ui.kcfg_api_key->setText(Choqok::PasswordManager::self()->readPassword(QStringLiteral("bitly_%1")
                             .arg(ui.kcfg_login->text())));
}

void Bit_ly_Config::save()
{
    KCModule::save();
    KConfigGroup grp(KSharedConfig::openConfig(), "Bit.ly Shortener");
    grp.writeEntry("login", ui.kcfg_login->text());
    grp.writeEntry("domain", domains.at(ui.kcfg_domain->currentIndex()));
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("bitly_%1").arg(ui.kcfg_login->text()),
            ui.kcfg_api_key->text());
}

void Bit_ly_Config::emitChanged()
{
    Q_EMIT changed(true);
}

void Bit_ly_Config::slotValidate()
{
    ui.validate_button->setEnabled(false);
    ui.validate_button->setText(i18n("Checking..."));
    QString login = QCoreApplication::applicationName();
    QString apiKey = QLatin1String("R_bdd1ae8b6191dd36e13fc77ca1d4f27f");
    QUrl reqUrl(QLatin1String("http://api.bit.ly/v3/validate"));
    QUrlQuery reqQuery;

    reqQuery.addQueryItem(QLatin1String("x_login"), ui.kcfg_login->text());
    reqQuery.addQueryItem(QLatin1String("x_apiKey"), ui.kcfg_api_key->text());

    if (Bit_ly_Settings::domain() == QLatin1String("j.mp")) {   //bit.ly is default domain
        reqQuery.addQueryItem(QLatin1String("domain"), QLatin1String("j.mp"));
    }

    reqQuery.addQueryItem(QLatin1String("login"), QLatin1String(login.toUtf8()));
    reqQuery.addQueryItem(QLatin1String("apiKey"), QLatin1String(apiKey.toUtf8()));
    reqQuery.addQueryItem(QLatin1String("format"), QLatin1String("txt"));
    reqUrl.setQuery(reqQuery);

    KIO::StoredTransferJob *job = KIO::storedGet(reqUrl, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        QString output(QLatin1String(job->data()));
        if (output.startsWith(QLatin1Char('0')))
            KMessageBox::error(this, i18nc("The your_api_key part of the URL is a fixed part of the URL "
                                           "and should probably not be changed in localization.",
                                           "Provided data is invalid. Try another login or API key.\n"
                                           "You can find it on http://bit.ly/a/your_api_key"));
        if (output.startsWith(QLatin1Char('1'))) {
            KMessageBox::information(this, i18n("You entered valid information."));
        }
    } else {
        Choqok::NotifyManager::error(job->errorString(), i18n("bit.ly Config Error"));
    }

    ui.validate_button->setEnabled(true);
    ui.validate_button->setText(i18n("Validate"));
}

#include "bit_ly_config.moc"
