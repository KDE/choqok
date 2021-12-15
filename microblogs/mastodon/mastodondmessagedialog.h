/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
    explicit MastodonDMessageDialog(MastodonAccount *theAccount, QWidget *parent = nullptr, Qt::WindowFlags flags = {});
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
