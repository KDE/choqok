/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2015 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

#include "moc_twitterdmessagedialog.cpp"
