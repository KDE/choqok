/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERLISTDIALOG_H
#define TWITTERLISTDIALOG_H

#include <QDialog>

#include "ui_twitterlistdialog_base.h"
#include "twitterlist.h"

namespace Choqok
{
class Account;
}

class QListWidget;
class QListWidgetItem;
class TwitterMicroBlog;
class TwitterAccount;
class TwitterApiAccount;

class TwitterListDialog : public QDialog
{
    Q_OBJECT
public:
    explicit TwitterListDialog(TwitterApiAccount *theAccount, QWidget *parent = nullptr);
    ~TwitterListDialog();

protected Q_SLOTS:
    virtual void accept() override;
    void slotUsernameChanged(const QString &name);
    void loadUserLists();
    void slotLoadUserlists(Choqok::Account *theAccount, QString username, QList<Twitter::List> list);
    void slotListItemChanged(QListWidgetItem *item);

private:
    Ui::TwitterListDialogBase ui;
    TwitterAccount *account;
    TwitterMicroBlog *blog;
    QWidget *mainWidget;
    QListWidget *listWidget;
};

#endif // TWITTERLISTDIALOG_H
