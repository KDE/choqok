/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "uploadmediadialog.h"
#include "ui_uploadmedia_base.h"

#include <QPointer>
#include <QProgressBar>

#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KCModuleInfo>
#include <KCModuleProxy>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginInfo>
#include <KTabWidget>
#include "libchoqokdebug.h"

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "pluginmanager.h"
#include "mediamanager.h"
#include "quickpost.h"
#include "uploader.h"

using namespace Choqok::UI;

class UploadMediaDialog::Private
{
public:
    Ui::UploadMediaBase ui;
    QMap <QString, KPluginInfo> availablePlugins;
    QList<KCModuleProxy*> moduleProxyList;
    QUrl localUrl;
    QPointer<QProgressBar> progress;
};

UploadMediaDialog::UploadMediaDialog(QWidget* parent, const QString& url)
    : KDialog(parent), d(new Private)
{
    QWidget *wd = new QWidget(parent);
    d->ui.setupUi(wd);
    setMainWidget(wd);
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Upload Medium"));
    resize(400,300);
    setButtonText(Ok, i18n("Upload"));
    connect(d->ui.imageUrl, SIGNAL(textChanged(QString)),
            this, SLOT(slotMediumChanged(QString)));
    load();
    if (url.isEmpty())
      d->ui.imageUrl->button()->click();
    else
      d->ui.imageUrl->setUrl(QUrl(url));
    connect(d->ui.uploaderPlugin, SIGNAL(currentIndexChanged(int)), SLOT(currentPluginChanged(int)));
    d->ui.aboutPlugin->setIcon(QIcon::fromTheme("help-about"));
    d->ui.configPlugin->setIcon(QIcon::fromTheme("configure"));
    connect( d->ui.aboutPlugin, SIGNAL(clicked(bool)), SLOT(slotAboutClicked()) );
    connect( d->ui.configPlugin, SIGNAL(clicked(bool)), SLOT(slotConfigureClicked()) );
    connect(Choqok::MediaManager::self(), SIGNAL(mediumUploaded(QUrl,QString)),
            SLOT(slotMediumUploaded(QUrl,QString)));
    connect(Choqok::MediaManager::self(), SIGNAL(mediumUploadFailed(QUrl,QString)),
            SLOT(slotMediumUploadFailed(QUrl,QString)));

}

UploadMediaDialog::~UploadMediaDialog()
{
    delete d;
}

void UploadMediaDialog::load()
{
    QList<KPluginInfo> plugins = Choqok::PluginManager::self()->availablePlugins("Uploaders");
    qCDebug(CHOQOK)<<plugins.count();

    Q_FOREACH (const KPluginInfo& plugin, plugins) {
        d->ui.uploaderPlugin->addItem( QIcon::fromTheme(plugin.icon()), plugin.name(), plugin.pluginName());
        d->availablePlugins.insert(plugin.pluginName(), plugin);
    }
    d->ui.uploaderPlugin->setCurrentIndex( d->ui.uploaderPlugin->findData( Choqok::BehaviorSettings::lastUsedUploaderPlugin() ) );
    if(d->ui.uploaderPlugin->currentIndex()==-1 && d->ui.uploaderPlugin->count()>0)
        d->ui.uploaderPlugin->setCurrentIndex(0);
}

void UploadMediaDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Ok){
        if(d->ui.uploaderPlugin->currentIndex()==-1 ||
          !QFile::exists(d->ui.imageUrl->url().toLocalFile()) ||
          !QFile(d->ui.imageUrl->url().toLocalFile()).size())
            return;
        if(d->progress)
            d->progress->deleteLater();
        d->progress = new QProgressBar(this);
        d->progress->setRange(0, 0);
        d->progress->setFormat(i18n("Uploading..."));
        mainWidget()->layout()->addWidget(d->progress);
        Choqok::BehaviorSettings::setLastUsedUploaderPlugin(d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString());
        d->localUrl = d->ui.imageUrl->url();
        QString plugin = d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString();
        showed = true;
        winSize = size();
        Choqok::MediaManager::self()->uploadMedium(d->localUrl, plugin);
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void Choqok::UI::UploadMediaDialog::currentPluginChanged(int index)
{
    QString key = d->ui.uploaderPlugin->itemData(index).toString();
//     qCDebug(CHOQOK)<<key;
    d->ui.configPlugin->setEnabled(!key.isEmpty() && d->availablePlugins.value(key).kcmServices().count() > 0);
}

void Choqok::UI::UploadMediaDialog::slotAboutClicked()
{
    const QString shorten = d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString();
    if(shorten.isEmpty())
        return;
    KPluginInfo info = d->availablePlugins.value(shorten);

    KAboutData aboutData(info.name(), info.name(), info.version(), info.comment(),
                         KAboutLicense::byKeyword(info.license()).key(), QString(),
                         QString(), info.website());
    aboutData.setProgramIconName(info.icon());
    aboutData.addAuthor(info.author(), QString(), info.email());

    KAboutApplicationDialog aboutPlugin(aboutData, this);
    aboutPlugin.exec();
}

void Choqok::UI::UploadMediaDialog::slotConfigureClicked()
{
        qCDebug(CHOQOK);
    KPluginInfo pluginInfo = d->availablePlugins.value( d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex() ).toString() );
    qCDebug(CHOQOK)<<pluginInfo.name()<<pluginInfo.kcmServices().count();

    QPointer<KDialog> configDialog = new KDialog(this);
    configDialog->setWindowTitle(pluginInfo.name());
    // The number of KCModuleProxies in use determines whether to use a tabwidget
    KTabWidget *newTabWidget = 0;
    // Widget to use for the setting dialog's main widget,
    // either a KTabWidget or a KCModuleProxy
    QWidget * mainWidget = 0;
    // Widget to use as the KCModuleProxy's parent.
    // The first proxy is owned by the dialog itself
    QWidget *moduleProxyParentWidget = configDialog;

    Q_FOREACH (const KService::Ptr &servicePtr, pluginInfo.kcmServices()) {
        if(!servicePtr->noDisplay()) {
            KCModuleInfo moduleInfo(servicePtr);
            KCModuleProxy *currentModuleProxy = new KCModuleProxy(moduleInfo, moduleProxyParentWidget);
            if (currentModuleProxy->realModule()) {
                d->moduleProxyList << currentModuleProxy;
                if (mainWidget && !newTabWidget) {
                    // we already created one KCModuleProxy, so we need a tab widget.
                    // Move the first proxy into the tab widget and ensure this and subsequent
                    // proxies are in the tab widget
                    newTabWidget = new KTabWidget(configDialog);
                    moduleProxyParentWidget = newTabWidget;
                    mainWidget->setParent( newTabWidget );
                    KCModuleProxy *moduleProxy = qobject_cast<KCModuleProxy*>(mainWidget);
                    if (moduleProxy) {
                        newTabWidget->addTab(mainWidget, moduleProxy->moduleInfo().moduleName());
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
    if (d->moduleProxyList.count()) {
        configDialog->setButtons(KDialog::Ok | KDialog::Cancel);

        QWidget *showWidget = new QWidget(configDialog);
        QVBoxLayout *layout = new QVBoxLayout;
        showWidget->setLayout(layout);
        layout->addWidget(mainWidget);
        layout->insertSpacing(-1, KDialog::marginHint());
        configDialog->setMainWidget(showWidget);

//         connect(&configDialog, SIGNAL(defaultClicked()), this, SLOT(slotDefaultClicked()));

        if (configDialog->exec() == QDialog::Accepted) {
            Q_FOREACH (KCModuleProxy *moduleProxy, d->moduleProxyList) {
                QStringList parentComponents = moduleProxy->moduleInfo().service()->property("X-KDE-ParentComponents").toStringList();
                moduleProxy->save();
//                 foreach (const QString &parentComponent, parentComponents) {
//                     emit configCommitted(parentComponent.toLatin1());
//                 }
            }
        } else {
            Q_FOREACH (KCModuleProxy *moduleProxy, d->moduleProxyList) {
                moduleProxy->load();
            }
        }

        qDeleteAll(d->moduleProxyList);
        d->moduleProxyList.clear();
    }
}

void Choqok::UI::UploadMediaDialog::slotMediumUploaded(const QUrl &localUrl, const QString& remoteUrl)
{
    if(d->localUrl == localUrl && showed){
        qCDebug(CHOQOK);
        Global::quickPostWidget()->appendText(remoteUrl);
        showed = false;
        close();
    }
}

void Choqok::UI::UploadMediaDialog::slotMediumUploadFailed(const QUrl &localUrl, const QString& errorMessage)
{
    if(d->localUrl == localUrl && showed){
        showed = false;
        KMessageBox::detailedSorry(Global::mainWindow(), i18n("Medium uploading failed."), errorMessage );
        show();
        d->progress->deleteLater();
    }
    resize(winSize);
}

void Choqok::UI::UploadMediaDialog::slotMediumChanged(const QString& url)
{
    d->ui.previewer->showPreview(QUrl(url));
}

