/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2010-2011 Emanuele Bigiarini <pulmro@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "dbushandler.h"

#include <QDBusConnection>
#include <QPointer>
#include <QTextDocument>

#include <KIO/StoredTransferJob>

#include "ChoqokAdaptor.h"
#include "choqokbehaviorsettings.h"
#include "libchoqokdebug.h"
#include "quickpost.h"
#include "shortenmanager.h"
#include "uploadmediadialog.h"

namespace Choqok
{

DbusHandler *DbusHandler::m_self = nullptr;

DbusHandler::DbusHandler()
{
    m_self = this;
    new ChoqokAdaptor(this);
    QDBusConnection::sessionBus().registerService(QLatin1String("org.kde.choqok"));
    QDBusConnection::sessionBus().registerObject(QLatin1String("/"), this);
}

DbusHandler::~DbusHandler()
{

}

QString DbusHandler::prepareUrl(const QString &url)
{
    if (Choqok::BehaviorSettings::shortenOnPaste() && url.count() > 30) {
        return ShortenManager::self()->shortenUrl(url);
    } else {
        return url;
    }
}

void DbusHandler::shareUrl(const QString &url, bool title)
{
    if (title) {
        QByteArray data;
        KIO::StoredTransferJob *job = KIO::storedGet(QUrl(url), KIO::NoReload, KIO::HideProgressInfo) ;
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
        } else {
            connect(job, &KIO::StoredTransferJob::result, this, &DbusHandler::slotTitleUrl);
            job->start();
        }
    }
    postText(prepareUrl(url));
}

void DbusHandler::slotTitleUrl(KJob *job)
{
    QString text;
    if (!job) {
        qCWarning(CHOQOK) << "NULL Job returned";
        return;
    }
    KIO::StoredTransferJob *stj = qobject_cast<KIO::StoredTransferJob *> (job);
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        QByteArray data = stj->data();
        QTextCodec *codec = QTextCodec::codecForHtml(data);
        m_doc.setHtml(codec->toUnicode(data));
        text.append(m_doc.metaInformation(QTextDocument::DocumentTitle));
    }
    QString url = stj->url().toDisplayString();
    text.append(QLatin1Char(' ') + prepareUrl(url));
    postText(text);
}

void DbusHandler::uploadFile(const QString &filename)
{
    QPointer<Choqok::UI::UploadMediaDialog> dlg = new Choqok::UI::UploadMediaDialog(nullptr, filename);
    dlg->show();
}

void DbusHandler::postText(const QString &text)
{
    // Before posting text ensure QuickPost widget has been created otherwise wait for it.
    // This is necessary when choqok is launched by a D-Bus call, because it can happen
    //  that DBusHandler is ready, but QuickPost widget not yet.
    if (Choqok::UI::Global::quickPostWidget() == nullptr) {
        m_textToPost = QString(text);
        connect(Choqok::UI::Global::mainWindow(), &UI::MainWindow::quickPostCreated,
                this, &DbusHandler::slotcreatedQuickPost);
        return;
    }
    if (Choqok::UI::Global::quickPostWidget()->isVisible()) {
        Choqok::UI::Global::quickPostWidget()->appendText(text);
    } else {
        Choqok::UI::Global::quickPostWidget()->setText(text);
    }
}

void DbusHandler::slotcreatedQuickPost()
{
    if (Choqok::UI::Global::quickPostWidget()->isVisible()) {
        Choqok::UI::Global::quickPostWidget()->appendText(m_textToPost);
    } else {
        Choqok::UI::Global::quickPostWidget()->setText(m_textToPost);
    }
}

void DbusHandler::updateTimelines()
{
    Choqok::UI::Global::mainWindow()->action("update_timeline")->trigger();
}

void DbusHandler::setShortening(bool flag)
{
    Choqok::BehaviorSettings::setShortenOnPaste(flag);
}

bool DbusHandler::getShortening()
{
    return Choqok::BehaviorSettings::shortenOnPaste();
}

DbusHandler *ChoqokDbus()
{
    if (DbusHandler::m_self == nullptr) {
        DbusHandler::m_self = new DbusHandler();
    }
    return DbusHandler::m_self;
}

}

#include "moc_dbushandler.cpp"
