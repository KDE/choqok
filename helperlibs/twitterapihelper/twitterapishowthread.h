/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPISHOWTHREAD_H
#define TWITTERAPISHOWTHREAD_H

#include <QWidget>

#include "choqoktypes.h"

namespace Choqok
{
class Account;
namespace UI
{
class PostWidget;
}
}

class CHOQOK_HELPER_EXPORT TwitterApiShowThread : public QWidget
{
    Q_OBJECT
public:
    TwitterApiShowThread(Choqok::Account *account, Choqok::Post *finalPost, QWidget *parent = nullptr);
    ~TwitterApiShowThread();

protected Q_SLOTS:
    void slotAddNewPost(Choqok::Account *theAccount, Choqok::Post *post);
    void raiseMainWindow();

Q_SIGNALS:
    void forwardResendPost(const QString &post);
    void forwardReply(const QString &txt, const QString &replyToId, const QString &replyToUsername);

protected:
    void addPostWidgetToUi(Choqok::UI::PostWidget *widget);
private:
    void setupUi();

    class Private;
    Private *const d;
};

#endif // TWITTERAPISHOWTHREAD_H
