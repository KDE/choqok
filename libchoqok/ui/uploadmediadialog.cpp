/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "uploadmediadialog.h"
#include "ui_uploadmedia_base.h"

#include <QComboBox>
#include <QPointer>
#include <QProgressBar>
#include <QTabWidget>
#include <KCMultiDialog>

#include <KAboutData>
#include <KAboutApplicationDialog>
#include <KCModuleProxy>
#include <KLocalizedString>
#include <KMessageBox>

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "libchoqokdebug.h"
#include "pluginmanager.h"
#include "mediamanager.h"
#include "quickpost.h"
#include "uploader.h"

using namespace Choqok::UI;

class UploadMediaDialog::Private
{
public:
    Ui::UploadMediaBase ui;
    QMap <QString, KPluginMetaData> availablePlugins;
    QList<KCModuleProxy *> moduleProxyList;
    QUrl localUrl;
    QPointer<QProgressBar> progress;
};

UploadMediaDialog::UploadMediaDialog(QWidget *parent, const QString &url)
    : QDialog(parent), d(new Private)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowTitle(i18n("Upload Medium"));

    d->ui.setupUi(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18n("Upload"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &UploadMediaDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &UploadMediaDialog::reject);
    d->ui.verticalLayout->addWidget(buttonBox);

    adjustSize();

    connect(d->ui.imageUrl, &KUrlRequester::textChanged,
            this, &UploadMediaDialog::slotMediumChanged);
    load();
    if (url.isEmpty()) {
        d->ui.imageUrl->button()->click();
    } else {
        d->ui.imageUrl->setUrl(QUrl(url));
    }
    connect(d->ui.uploaderPlugin, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &UploadMediaDialog::currentPluginChanged);
    d->ui.aboutPlugin->setIcon(QIcon::fromTheme(QLatin1String("help-about")));
    d->ui.configPlugin->setIcon(QIcon::fromTheme(QLatin1String("configure")));
    connect(d->ui.aboutPlugin, &QPushButton::clicked, this, &UploadMediaDialog::slotAboutClicked);
    connect(d->ui.configPlugin, &QPushButton::clicked, this, &UploadMediaDialog::slotConfigureClicked);
    connect(Choqok::MediaManager::self(), &MediaManager::mediumUploaded,
            this, &UploadMediaDialog::slotMediumUploaded);
    connect(Choqok::MediaManager::self(), &MediaManager::mediumUploadFailed,
            this, &UploadMediaDialog::slotMediumUploadFailed);
}

UploadMediaDialog::~UploadMediaDialog()
{
    delete d;
}

void UploadMediaDialog::load()
{
    QVector<KPluginMetaData> plugins = Choqok::PluginManager::self()->availablePlugins(QLatin1String("Uploaders"));
    qCDebug(CHOQOK) << plugins.count();

    for (const KPluginMetaData &plugin: plugins) {
        d->ui.uploaderPlugin->addItem(QIcon::fromTheme(plugin.iconName()), plugin.name(), plugin.name());
        d->availablePlugins.insert(plugin.pluginId(), plugin);
    }
    d->ui.uploaderPlugin->setCurrentIndex(d->ui.uploaderPlugin->findData(Choqok::BehaviorSettings::lastUsedUploaderPlugin()));
    if (d->ui.uploaderPlugin->currentIndex() == -1 && d->ui.uploaderPlugin->count() > 0) {
        d->ui.uploaderPlugin->setCurrentIndex(0);
    }
}

void UploadMediaDialog::accept()
{
    if (d->ui.uploaderPlugin->currentIndex() == -1 ||
            !QFile::exists(d->ui.imageUrl->url().toLocalFile()) ||
            !QFile(d->ui.imageUrl->url().toLocalFile()).size()) {
        return;
    }
    if (d->progress) {
        d->progress->deleteLater();
    }
    d->progress = new QProgressBar(this);
    d->progress->setRange(0, 0);
    d->progress->setFormat(i18n("Uploading..."));
    d->ui.verticalLayout->addWidget(d->progress);
    Choqok::BehaviorSettings::setLastUsedUploaderPlugin(d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString());
    d->localUrl = d->ui.imageUrl->url();
    QString plugin = d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString();
    showed = true;
    winSize = size();
    Choqok::MediaManager::self()->uploadMedium(d->localUrl, plugin);
}

void Choqok::UI::UploadMediaDialog::currentPluginChanged(int index)
{
    QString key = d->ui.uploaderPlugin->itemData(index).toString();
    d->ui.configPlugin->setEnabled(!key.isEmpty() && d->availablePlugins.value(key).value(QStringLiteral("X-KDE-ConfigModule"), QString()) != QString());
}

void Choqok::UI::UploadMediaDialog::slotAboutClicked()
{
    const QString shorten = d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString();
    if (shorten.isEmpty()) {
        return;
    }
    auto info = d->availablePlugins.value(shorten);

    KAboutData aboutData(info.name(), info.name(), info.version(), info.description(),
                         KAboutLicense::byKeyword(info.license()).key(), QString(),
                         QString(), info.website());
    aboutData.addAuthor(info.authors()[0].name(), info.authors()[0].task(), info.authors()[0].emailAddress());

    KAboutApplicationDialog aboutPlugin(aboutData, this);
    aboutPlugin.setWindowIcon(QIcon::fromTheme(info.iconName()));
    aboutPlugin.exec();
}

void Choqok::UI::UploadMediaDialog::slotConfigureClicked()
{
    auto dialog = new KCMultiDialog(this);
    QString id = d->availablePlugins.value(d->ui.uploaderPlugin->itemData(d->ui.uploaderPlugin->currentIndex()).toString()).value(QStringLiteral("X-KDE-ConfigModule"), QString());
    KPluginMetaData md(id);
    dialog->addModule(md);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->show();
}

void Choqok::UI::UploadMediaDialog::slotMediumUploaded(const QUrl &localUrl, const QString &remoteUrl)
{
    if (d->localUrl == localUrl && showed) {
        qCDebug(CHOQOK);
        Global::quickPostWidget()->appendText(remoteUrl);
        showed = false;
        close();
    }
}

void Choqok::UI::UploadMediaDialog::slotMediumUploadFailed(const QUrl &localUrl, const QString &errorMessage)
{
    if (d->localUrl == localUrl && showed) {
        showed = false;
        KMessageBox::detailedError(Global::mainWindow(),
                                   i18n("Medium uploading failed."),
                                   errorMessage);
        show();
        d->progress->deleteLater();
    }
    resize(winSize);
}

void Choqok::UI::UploadMediaDialog::slotMediumChanged(const QString &url)
{
    d->ui.previewer->showPreview(QUrl::fromLocalFile(url));
}

