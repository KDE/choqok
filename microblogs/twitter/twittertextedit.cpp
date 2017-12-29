/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017 Andrea Scarpino <scarpino@kde.org>

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
#include "twittertextedit.h"

#include <QJsonDocument>
#include <QLabel>

#include <KIO/Job>

#include "urlutils.h"

#include "twitterapiaccount.h"
#include "twitterapimicroblog.h"

#include "twitterdebug.h"

class TwitterTextEdit::Private
{
public:
    Private(Choqok::Account *theAccount)
        : acc(theAccount), tCoMaximumLength(0), tCoMaximumLengthHttps(0)
    {}
    Choqok::Account *acc;
    int tCoMaximumLength;
    int tCoMaximumLengthHttps;
};

TwitterTextEdit::TwitterTextEdit(Choqok::Account *theAccount, QWidget *parent)
    : TwitterApiTextEdit(theAccount, parent), d(new Private(theAccount))
{
    qCDebug(CHOQOK);
    fetchTCoMaximumLength();
}

TwitterTextEdit::~TwitterTextEdit()
{
    delete d;
}

void TwitterTextEdit::updateRemainingCharsCount()
{
    QString txt = this->toPlainText();
    int count = txt.count();
    if (count) {
        lblRemainChar->show();
        if (charLimit()) {
            int remain = charLimit() - count;

            for (const QString &url: UrlUtils::detectUrls(txt)) {
                // Twitter does not wrapps urls with login informations
                if (!url.contains(QLatin1Char('@'))) {
                    int diff = -1;
                    if (url.startsWith(QLatin1String("http://"))) {
                        diff = url.length() - d->tCoMaximumLength;
                    } else if (url.startsWith(QLatin1String("https://"))) {
                        diff = url.length() - d->tCoMaximumLengthHttps;
                    }

                    if (diff > 0) {
                        remain += diff;
                    }
                }
            }

            if (remain < 0) {
                lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: red;}"));
            } else if (remain < 30) {
                lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: rgb(242, 179, 19);}"));
            } else {
                lblRemainChar->setStyleSheet(QLatin1String("QLabel {color: green;}"));
            }
            lblRemainChar->setText(QString::number(remain));
        } else {
            lblRemainChar->setText(QString::number(count));
            lblRemainChar->setStyleSheet(QLatin1String(QLatin1String("QLabel {color: blue;}")));
        }
        txt.remove(QRegExp(QLatin1String("@([^\\s\\W]+)")));
        txt = txt.trimmed();
        if (firstChar() != txt[0]) {
            setFirstChar(txt[0]);
            txt.prepend(QLatin1Char(' '));
            QTextBlockFormat f;
            f.setLayoutDirection((Qt::LayoutDirection) txt.isRightToLeft());
            textCursor().mergeBlockFormat(f);
        }
    } else {
        lblRemainChar->hide();
    }
}

void TwitterTextEdit::fetchTCoMaximumLength()
{
    TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *>(d->acc);
    if (acc) {
        QUrl url = acc->apiUrl();
        url.setPath(url.path() + QLatin1String("/help/configuration.json"));

        KIO::StoredTransferJob *job = KIO::storedGet(url, KIO::Reload, KIO::HideProgressInfo);
        if (!job) {
            qCDebug(CHOQOK) << "Cannot create an http GET request!";
            return;
        }
        TwitterApiMicroBlog *mBlog = qobject_cast<TwitterApiMicroBlog *>(acc->microblog());
        job->addMetaData(QStringLiteral("customHTTPHeader"),
                         QStringLiteral("Authorization: ") +
                         QLatin1String(mBlog->authorizationHeader(acc, url, QNetworkAccessManager::GetOperation)));
        connect(job, &KIO::StoredTransferJob::result, this, &TwitterTextEdit::slotTCoMaximumLength);
        job->start();
    } else {
        qCDebug(CHOQOK) << "the account is not a TwitterAPIAccount!";
    }
}

void TwitterTextEdit::slotTCoMaximumLength(KJob *job)
{
    if (job->error()) {
        qCDebug(CHOQOK) << "Job Error:" << job->errorString();
    } else {
        KIO::StoredTransferJob *j = qobject_cast<KIO::StoredTransferJob * >(job);
        const QJsonDocument json = QJsonDocument::fromJson(j->data());
        if (!json.isNull()) {
            const QVariantMap reply = json.toVariant().toMap();
            d->tCoMaximumLength = reply[QLatin1String("short_url_length")].toInt();
            d->tCoMaximumLengthHttps = reply[QLatin1String("short_url_length_https")].toInt();
        } else {
            qCDebug(CHOQOK) << "Cannot parse JSON reply";
        }
    }
}
