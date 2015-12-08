/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
    explicit TwitterApiDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~TwitterApiDMessageDialog();
    void setTo(const QString &username);

protected Q_SLOTS:
    virtual void accept();

    void followersUsernameListed(TwitterApiAccount *, QStringList);
    void submitPost(QString);
    void reloadFriendslist();
    void postCreated(Choqok::Account *, Choqok::Post *);
    void errorPost(Choqok::Account *, Choqok::Post *, Choqok::MicroBlog::ErrorType,
                   QString, Choqok::MicroBlog::ErrorLevel);

protected:
    virtual void setupUi(QWidget *mainWidget);
    void setFriends(const QStringList friends);
    Choqok::UI::TextEdit *editor();

private:
    class Private;
    Private *const d;
};

#endif // TWITTERAPIDMESSAGEDIALOG_H
