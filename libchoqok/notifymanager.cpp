/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "notifymanager.h"

#include <QTimer>

#include <KNotification>

#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"

namespace Choqok
{

class NotifyManagerPrivate
{
public:
    NotifyManagerPrivate()
    {
        lastErrorClearance.setSingleShot(true);
        lastErrorClearance.setInterval(3000);
        QObject::connect(&lastErrorClearance, &QTimer::timeout,
                         Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::resetNotifyManager);
    }
    void triggerNotify(const QString &eventId, const QString &title,
                       const QString &message,
                       KNotification::NotificationFlags flags = KNotification::CloseOnTimeout);

    QList<QString> lastErrorMessages;
    QTimer          lastErrorClearance;
};

Q_GLOBAL_STATIC(NotifyManagerPrivate, _nmp)

NotifyManager::NotifyManager()
{
}

NotifyManager::~NotifyManager()
{
}

void NotifyManager::resetNotifyManager()
{
    _nmp->lastErrorMessages.clear();
}

void NotifyManager::success(const QString &message, const QString &title)
{
    if (Choqok::UI::Global::mainWindow()->isActiveWindow()) {
        choqokMainWindow->showStatusMessage(message);
    } else {
        _nmp->triggerNotify(QLatin1String("job-success"), title, message);
    }
}

void NotifyManager::error(const QString &message, const QString &title)
{
    if (!_nmp->lastErrorMessages.contains(message)) {
        _nmp->triggerNotify(QLatin1String("job-error"), title, message);
        _nmp->lastErrorMessages.append(message);
        _nmp->lastErrorClearance.start();
    }

}

void NotifyManager::newPostArrived(const QString &message, const QString &title)
{
    QString fullMsg = QStringLiteral("<b>%1:</b>\n%2").arg(title).arg(message);
    if (Choqok::UI::Global::mainWindow()->isActiveWindow()) {
        choqokMainWindow->showStatusMessage(message);
    } else {
        if (Choqok::BehaviorSettings::knotify()) {
            KNotification *n = new KNotification(QLatin1String("new-post-arrived"));
            n->setActions(QStringList(i18nc("Show Choqok MainWindow", "Show Choqok")));
            n->setText(fullMsg);
            QObject::connect(n, (void (KNotification::*)())&KNotification::activated,
                             choqokMainWindow, &Choqok::UI::MainWindow::activateChoqok);
            n->sendEvent();
        }
    }
}

void NotifyManager::shortening(const QString &message, const QString &title)
{
    _nmp->triggerNotify(QLatin1String("shortening"), title, message);
}

void NotifyManagerPrivate::triggerNotify(const QString &eventId, const QString &title,
        const QString &message, KNotification::NotificationFlags flags)
{
    QString fullMsg = QStringLiteral("<b>%1:</b><br/>%2").arg(title).arg(message);
    KNotification::event(eventId, fullMsg, QPixmap(), Choqok::UI::Global::mainWindow(), flags);
}

}
