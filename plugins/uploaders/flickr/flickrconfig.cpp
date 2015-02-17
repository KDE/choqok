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
#include <QTextDocument>
#include <QVBoxLayout>

#include <KAboutData>
#include <KAction>
#include <KActionCollection>
#include <KGenericFactory>
#include <KIO/Job>
#include <KIO/NetAccess>
#include <KLocale>
#include <KMessageBox>
#include <KPluginFactory>
#include <KPushButton>
#include "choqokdebug.h"

#include "choqoktools.h"
#include "passwordmanager.h"

#include "flickrsettings.h"

K_PLUGIN_FACTORY( FlickrConfigFactory, registerPlugin < FlickrConfig > (); )
K_EXPORT_PLUGIN( FlickrConfigFactory( "kcm_choqok_flickr" ) )

const QString apiKey = "13f602e6e705834d8cdd5dd2ccb19651";
const QString apiSecret = "98c89dbe39ae3bea";
QString apiKeSec = apiSecret + QString( "api_key" ) + apiKey;

FlickrConfig::FlickrConfig(QWidget* parent, const QVariantList& ):
        KCModule( FlickrConfigFactory::componentData(), parent)
{
    QVBoxLayout *layout = new QVBoxLayout( this );
    QWidget *wd = new QWidget( this );
    wd->setObjectName("mFlickrCtl");
    ui.setupUi(wd);
    addConfig( FlickrSettings::self(), wd );
    layout->addWidget(wd);

    connect( ui.authButton, SIGNAL( clicked() ), SLOT(slotAuthButton_clicked()) );
    connect( ui.cfg_shorturl, SIGNAL( stateChanged(int)), SLOT(emitChanged()) );
    connect( ui.cfg_forprivate, SIGNAL( clicked(bool)), SLOT(emitChanged()) );
    connect( ui.cfg_forfriends, SIGNAL( stateChanged(int)), SLOT(emitChanged()) );
    connect( ui.cfg_forfamily, SIGNAL( stateChanged(int)), SLOT(emitChanged()) );
    connect( ui.cfg_forpublic, SIGNAL( clicked(bool)), SLOT(emitChanged()) );
    connect( ui.cfg_safe, SIGNAL( clicked(bool)), SLOT(emitChanged()) );
    connect( ui.cfg_moderate, SIGNAL( clicked(bool)), SLOT(emitChanged()) );
    connect( ui.cfg_restricted, SIGNAL( clicked(bool)), SLOT(emitChanged()) );
    connect( ui.cfg_hidefromsearch, SIGNAL( stateChanged(int)), SLOT(emitChanged()) );
}

FlickrConfig::~FlickrConfig()
{
}

void FlickrConfig::load()
{
    qCDebug(CHOQOK);
    KCModule::load();
    KConfigGroup grp( KGlobal::config(), "Flickr Uploader" );
    m_nsid = grp.readEntry( "nsid", "");
    m_username = grp.readEntry( "username", "");
    m_fullname = grp.readEntry( "fullname", "");
    ui.cfg_shorturl->setChecked( grp.readEntry( "shorturl", true ) );
    ui.cfg_forprivate->setChecked( grp.readEntry( "forprivate", false ) );
    ui.cfg_forfriends->setChecked( grp.readEntry( "forfriends", false ));
    ui.cfg_forfamily->setChecked( grp.readEntry( "forfamily", false ));
    ui.cfg_forpublic->setChecked( grp.readEntry( "forpublic", true ) );
    ui.cfg_safe->setChecked( grp.readEntry( "safe", true ) );
    ui.cfg_moderate->setChecked( grp.readEntry( "moderate", false ) );
    ui.cfg_restricted->setChecked( grp.readEntry( "restricted", false ) );
    ui.cfg_hidefromsearch->setChecked( grp.readEntry( "hidefromsearch", false ) );
    m_token = Choqok::PasswordManager::self()->readPassword( QString( "flickr_%1" )
              .arg( m_username ) );
    setAuthenticated(!m_token.isEmpty());
}

void FlickrConfig::save()
{
    qCDebug(CHOQOK);
    KCModule::save();
    KConfigGroup grp( KGlobal::config(), "Flickr Uploader" );
    grp.writeEntry( "nsid", m_nsid );
    grp.writeEntry( "username", m_username );
    grp.writeEntry( "fullname", m_fullname );
    grp.writeEntry( "shorturl", ui.cfg_shorturl->isChecked() );
    grp.writeEntry( "forprivate", ui.cfg_forprivate->isChecked() );
    grp.writeEntry( "forfriends", ui.cfg_forfriends->isChecked() );
    grp.writeEntry( "forfamily", ui.cfg_forfamily->isChecked() );
    grp.writeEntry( "forpublic", ui.cfg_forpublic->isChecked() );
    grp.writeEntry( "safe", ui.cfg_safe->isChecked() );
    grp.writeEntry( "moderate", ui.cfg_moderate->isChecked() );
    grp.writeEntry( "restricted", ui.cfg_restricted->isChecked() );
    grp.writeEntry( "hidefromsearch", ui.cfg_hidefromsearch->isChecked() );
    Choqok::PasswordManager::self()->writePassword( QString( "flickr_%1" ).arg( m_username ),
            m_token );
}

void FlickrConfig::emitChanged()
{
    ui.cfg_forfamily->setEnabled(ui.cfg_forprivate->isChecked());
    ui.cfg_forfriends->setEnabled(ui.cfg_forprivate->isChecked());
    Q_EMIT changed(true);
}

void FlickrConfig::getFrob()
{
    qCDebug(CHOQOK)<<"Get Frob";
    m_frob.clear();
    QUrl url( "http://flickr.com/services/rest/" );
    url.addQueryItem( "method", "flickr.auth.getFrob" );
    url.addQueryItem( "api_key", apiKey.toUtf8() );
    url.addQueryItem( "api_sig",  createSign( "methodflickr.auth.getFrob" ) );

    QString errMsg;
    KIO::Job* job = KIO::get ( url, KIO::Reload, KIO::HideProgressInfo );
    QByteArray data;
    if ( KIO::NetAccess::synchronousRun ( job, 0, &data ) ) {
        QDomDocument rep;
        rep.setContent( data );
        QDomElement element = rep.documentElement();
        if ( element.tagName() == "rsp" ) {
            QString res;
            res = element.attribute( "stat" , "fail" );
            QDomNode node = element.firstChild();
            while ( !node.isNull() ) {
                QDomElement elem = node.toElement();
                if ( res == "ok" ) {
                    if (elem.tagName() == "frob")
                        m_frob = elem.text();
                    return;
                } else if ( res == "fail" ) {
                    if (elem.tagName() == "err") {
                        errMsg = elem.text();
                        int errCode = elem.attribute( "code" , "0" ).toInt();
                        switch (errCode) {
                        case 96:
                        case 97:
                            errMsg = i18n( "Signature problem. Please try again later" );
                            break;
                        case 105:
                            errMsg = i18n( "The requested service is temporarily unavailable. Try again later" );
                            break;
                        default:
                            errMsg = i18n( "Unknown Error: %1. Please try again later" ).arg( errCode );
                            break;
                        }
                    }
                } else {
                    errMsg = i18n( "Malformed response" );
                }
                node = node.nextSibling();
            }
        } else {
            errMsg = i18n( "Malformed response" );
        }
    } else {
        errMsg = job->errorString();
    }
    if (!errMsg.isEmpty())
        KMessageBox::error( ui.gridLayout->widget(), errMsg, i18n("Flickr authorization") );
}

void FlickrConfig::getToken()
{
    qCDebug(CHOQOK)<<"Get Token";
    m_token.clear();
    QUrl url( "http://flickr.com/services/rest/" );
    url.addQueryItem( "method", "flickr.auth.getToken" );
    url.addQueryItem( "api_key", apiKey.toUtf8() );
    url.addQueryItem( "frob", m_frob.toUtf8() );
    url.addQueryItem( "api_sig",  createSign( "frob" + m_frob.toUtf8() + "methodflickr.auth.getToken" ) );

    QString errMsg;
    KIO::Job* job = KIO::get ( url, KIO::Reload, KIO::HideProgressInfo );
    QByteArray data;

    if ( KIO::NetAccess::synchronousRun ( job, 0, &data ) ) {
        QDomDocument rep;
        rep.setContent( data );
        QDomElement element = rep.documentElement();
        if ( element.tagName() == "rsp" ) {
            QString res;
            res = element.attribute( "stat" , "fail" );
            QDomNode node = element.firstChild();
            while ( !node.isNull() ) {
                QDomElement elem = node.toElement();
                if ( res == "ok" ) {
                    QDomNode authNode = node.firstChild();
                    while ( !authNode.isNull()) {
                        QDomElement elem = authNode.toElement();
                        if (elem.tagName() == "token")
                            m_token = elem.text();

                        if (elem.tagName() == "user") {
                            m_nsid = elem.attribute("nsid");
                            m_username = elem.attribute("username");
                            m_fullname = elem.attribute("fullname");
                        }
                        authNode = authNode.nextSibling();
                    }
                } else if ( res == "fail" ) {
                    if (elem.tagName() == "err") {
                        errMsg = elem.text();
                        int errCode = elem.attribute( "code" , "0" ).toInt();
                        switch (errCode) {
                        case 96:
                        case 97:
                        case 108:
                            errMsg = i18n( "Something happens with signature. Please retry" );
                            break;
                        case 105:
                            errMsg = i18n( "The requested service is temporarily unavailable. Try again later" );
                            break;
                        default:
                            errMsg = i18n( "Something happens wrong. Error %1. Try again later" ).arg( errCode );
                            break;
                        }
                    }
                } else {
                    errMsg = i18n( "Malformed response" );
                }
                node = node.nextSibling();
            }
        } else {
            errMsg = i18n( "Malformed response" );
        }
    } else {
        errMsg = job->errorString();
    }
    
    if (!errMsg.isEmpty()) {
        KMessageBox::error( ui.gridLayout->widget(), errMsg, i18n("Flickr authorization") );
        return;
    }

    if ( !m_token.isEmpty() ) {
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
        ui.authButton->setIcon(QIcon::fromTheme("object-unlocked"));
        ui.authLed->on();
        ui.authLabel->setText(i18n("Authorized as %1").arg(m_username));
        if (!m_fullname.isEmpty())
            ui.authLabel->setText(ui.authLabel->text() + QString(" (%1)").arg(Qt::escape(m_fullname)));
    } else {
        ui.authButton->setIcon(QIcon::fromTheme("object-locked"));
        ui.authLed->off();
        ui.authLabel->setText(i18n("Not authorized"));
    }
}

void FlickrConfig::slotAuthButton_clicked()
{
    getFrob();
    if ( !m_frob.isEmpty() ) {
        QString oUrl = "http://flickr.com/services/auth/?";
        oUrl.append( "api_key=" + apiKey );
        oUrl.append( "&perms=write" );
        oUrl.append( "&frob=" + m_frob );
        oUrl.append( "&api_sig=" + createSign( "frob" + m_frob.toUtf8() + "permswrite" ) );
        Choqok::openUrl( oUrl );

        KPushButton *btn = new KPushButton(QIcon::fromTheme("dialog-ok"), i18n("Click here when you authorized Choqok"), this);
        connect(btn, SIGNAL(clicked(bool)), SLOT(getToken()));
        btn->setWindowFlags(Qt::Dialog);
        ui.authTab->layout()->addWidget(btn);
        ui.authButton->setEnabled(false);
    } else {
        return;
    }
}

QByteArray FlickrConfig::createSign( QByteArray req )
{
    return QCryptographicHash::hash( apiKeSec.toUtf8().append( req ),QCryptographicHash::Md5 ).toHex();
}

#include "flickrconfig.moc"
