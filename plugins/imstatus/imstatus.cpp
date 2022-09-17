/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Andrey Esin <gmlastik@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "imstatus.h"

#include <QTimer>

#include <KPluginFactory>

#include "choqokuiglobal.h"
#include "quickpost.h"

#include "imqdbus.h"
#include "imstatussettings.h"

K_PLUGIN_CLASS_WITH_JSON(IMStatus, "choqok_imstatus.json")

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
    if (Choqok::UI::Global::quickPostWidget() != nullptr) {
        connect(Choqok::UI::Global::quickPostWidget(), &Choqok::UI::QuickPost::newPostSubmitted,
                this, &IMStatus::slotIMStatus);
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
        statusMessage.replace(QLatin1String("%url%"), newPost->link.toDisplayString(), Qt::CaseInsensitive);
        statusMessage.replace(QLatin1String("%client%"), QLatin1String("Choqok"), Qt::CaseInsensitive);
        if (!IMStatusSettings::repeat() && !newPost->repeatedFromUser.userName.isEmpty()) {
            return;
        }
        if (!IMStatusSettings::reply() && !newPost->replyToUser.userName.isEmpty()) {
            return;
        }
        d->im->updateStatusMessage(IMStatusSettings::imclient(), statusMessage);
    }
}

#include "imstatus.moc"
