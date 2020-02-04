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
                                                             .arg(microblog->userNameFromAcct(currentPost()->repeatedFromUser.userName)));
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
            .arg(MastodonMicroBlog::userNameFromAcct(user.userName));
}

void MastodonPostWidget::initUi()
{
    Choqok::UI::PostWidget::initUi();

    if (isResendAvailable()) {
        buttons().value(QLatin1String("btnResend"))->setToolTip(i18nc("@info:tooltip", "Boost"));
    }

    QPushButton *btnRe = addButton(QLatin1String("btnReply"), i18nc("@info:tooltip", "Reply"), QLatin1String("edit-undo"));
    connect(btnRe, &QPushButton::clicked, this, &MastodonPostWidget::slotReply);
    QMenu *menu = new QMenu(btnRe);
    btnRe->setMenu(menu);

    QAction *actRep = new QAction(QIcon::fromTheme(QLatin1String("edit-undo")), i18n("Reply to %1", currentPost()->author.userName), menu);
    menu->addAction(actRep);
    menu->setDefaultAction(actRep);
    connect(actRep, &QAction::triggered, this, &MastodonPostWidget::slotReply);

    QAction *actWrite = new QAction(QIcon::fromTheme(QLatin1String("document-edit")), i18n("Write to %1", currentPost()->author.userName), menu);
    menu->addAction(actWrite);
    connect(actWrite, &QAction::triggered, this, &MastodonPostWidget::slotWriteTo);

    if (!currentPost()->isPrivate) {
        QAction *actReplytoAll = new QAction(i18n("Reply to all"), menu);
        menu->addAction(actReplytoAll);
        connect(actReplytoAll, &QAction::triggered, this, &MastodonPostWidget::slotReplyToAll);
    }

    d->btnFavorite = addButton(QLatin1String("btnFavorite"), i18nc("@info:tooltip", "Favourite"), QLatin1String("rating"));
    d->btnFavorite->setCheckable(true);
    connect(d->btnFavorite, &QPushButton::clicked, this, &MastodonPostWidget::toggleFavorite);
    updateFavStat();
}

void MastodonPostWidget::slotReply()
{
    setReadWithSignal();
    if (currentPost()->isPrivate) {
        MastodonAccount *account = qobject_cast<MastodonAccount *>(currentAccount());
        MastodonMicroBlog *microblog = qobject_cast<MastodonMicroBlog * >(account->microblog());
        microblog->showDirectMessageDialog(account, currentPost()->author.userName);
    } else {
        QString replyto = QStringLiteral("@%1").arg(currentPost()->author.userName);
        QString postId = currentPost()->postId;
        QString username = currentPost()->author.userName;
        if (!currentPost()->repeatedFromUser.userName.isEmpty()) {
            replyto.prepend(QStringLiteral("@%1 ").arg(currentPost()->repeatedFromUser.userName));
            postId = currentPost()->repeatedPostId;
        }
        Q_EMIT reply(replyto, postId,  username);
    }
}

void MastodonPostWidget::slotWriteTo()
{
    Q_EMIT reply(QStringLiteral("@%1").arg(currentPost()->author.userName), QString(), currentPost()->author.userName);
}

void MastodonPostWidget::slotReplyToAll()
{
    QString txt = QStringLiteral("@%1").arg(currentPost()->author.userName);
    Q_EMIT reply(txt, currentPost()->postId, currentPost()->author.userName);
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
