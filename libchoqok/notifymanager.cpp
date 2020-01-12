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
    QString fullMsg = QStringLiteral("<b>%1:</b><br/>%2").arg(title).arg(message);
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
