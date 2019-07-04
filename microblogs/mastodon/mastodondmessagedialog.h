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

#ifndef MASTODONDMESSAGEDIALOG_H
#define MASTODONDMESSAGEDIALOG_H

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

class MastodonAccount;

class MastodonDMessageDialog : public QDialog
{
    Q_OBJECT
public:
    explicit MastodonDMessageDialog(MastodonAccount *theAccount, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~MastodonDMessageDialog();
    void setTo(const QString &username);

protected Q_SLOTS:
    virtual void accept() override;

    void followersUsernameListed(MastodonAccount *, QStringList);
    void submitPost(QString);
    void reloadFriendslist();
    void errorPost(Choqok::Account *, Choqok::Post *, Choqok::MicroBlog::ErrorType,
                   QString, Choqok::MicroBlog::ErrorLevel);

protected:
    void setupUi(QWidget *mainWidget);
    void setFriends(const QStringList friends);
    Choqok::UI::TextEdit *editor();
    MastodonAccount *account();

private:
    class Private;
    Private *const d;
};

#endif // MASTODONDMESSAGEDIALOG_H
