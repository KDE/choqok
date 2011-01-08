/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "translator.h"
#include <KGenericFactory>
#include <choqokuiglobal.h>
#include "postwidget.h"
#include <kio/jobclasses.h>
#include <KIO/Job>
#include <shortenmanager.h>
#include "translatorsettings.h"
#include <KAction>
#include <QMenu>
#include <kstandarddirs.h>
#include <QFile>
#include <qjson/parser.h>
#include <choqokappearancesettings.h>

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Translator > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_translator" ) )

Translator::Translator(QObject* parent, const QList< QVariant >& )
    :Choqok::Plugin(MyPluginFactory::componentData(), parent)
{
    kDebug();
    translateAction = new KAction(i18n("Translate to ..."), this);
    Choqok::UI::PostWidget::addAction(translateAction);
    translateAction->setMenu(setupTranslateMenu());
}

Translator::~Translator()
{

}

void Translator::translate()
{
    QString lang = qobject_cast<KAction*>(sender())->data().toString();
    Choqok::UI::PostWidget *wd;
    wd = dynamic_cast<Choqok::UI::PostWidgetUserData *>(translateAction->userData(32))->postWidget();
    if(!wd || lang.isEmpty())
        return;

    KUrl url ( "https://www.googleapis.com/language/translate/v2");
    url.addQueryItem("key", "AIzaSyBqB4DN7CRIvMl4NKmffC-QlFilGVVRsmI");
    url.addQueryItem("q", wd->content());
    url.addQueryItem("format", "html");
    url.addQueryItem("target", lang);

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::NoReload, KIO::HideProgressInfo);
    mJobPostWidget.insert(job, wd);
    connect(job, SIGNAL(result(KJob*)), SLOT(slotTranslated(KJob*)));
    job->start();
}

void Translator::slotTranslated(KJob* job)
{
    Choqok::UI::PostWidget* wd = mJobPostWidget.take(job);
    KIO::StoredTransferJob* stj = qobject_cast<KIO::StoredTransferJob*>(job);
    if(!wd || !stj)
        return;
    QByteArray data = stj->data();
    kDebug()<<data;
    QVariantMap json = QJson::Parser().parse(data).toMap();
    QString errorMessage;
    if(job->error() == KJob::NoError){
        if( json.contains("data") ) {
            QVariantMap trMap = json["data"].toMap()["translations"].toList()[0].toMap();
            QString srcLang = KGlobal::locale()->languageCodeToName(trMap["detectedSourceLanguage"].toString());
            QString color;
            if( Choqok::AppearanceSettings::isCustomUi() ) {
                color = Choqok::AppearanceSettings::readForeColor().lighter().name();
            } else {
                color = wd->palette().dark().color().name();
            }
            QString translatedNotice = i18n("<span style=\"color:%2; font-size:small;\">Translated from %1:</span>", srcLang, color);
            wd->setContent(QString("%1<br/>%2").arg(translatedNotice)
                                            .arg(trMap["translatedText"].toString()));
            return;
        } else if(json.contains("error")){
            QVariantMap trMap = json["error"].toMap()["errors"].toList()[0].toMap();
            errorMessage = trMap["message"].toString();
        }
    } else {
        errorMessage = job->errorString();
    }
    Choqok::UI::Global::mainWindow()->showStatusMessage(i18n("Translation failed: %1", errorMessage));
}

QMenu* Translator::setupTranslateMenu()
{
    QMenu *menu = new QMenu;
    if(TranslatorSettings::languages().isEmpty()){
        langs.clear();
        QFile file(KStandardDirs::locate("data", "languagecodes"));
        if(file.exists()) {
            file.open(QFile::ReadOnly);
            while (!file.atEnd()) {
                langs << QLatin1String(file.readLine().trimmed());
            }
        }
    } else {
        langs =  TranslatorSettings::languages();
    }
//     kDebug()<<langs;
    foreach(const QString& lang, langs){
        QString flag = KStandardDirs::locate( "locale", QString( "l10n/%1/flag.png" ).arg( lang.toLower() ) );
        KIcon icon;
        if ( QFile::exists( flag ) )
            icon.addPixmap(QPixmap( flag ));
        QString langStr = KGlobal::locale()->languageCodeToName(lang);
        KAction* act = new KAction(icon, langStr.isEmpty() ? lang : langStr, 0);
        act->setData(lang);
        connect( act, SIGNAL(triggered(bool)), SLOT(translate()));
        menu->addAction(act);
    }
    return menu;
}

#include "translator.moc"
