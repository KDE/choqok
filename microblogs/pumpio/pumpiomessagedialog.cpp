/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013  Andrea Scarpino <scarpino@kde.org>

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

#include "pumpiomessagedialog.h"

#include <KDebug>

#include "pumpioaccount.h"
#include "pumpiomicroblog.h"
#include "pumpiopost.h"

class PumpIOMessageDialog::Private
{
public:
    Choqok::Account *account;
};

PumpIOMessageDialog::PumpIOMessageDialog(Choqok::Account* theAccount, QWidget* parent,
                                         Qt::WindowFlags flags)
                                        : KDialog(parent, flags)
                                        , d(new Private)
{
    d->account = theAccount;

    setupUi(this);

    setMainWidget(widget);

    PumpIOAccount* acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        Q_FOREACH (const QVariant& list, acc->lists()) {
            QVariantMap l = list.toMap();
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(l.value("name").toString());
            item->setData(Qt::UserRole, l.value("id").toString());
            toList->addItem(item);
            ccList->addItem(item->clone());
        }
        //Lists are not sorted
        toList->sortItems();
        ccList->sortItems();

        Q_FOREACH (const QString& username, acc->following()) {
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(PumpIOMicroBlog::userNameFromAcct(username));
            item->setData(Qt::UserRole, username);
            toList->addItem(item);
            ccList->addItem(item->clone());
        }
    }

    connect(btnReload, SIGNAL(clicked(bool)), this, SLOT(fetchFollowing()));
    connect(this, SIGNAL(okClicked()), this, SLOT(sendPost()));
}

PumpIOMessageDialog::~PumpIOMessageDialog()
{
    delete d;
}

void PumpIOMessageDialog::fetchFollowing()
{
    kDebug();
    toList->clear();
    ccList->clear();
    PumpIOMicroBlog *microblog = qobject_cast<PumpIOMicroBlog*>(d->account->microblog());
    if (microblog) {
        microblog->fetchFollowing(d->account);
        connect(microblog, SIGNAL(followingFetched(Choqok::Account*)), this,
                SLOT(slotFetchFollowing(Choqok::Account*)));
    }
}

void PumpIOMessageDialog::slotFetchFollowing(Choqok::Account* theAccount)
{
    kDebug();
    if (theAccount == d->account) {
        PumpIOAccount* acc = qobject_cast<PumpIOAccount *>(theAccount);
        if (acc) {
            Q_FOREACH (const QVariant& list, acc->lists()) {
                QVariantMap l = list.toMap();
                QListWidgetItem *item = new QListWidgetItem;
                item->setText(l.value("name").toString());
                item->setData(Qt::UserRole, l.value("id").toString());
                toList->addItem(item);
                ccList->addItem(item->clone());
            }
            toList->sortItems();
            ccList->sortItems();

            Q_FOREACH (const QString& username, acc->following()) {
                QListWidgetItem *item = new QListWidgetItem;
                item->setText(PumpIOMicroBlog::userNameFromAcct(username));
                item->setData(Qt::UserRole, username);
                toList->addItem(item);
                ccList->addItem(item->clone());
            }
        }
    }
}

void PumpIOMessageDialog::sendPost()
{
    kDebug();
    PumpIOAccount* acc = qobject_cast<PumpIOAccount *>(d->account);
    if (acc) {
        if (acc->following().isEmpty() || txtMessage->toPlainText().isEmpty()
            || (toList->selectedItems().isEmpty() && ccList->selectedItems().isEmpty())) {
            return;
        }
        hide();
        PumpIOMicroBlog* microblog = qobject_cast<PumpIOMicroBlog*>(d->account->microblog());
        if (microblog) {
            PumpIOPost* post = new PumpIOPost;
            post->content = txtMessage->toPlainText();
            post->type = "note";

            QVariantList to;
            Q_FOREACH (QListWidgetItem *item, toList->selectedItems()) {
                QVariantMap user;
                QString id = item->data(Qt::UserRole).toString();
                if (id.contains("acct:")) {
                    user.insert("objectType", "person");
                } else {
                    user.insert("objectType", "collection");
                }
                user.insert("id", id);
                to.append(user);
            }

            QVariantList cc;
            Q_FOREACH (QListWidgetItem *item, ccList->selectedItems()) {
                QVariantMap user;
                QString id = item->data(Qt::UserRole).toString();
                if (id.contains("acct:")) {
                    user.insert("objectType", "person");
                } else {
                    user.insert("objectType", "collection");
                }
                user.insert("id", id);
                cc.append(user);
            }

            microblog->createPost(acc, post, to, cc);
        }
    }
}
