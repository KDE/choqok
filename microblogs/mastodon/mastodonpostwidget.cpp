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

#include "mastodonpostwidget.h"

#include <QAction>
#include <QMenu>
#include <QPushButton>

#include <KLocalizedString>

#include "mediamanager.h"
#include "textbrowser.h"

#include "mastodonaccount.h"
#include "mastodondebug.h"
#include "mastodonmicroblog.h"
#include "mastodonpost.h"

const QIcon MastodonPostWidget::unFavIcon(Choqok::MediaManager::convertToGrayScale(QIcon::fromTheme(QLatin1String("rating")).pixmap(16)));

class MastodonPostWidget::Private
{
public:
    QPushButton *btnFavorite;
};

MastodonPostWidget::MastodonPostWidget(Choqok::Account *account, Choqok::Post *post,
                                   QWidget *parent):
    PostWidget(account, post, parent), d(new Private)
{
}

MastodonPostWidget::~MastodonPostWidget()
{
    delete d;
}

QString MastodonPostWidget::generateSign()
{
    QString ss;

    MastodonPost *post = dynamic_cast<MastodonPost * >(currentPost());
    MastodonAccount *account = qobject_cast<MastodonAccount * >(currentAccount());
    MastodonMicroBlog *microblog = qobject_cast<MastodonMicroBlog * >(account->microblog());
    if (post) {
        ss += QStringLiteral("<b>%1 - </b>").arg(getUsernameHyperlink(currentPost()->author));

        QDateTime time;
        if (post->repeatedDateTime.isNull()) {
            time = post->creationDateTime;
        } else {
            time = post->repeatedDateTime;
        }

        ss += QStringLiteral("<a href=\"%1\" title=\"%2\">%3</a>").arg(post->link.toDisplayString())
                .arg(post->creationDateTime.toString(Qt::DefaultLocaleLongDate))
                .arg(formatDateTime(time));

        if (!post->source.isEmpty()) {
            ss += QLatin1String(" - ") + post->source;
        }

        //ReTweet detection
        if (!currentPost()->repeatedFromUser.userName.isEmpty()) {
            const QString retweet = QLatin1String("<br/>") +
                    microblog->generateRepeatedByUserTooltip(QStringLiteral("<a href=\"%1\">%2</a>")
                                                             .arg(currentPost()->repeatedFromUser.homePageUrl.toDisplayString())
                                                             .arg(currentPost()->repeatedFromUser.userName));
            ss.append(retweet);
        }
    } else {
        qCDebug(CHOQOK) << "post is not a MastodonPost!";
    }

    return ss;
}

QString MastodonPostWidget::getUsernameHyperlink(const Choqok::User &user) const
{
    return QStringLiteral("<a href=\"%1\" title=\"%2\">%3</a>")
            .arg(user.homePageUrl.toDisplayString())
            .arg(user.description.isEmpty() ? user.realName : user.description.toHtmlEscaped())
            .arg(user.userName);
}

void MastodonPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    if (isResendAvailable()) {
        buttons().value(QLatin1String("btnResend"))->setToolTip(i18nc("@info:tooltip", "Boost"));
    }

    d->btnFavorite = addButton(QLatin1String("btnFavorite"), i18nc("@info:tooltip", "Favourite"), QLatin1String("rating"));
    d->btnFavorite->setCheckable(true);
    connect(d->btnFavorite, &QPushButton::clicked, this, &MastodonPostWidget::toggleFavorite);
    updateFavStat();
}

void MastodonPostWidget::slotResendPost()
{
    qCDebug(CHOQOK);
    setReadWithSignal();
    MastodonMicroBlog *microBlog = qobject_cast<MastodonMicroBlog *>(currentAccount()->microblog());
    microBlog->toggleReblog(currentAccount(), currentPost());
}

void MastodonPostWidget::toggleFavorite()
{
    qCDebug(CHOQOK);
    setReadWithSignal();
    MastodonMicroBlog *microBlog = qobject_cast<MastodonMicroBlog *>(currentAccount()->microblog());
    connect(microBlog, &MastodonMicroBlog::favorite, this, &MastodonPostWidget::slotToggleFavorite);
    microBlog->toggleFavorite(currentAccount(), currentPost());
}

void MastodonPostWidget::slotToggleFavorite(Choqok::Account *, Choqok::Post *)
{
    qCDebug(CHOQOK);
    updateFavStat();
}

void MastodonPostWidget::updateFavStat()
{
    d->btnFavorite->setChecked(currentPost()->isFavorited);
    if (currentPost()->isFavorited) {
        d->btnFavorite->setIcon(QIcon::fromTheme(QLatin1String("rating")));
    } else {
        d->btnFavorite->setIcon(unFavIcon);
    }
}
