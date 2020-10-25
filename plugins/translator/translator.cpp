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

#include "translator.h"
#include <KGenericFactory>
#include <choqokuiglobal.h>
#include "postwidget.h"
#include <kio/jobclasses.h>
#include <KIO/Job>
#include <shortenmanager.h>
#include "translatorsettings.h"
#include <QAction>
#include <QMenu>
#include <kstandarddirs.h>
#include <QFile>
#include <qjson/parser.h>
#include <choqokappearancesettings.h>
#include <KPluginInfo>
#include <KCModuleProxy>
#include <QVBoxLayout>
#include <KDialog>
#include <KTabWidget>
#include "sharedtools.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Translator > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_translator" ) )

Translator::Translator(QObject* parent, const QList< QVariant >& )
    :Choqok::Plugin(MyPluginFactory::componentData(), parent)
{
    translateAction = new QAction(i18n("Translate to..."), this);
    Choqok::UI::PostWidget::addAction(translateAction);
    translateAction->setMenu(setupTranslateMenu());
    connect(TranslatorSettings::self(), SIGNAL(configChanged()), SLOT(slotUpdateMenu()));
}

Translator::~Translator()
{

}

void Translator::translate()
{
    QString lang = qobject_cast<QAction *>(sender())->data().toString();
    Choqok::UI::PostWidget *wd;
    wd = dynamic_cast<Choqok::UI::PostWidgetUserData *>(translateAction->userData(32))->postWidget();
    if(!wd || lang.isEmpty())
        return;

    QUrl url ( "https://www.googleapis.com/language/translate/v2");
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
//     qDebug()<<data;
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
            QString translatedNotice =
            i18nc("%1 is the name of the source language in localized form", "<span style=\"color:%2; font-size:small;\">Translated from %1: (<a href='choqok://showoriginalpost' style=\"text-decoration:none\" >original post</a>)</span>", srcLang, color);
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
    TranslatorSettings::self()->readConfig();
    langs =  TranslatorSettings::languages();
    for (const QString& lang, langs) {
        KIcon icon;
		icon.addPixmap(QPixmap(SharedTools::self()->languageFlag(lang)));
        QString langStr = KGlobal::locale()->languageCodeToName(lang);
        QAction * act =
        new QAction(icon, langStr.isEmpty() ? SharedTools::self()->missingLangs().value(lang) : langStr, 0);
        act->setData(lang);
        connect( act, SIGNAL(triggered(bool)), SLOT(translate()));
        menu->addAction(act);
    }
    menu->addSeparator();
    QAction * setup = new QAction(QIcon::fromTheme("configure"), i18n("Configure Translator"), menu);
    connect(setup, SIGNAL(triggered(bool)), this, SLOT(slotConfigureTranslator()));
    menu->addAction(setup);
    return menu;
}

void Translator::slotUpdateMenu()
{
    qDeleteAll(translateAction->menu()->actions());
    translateAction->menu()->clear();
    translateAction->setMenu(setupTranslateMenu());
}

void Translator::slotConfigureTranslator()
{
    KPluginInfo pluginInfo = this->pluginInfo();
    QPointer<KDialog> configDialog = new KDialog(Choqok::UI::Global::mainWindow());
    configDialog->setWindowTitle(pluginInfo.name());
    // The number of KCModuleProxies in use determines whether to use a tabwidget
    KTabWidget *newTabWidget = 0;
    // Widget to use for the setting dialog's main widget,
    // either a KTabWidget or a KCModuleProxy
    QWidget * mainWidget = 0;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = configDialog;

    QList<KCModuleProxy*> moduleProxyList;

    for (const KService::Ptr &servicePtr, pluginInfo.kcmServices()) {
        if(!servicePtr->noDisplay()) {
            KCModuleProxy *currentModuleProxy = new KCModuleProxy(servicePtr, moduleProxyParentWidget);
            if (currentModuleProxy->realModule()) {
                moduleProxyList << currentModuleProxy;
                if (mainWidget && !newTabWidget) {
                    // we already created one KCModuleProxy, so we need a tab widget.
                    // Move the first proxy into the tab widget and ensure this and subsequent
                    // proxies are in the tab widget
                    newTabWidget = new KTabWidget(configDialog);
                    moduleProxyParentWidget = newTabWidget;
                    mainWidget->setParent( newTabWidget );
                    KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy*>(mainWidget);
                    if (moduleProxy) {
                        newTabWidget->addTab(mainWidget, servicePtr->name());
                        mainWidget = newTabWidget;
                    } else {
                        delete newTabWidget;
                        newTabWidget = 0;
                        moduleProxyParentWidget = configDialog;
                        mainWidget->setParent(0);
                    }
                }

                if (newTabWidget) {
                    newTabWidget->addTab(currentModuleProxy, servicePtr->name());
                } else {
                    mainWidget = currentModuleProxy;
                }
            } else {
                delete currentModuleProxy;
            }
        }
    }
        // it could happen that we had services to show, but none of them were real modules.
    if (moduleProxyList.count()) {
        configDialog->setButtons(KDialog::Ok | KDialog::Cancel);

        QWidget *showWidget = new QWidget(configDialog);
        QVBoxLayout *layout = new QVBoxLayout;
        showWidget->setLayout(layout);
        layout->addWidget(mainWidget);
        layout->insertSpacing(-1, KDialog::marginHint());
        configDialog->setMainWidget(showWidget);

        if (configDialog->exec() == QDialog::Accepted) {
            for (KCModuleProxy *moduleProxy, moduleProxyList) {
                QStringList parentComponents = moduleProxy->moduleInfo().service()->property("X-KDE-ParentComponents").toStringList();
                moduleProxy->save();
                slotUpdateMenu();
            }
        } else {
            for (KCModuleProxy *moduleProxy, moduleProxyList) {
                moduleProxy->load();
            }
        }

        qDeleteAll(moduleProxyList);
        moduleProxyList.clear();
    }
}


#include "translator.moc"
