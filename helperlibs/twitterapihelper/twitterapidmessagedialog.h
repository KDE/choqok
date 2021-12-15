/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIDMESSAGEDIALOG_H
#define TWITTERAPIDMESSAGEDIALOG_H

#include <QDialog>

#include "microblog.h"

namespace Choqok
{
class Account;
class Post;

    namespace UI
    {
        class TextEdit;
    }
}

class TwitterApiAccount;

class CHOQOK_HELPER_EXPORT TwitterApiDMessageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TwitterApiDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
    ~TwitterApiDMessageDialog();
    void setTo(const QString &username);

protected Q_SLOTS:
    virtual void accept() override;

    void followersUsernameListed(TwitterApiAccount *, QStringList);
    void submitPost(QString);
    void reloadFriendslist();
    void errorPost(Choqok::Account *, Choqok::Post *, Choqok::MicroBlog::ErrorType,
                   QString, Choqok::MicroBlog::ErrorLevel);

protected:
    void setupUi(QWidget *mainWidget);
    void setFriends(const QStringList friends);
    Choqok::UI::TextEdit *editor();
    TwitterApiAccount *account();

private:
    class Private;
    Private *const d;
};

#endif // TWITTERAPIDMESSAGEDIALOG_H
