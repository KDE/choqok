/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIWHOISWIDGET_H
#define TWITTERAPIWHOISWIDGET_H

#include <QUrl>
#include <QFrame>

#include "choqoktypes.h"
#include "choqok_export.h"

namespace Choqok
{
class Account;
}

class TwitterApiAccount;
class KJob;
class CHOQOK_HELPER_EXPORT TwitterApiWhoisWidget : public QFrame
{
    Q_OBJECT
public:
    TwitterApiWhoisWidget(TwitterApiAccount *theAccount, const QString &userName,
                          const Choqok::Post &post, QWidget *parent = nullptr);
    ~TwitterApiWhoisWidget();
    void show(QPoint pos);

protected Q_SLOTS:
    void checkAnchor(const QUrl url);
    void userInfoReceived(KJob *job);
    void slotCancel();
    void avatarFetchError(const QUrl &remoteUrl, const QString &errMsg);
    void avatarFetched(const QUrl &remoteUrl, const QPixmap &pixmap);

    void slotFriendshipCreated(Choqok::Account *, const QString &);
    void slotFriendshipDestroyed(Choqok::Account *, const QString &);
//     void slotUserBlocked(Choqok::Account*, const QString&);

protected:
    void updateHtml();
    void setActionImages();

//     static const QString baseText;
private:
    void setupUi();
    void showForm();
    void loadUserInfo(TwitterApiAccount *thAccount, const QString &username);
    class Private;
    Private *const d;
};

#endif // TWITTERAPIWHOISWIDGET_H
