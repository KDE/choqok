/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <QApplication>
#include "choqokuiglobal.h"
#include <kglobal.h>
#include <choqokbehaviorsettings.h>

namespace Choqok
{

class NotifyManagerPrivate
{
public:
    void triggerNotify( const QString &eventId, const QString &title,
                        const QString &message,
                        KNotification::NotificationFlags flags = KNotification::CloseOnTimeout );
};

K_GLOBAL_STATIC(NotifyManagerPrivate, _nmp)

NotifyManager::NotifyManager()
{
}

NotifyManager::~NotifyManager()
{
}

void NotifyManager::success( const QString& message, const QString& title )
{
    if(Choqok::UI::Global::mainWindow()->isActiveWindow()){
        choqokMainWindow->showStatusMessage(message);
    } else {
        _nmp->triggerNotify("job-success", title, message);
    }
}

void NotifyManager::error( const QString& message, const QString& title )
{
    _nmp->triggerNotify("job-error", title, message);
}

void NotifyManager::newPostArrived( const QString& message, const QString& title )
{
    QString fullMsg = QString( "<qt><b>%1:</b><br/>%2</qt>" ).arg(title).arg(message);
    if(Choqok::UI::Global::mainWindow()->isActiveWindow() || Choqok::BehaviorSettings::knotify()){
        choqokMainWindow->showStatusMessage(message);
    } else {
        if ( Choqok::BehaviorSettings::knotify() ){
             KNotification *n = new KNotification("new-post-arrived", choqokMainWindow);
             n->setActions(QStringList(i18nc( "Show Choqok MainWindow", "Show Choqok")));
             n->setText(fullMsg);
             QObject::connect(n, SIGNAL(activated(uint)), choqokMainWindow, SLOT(activateChoqok()));
             n->sendEvent();
        }
    }
}

void NotifyManager::shortening( const QString& message, const QString& title )
{
    _nmp->triggerNotify("shortening", title, message);
}

void NotifyManagerPrivate::triggerNotify(const QString& eventId, const QString& title,
                             const QString& message, KNotification::NotificationFlags flags)
{
    QString fullMsg = QString( "<qt><b>%1:</b><br/>%2</qt>" ).arg(title).arg(message);
    KNotification::event(eventId, fullMsg, QPixmap(), Choqok::UI::Global::mainWindow(), flags);
}

}
