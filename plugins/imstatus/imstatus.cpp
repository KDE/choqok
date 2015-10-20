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

#include "imstatus.h"

#include <QTimer>

#include <KPluginFactory>

#include "choqokuiglobal.h"
#include "quickpost.h"

#include "imqdbus.h"
#include "imstatussettings.h"

K_PLUGIN_FACTORY_WITH_JSON(IMStatusFactory, "choqok_imstatus.json",
                           registerPlugin < IMStatus > ();)

class IMStatusPrivate
{
public:
    IMStatusPrivate() {}
    IMQDBus *im;
};

IMStatus::IMStatus(QObject *parent, const QList<QVariant> &)
    : Choqok::Plugin(QLatin1String("choqok_imstatus"), parent), d(new IMStatusPrivate())
{
    QTimer::singleShot(500, this, SLOT(update()));
    d->im = new IMQDBus(this);
}

IMStatus::~IMStatus()
{
    delete d;
}

void IMStatus::update()
{
    if (Choqok::UI::Global::quickPostWidget() != 0) {
        connect(Choqok::UI::Global::quickPostWidget(), SIGNAL(newPostSubmitted(Choqok::JobResult,Choqok::Post*)),
                this, SLOT(slotIMStatus(Choqok::JobResult,Choqok::Post*)));
    } else {
        QTimer::singleShot(500, this, SLOT(update()));
    }
}

void IMStatus::slotIMStatus(Choqok::JobResult res, Choqok::Post *newPost)
{
    if (res == Choqok::Success) {
        IMStatusSettings::self()->load();
        QString statusMessage = IMStatusSettings::templtate();
        statusMessage.replace(QLatin1String("%status%"), newPost->content, Qt::CaseInsensitive);
        statusMessage.replace(QLatin1String("%username%"), newPost->author.userName, Qt::CaseInsensitive);
        statusMessage.replace(QLatin1String("%fullname%"), newPost->author.realName, Qt::CaseInsensitive);
        statusMessage.replace(QLatin1String("%time%"), newPost->creationDateTime.toString(QLatin1String("hh:mm:ss")), Qt::CaseInsensitive);
        statusMessage.replace(QLatin1String("%url%"), newPost->link, Qt::CaseInsensitive);
        statusMessage.replace(QLatin1String("%client%"), QLatin1String("Choqok"), Qt::CaseInsensitive);
        if (!IMStatusSettings::repeat() && !newPost->repeatedFromUsername.isEmpty()) {
            return;
        }
        if (!IMStatusSettings::reply() && !newPost->replyToUserName.isEmpty()) {
            return;
        }
        d->im->updateStatusMessage(IMStatusSettings::imclient(), statusMessage);
    }
}

#include "imstatus.moc"
