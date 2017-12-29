/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2015 Andrea Scarpino <scarpino@kde.org>

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
#include "twitterdmessagedialog.h"

#include <QJsonDocument>

#include <KIO/Job>

#include "choqoktextedit.h"

#include "twitterapiaccount.h"
#include "twitterapidebug.h"
#include "twitterapimicroblog.h"

TwitterDMessageDialog::TwitterDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent,
        Qt::WindowFlags flags)
    : TwitterApiDMessageDialog(theAccount, parent, flags)
{
    fetchTextLimit();
}

TwitterDMessageDialog::~TwitterDMessageDialog()
{
}

void TwitterDMessageDialog::fetchTextLimit()
{
    QUrl url = account()->apiUrl();
    url.setPath(url.path() + QLatin1String("/help/configuration.json"));

    KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
    if (!job) {
        qCDebug(CHOQOK) << "Cannot create an http GET request!";
        return;
    }
    TwitterApiMicroBlog *mBlog = qobject_cast<TwitterApiMicroBlog *>(account()->microblog());
    job->addMetaData(QStringLiteral("customHTTPHeader"),
                     QStringLiteral("Authorization: ") +
                     QLatin1String(mBlog->authorizationHeader(account(), url, QNetworkAccessManager::GetOperation)));
    connect(job, &KIO::StoredTransferJob::result, this, &TwitterDMessageDialog::slotTextLimit);
    job->start();
}

void TwitterDMessageDialog::slotTextLimit(KJob *job)
{
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);
        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const int textLimit = json.toVariant().toMap().value(QLatin1String("dm_text_character_limit")).toInt();
            editor()->setCharLimit(textLimit);
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }
}
