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

#include "flickrconfig.h"

#include <QCryptographicHash>
#include <QDomDocument>
#include <QDomElement>
#include <QPushButton>
#include <QVBoxLayout>

#include <KAboutData>
#include <KConfigGroup>
#include <KIO/StoredTransferJob>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KSharedConfig>

#include "choqoktools.h"
#include "passwordmanager.h"

#include "flickrsettings.h"

K_PLUGIN_FACTORY_WITH_JSON(FlickrConfigFactory, "choqok_flickr_config.json",
                           registerPlugin < FlickrConfig > ();)

const QString apiKey = QLatin1String("13f602e6e705834d8cdd5dd2ccb19651");
const QString apiSecret = QLatin1String("98c89dbe39ae3bea");
const QString apiKeSec = apiSecret + QLatin1String("api_key") + apiKey;

FlickrConfig::FlickrConfig(QWidget *parent, const QVariantList &)
    : KCModule(KAboutData::pluginData(QLatin1String("kcm_choqok_flickr")), parent)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mFlickrCtl"));
    ui.setupUi(wd);
    addConfig(FlickrSettings::self(), wd);
    layout->addWidget(wd);

    connect(ui.authButton, SIGNAL(clicked()), SLOT(slotAuthButton_clicked()));
    connect(ui.cfg_shorturl, SIGNAL(stateChanged(int)), SLOT(emitChanged()));
    connect(ui.cfg_forprivate, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_forfriends, SIGNAL(stateChanged(int)), SLOT(emitChanged()));
    connect(ui.cfg_forfamily, SIGNAL(stateChanged(int)), SLOT(emitChanged()));
    connect(ui.cfg_forpublic, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_safe, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_moderate, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_restricted, SIGNAL(clicked(bool)), SLOT(emitChanged()));
    connect(ui.cfg_hidefromsearch, SIGNAL(stateChanged(int)), SLOT(emitChanged()));
}

FlickrConfig::~FlickrConfig()
{
}

void FlickrConfig::load()
{
    KCModule::load();
    KConfigGroup grp(KSharedConfig::openConfig(), "Flickr Uploader");
    m_nsid = grp.readEntry("nsid", "");
    m_username = grp.readEntry("username", "");
    m_fullname = grp.readEntry("fullname", "");
    ui.cfg_shorturl->setChecked(grp.readEntry("shorturl", true));
    ui.cfg_forprivate->setChecked(grp.readEntry("forprivate", false));
    ui.cfg_forfriends->setChecked(grp.readEntry("forfriends", false));
    ui.cfg_forfamily->setChecked(grp.readEntry("forfamily", false));
    ui.cfg_forpublic->setChecked(grp.readEntry("forpublic", true));
    ui.cfg_safe->setChecked(grp.readEntry("safe", true));
    ui.cfg_moderate->setChecked(grp.readEntry("moderate", false));
    ui.cfg_restricted->setChecked(grp.readEntry("restricted", false));
    ui.cfg_hidefromsearch->setChecked(grp.readEntry("hidefromsearch", false));
    m_token = Choqok::PasswordManager::self()->readPassword(QStringLiteral("flickr_%1")
              .arg(m_username));
    setAuthenticated(!m_token.isEmpty());
}

void FlickrConfig::save()
{
    KCModule::save();
    KConfigGroup grp(KSharedConfig::openConfig(), "Flickr Uploader");
    grp.writeEntry("nsid", m_nsid);
    grp.writeEntry("username", m_username);
    grp.writeEntry("fullname", m_fullname);
    grp.writeEntry("shorturl", ui.cfg_shorturl->isChecked());
    grp.writeEntry("forprivate", ui.cfg_forprivate->isChecked());
    grp.writeEntry("forfriends", ui.cfg_forfriends->isChecked());
    grp.writeEntry("forfamily", ui.cfg_forfamily->isChecked());
    grp.writeEntry("forpublic", ui.cfg_forpublic->isChecked());
    grp.writeEntry("safe", ui.cfg_safe->isChecked());
    grp.writeEntry("moderate", ui.cfg_moderate->isChecked());
    grp.writeEntry("restricted", ui.cfg_restricted->isChecked());
    grp.writeEntry("hidefromsearch", ui.cfg_hidefromsearch->isChecked());
    Choqok::PasswordManager::self()->writePassword(QStringLiteral("flickr_%1").arg(m_username),
            m_token);
}

void FlickrConfig::emitChanged()
{
    ui.cfg_forfamily->setEnabled(ui.cfg_forprivate->isChecked());
    ui.cfg_forfriends->setEnabled(ui.cfg_forprivate->isChecked());
    Q_EMIT changed(true);
}

void FlickrConfig::getFrob()
{
    m_frob.clear();
    QUrl url(QLatin1String("https://flickr.com/services/rest/"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("method"), QLatin1String("flickr.auth.getFrob"));
    urlQuery.addQueryItem(QLatin1String("api_key"), QLatin1String(apiKey.toUtf8()));
    urlQuery.addQueryItem(QLatin1String("api_sig"),  QLatin1String(createSign("methodflickr.auth.getFrob")));
    url.setQuery(urlQuery);

    QString errMsg;
    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    job->exec();
    if (!job->error()) {
        QDomDocument rep;
        rep.setContent(job->data());
        QDomElement element = rep.documentElement();
        if (element.tagName() == QLatin1String("rsp")) {
            QString res;
            res = element.attribute(QLatin1String("stat") , QLatin1String("fail"));
            QDomNode node = element.firstChild();
            while (!node.isNull()) {
                QDomElement elem = node.toElement();
                if (res == QLatin1String("ok")) {
                    if (elem.tagName() == QLatin1String("frob")) {
                        m_frob = elem.text();
                    }
                    return;
                } else if (res == QLatin1String("fail")) {
                    if (elem.tagName() == QLatin1String("err")) {
                        errMsg = elem.text();
                        int errCode = elem.attribute(QLatin1String("code") , QLatin1String("0")).toInt();
                        switch (errCode) {
                        case 96:
                        case 97:
                            errMsg = i18n("Signature problem. Please try again later");
                            break;
                        case 105:
                            errMsg = i18n("The requested service is temporarily unavailable. Try again later");
                            break;
                        default:
                            errMsg = i18n("Unknown Error:%1. Please try again later").arg(errCode);
                            break;
                        }
                    }
                } else {
                    errMsg = i18n("Malformed response");
                }
                node = node.nextSibling();
            }
        } else {
            errMsg = i18n("Malformed response");
        }
    } else {
        errMsg = job->errorString();
    }
    if (!errMsg.isEmpty()) {
        KMessageBox::error(ui.gridLayout->widget(), errMsg, i18n("Flickr authorization"));
    }
}

void FlickrConfig::getToken()
{
    m_token.clear();
    QUrl url(QLatin1String("https://flickr.com/services/rest/"));
    QUrlQuery urlQuery;
    urlQuery.addQueryItem(QLatin1String("method"), QLatin1String("flickr.auth.getToken"));
    urlQuery.addQueryItem(QLatin1String("api_key"), QLatin1String(apiKey.toUtf8()));
    urlQuery.addQueryItem(QLatin1String("frob"), QLatin1String(m_frob.toUtf8()));
    urlQuery.addQueryItem(QLatin1String("api_sig"),  QLatin1String(createSign("frob" + m_frob.toUtf8() + "methodflickr.auth.getToken")));
    url.setQuery(urlQuery);

    QString errMsg;
    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    job->exec();

    if (!job->error()) {
        QDomDocument rep;
        rep.setContent(job->data());
        QDomElement element = rep.documentElement();
        if (element.tagName() == QLatin1String("rsp")) {
            QString res;
            res = element.attribute(QLatin1String("stat") , QLatin1String("fail"));
            QDomNode node = element.firstChild();
            while (!node.isNull()) {
                QDomElement elem = node.toElement();
                if (res == QLatin1String("ok")) {
                    QDomNode authNode = node.firstChild();
                    while (!authNode.isNull()) {
                        QDomElement elem = authNode.toElement();
                        if (elem.tagName() == QLatin1String("token")) {
                            m_token = elem.text();
                        }

                        if (elem.tagName() == QLatin1String("user")) {
                            m_nsid = elem.attribute(QLatin1String("nsid"));
                            m_username = elem.attribute(QLatin1String("username"));
                            m_fullname = elem.attribute(QLatin1String("fullname"));
                        }
                        authNode = authNode.nextSibling();
                    }
                } else if (res == QLatin1String("fail")) {
                    if (elem.tagName() == QLatin1String("err")) {
                        errMsg = elem.text();
                        int errCode = elem.attribute(QLatin1String("code") , QLatin1String("0")).toInt();
                        switch (errCode) {
                        case 96:
                        case 97:
                        case 108:
                            errMsg = i18n("Something happens with signature. Please retry");
                            break;
                        case 105:
                            errMsg = i18n("The requested service is temporarily unavailable. Try again later");
                            break;
                        default:
                            errMsg = i18n("Something happens wrong. Error %1. Try again later").arg(errCode);
                            break;
                        }
                    }
                } else {
                    errMsg = i18n("Malformed response");
                }
                node = node.nextSibling();
            }
        } else {
            errMsg = i18n("Malformed response");
        }
    } else {
        errMsg = job->errorString();
    }

    if (!errMsg.isEmpty()) {
        KMessageBox::error(ui.gridLayout->widget(), errMsg, i18n("Flickr authorization"));
        return;
    }

    if (!m_token.isEmpty()) {
        setAuthenticated(true);
        FlickrConfig::save();
        ui.tabWidget->setCurrentIndex(1);
    } else {
        setAuthenticated(false);
    }
    ui.authButton->setEnabled(true);
}

void FlickrConfig::setAuthenticated(bool authenticated)
{
    isAuthenticated = authenticated;
    if (authenticated) {
        ui.authButton->setIcon(QIcon::fromTheme(QLatin1String("object-unlocked")));
        ui.authLed->on();
        ui.authLabel->setText(i18n("Authorized as %1").arg(m_username));
        if (!m_fullname.isEmpty()) {
            ui.authLabel->setText(ui.authLabel->text() + QStringLiteral(" (%1)").arg(m_fullname.toHtmlEscaped()));
        }
    } else {
        ui.authButton->setIcon(QIcon::fromTheme(QLatin1String("object-locked")));
        ui.authLed->off();
        ui.authLabel->setText(i18n("Not authorized"));
    }
}

void FlickrConfig::slotAuthButton_clicked()
{
    getFrob();
    if (!m_frob.isEmpty()) {
        QUrl oUrl(QLatin1String("https://flickr.com/services/auth/?"));
        oUrl.setPath(oUrl.path() + QLatin1String("api_key=") + apiKey +
                     QLatin1String("&perms=write&frob=") + m_frob +
                     QLatin1String("&api_sig=") + QLatin1String(createSign("frob" + m_frob.toUtf8() + "permswrite")));
        Choqok::openUrl(oUrl);

        QPushButton *btn = new QPushButton(QIcon::fromTheme(QLatin1String("dialog-ok")), i18n("Click here when you authorized Choqok"), this);
        connect(btn, SIGNAL(clicked(bool)), SLOT(getToken()));
        btn->setWindowFlags(Qt::Dialog);
        ui.authTab->layout()->addWidget(btn);
        ui.authButton->setEnabled(false);
    } else {
        return;
    }
}

QByteArray FlickrConfig::createSign(QByteArray req)
{
    return QCryptographicHash::hash(apiKeSec.toUtf8().append(req), QCryptographicHash::Md5).toHex();
}

#include "flickrconfig.moc"
