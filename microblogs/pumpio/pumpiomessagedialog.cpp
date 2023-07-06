/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "pumpiomessagedialog.h"

#include <QDialogButtonBox>
#include <QFileDialog>
#include <QPointer>
#include <QPushButton>

#include "pumpioaccount.h"
#include "pumpiodebug.h"
#include "pumpiomicroblog.h"
#include "pumpiopost.h"

class PumpIOMessageDialog::Private
{
public:
    Choqok::Account *account;
    QString mediumToAttach;
    QPointer<QLabel> mediumName;
    QPointer<QPushButton> btnCancel;
};

PumpIOMessageDialog::PumpIOMessageDialog(Choqok::Account *theAccount, QWidget *parent,
        Qt::WindowFlags flags)
    : QDialog(parent, flags)
    , d(new Private)
{
    d->account = theAccount;

    setupUi(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &PumpIOMessageDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &PumpIOMessageDialog::reject);
    verticalLayout->addWidget(buttonBox);

    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
    if (acc) {
        for (const QVariant &list: acc->lists()) {
            QVariantMap l = list.toMap();
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(l.value(QLatin1String("name")).toString());
            item->setData(Qt::UserRole, l.value(QLatin1String("id")).toString());
            toList->addItem(item);
            ccList->addItem(item->clone());
        }
        //Lists are not sorted
        toList->sortItems();
        ccList->sortItems();

        for (const QString &username: acc->following()) {
            QListWidgetItem *item = new QListWidgetItem;
            item->setText(PumpIOMicroBlog::userNameFromAcct(username));
            item->setData(Qt::UserRole, username);
            toList->addItem(item);
            ccList->addItem(item->clone());
        }
    }

    connect(btnReload, &QPushButton::clicked, this, &PumpIOMessageDialog::fetchFollowing);
    connect(btnAttach, &QPushButton::clicked, this, &PumpIOMessageDialog::attachMedia);
}

PumpIOMessageDialog::~PumpIOMessageDialog()
{
    delete d;
}

void PumpIOMessageDialog::fetchFollowing()
{
    qCDebug(CHOQOK);
    toList->clear();
    ccList->clear();
    PumpIOMicroBlog *microblog = qobject_cast<PumpIOMicroBlog *>(d->account->microblog());
    if (microblog) {
        microblog->fetchFollowing(d->account);
        connect(microblog, &PumpIOMicroBlog::followingFetched, this,
                &PumpIOMessageDialog::slotFetchFollowing);
    }
}

void PumpIOMessageDialog::slotFetchFollowing(Choqok::Account *theAccount)
{
    qCDebug(CHOQOK);
    if (theAccount == d->account) {
        PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(theAccount);
        if (acc) {
            for (const QVariant &list: acc->lists()) {
                QVariantMap l = list.toMap();
                QListWidgetItem *item = new QListWidgetItem;
                item->setText(l.value(QLatin1String("name")).toString());
                item->setData(Qt::UserRole, l.value(QLatin1String("id")).toString());
                toList->addItem(item);
                ccList->addItem(item->clone());
            }
            toList->sortItems();
            ccList->sortItems();

            for (const QString &username: acc->following()) {
                QListWidgetItem *item = new QListWidgetItem;
                item->setText(PumpIOMicroBlog::userNameFromAcct(username));
                item->setData(Qt::UserRole, username);
                toList->addItem(item);
                ccList->addItem(item->clone());
            }
        }
    }
}

void PumpIOMessageDialog::accept()
{
    qCDebug(CHOQOK);
    PumpIOAccount *acc = qobject_cast<PumpIOAccount *>(d->account);
    if (acc) {
        if (acc->following().isEmpty() || txtMessage->toPlainText().isEmpty()
                || (toList->selectedItems().isEmpty() && ccList->selectedItems().isEmpty())) {
            return;
        }
        hide();
        PumpIOMicroBlog *microblog = qobject_cast<PumpIOMicroBlog *>(d->account->microblog());
        if (microblog) {
            PumpIOPost *post = new PumpIOPost;
            post->content = txtMessage->toPlainText();

            QVariantList to;
            for (QListWidgetItem *item: toList->selectedItems()) {
                QVariantMap user;
                QString id = item->data(Qt::UserRole).toString();
                if (id.contains(QLatin1String("acct:"))) {
                    user.insert(QLatin1String("objectType"), QLatin1String("person"));
                } else {
                    user.insert(QLatin1String("objectType"), QLatin1String("collection"));
                }
                user.insert(QLatin1String("id"), id);
                to.append(user);
            }

            QVariantList cc;
            for (QListWidgetItem *item: ccList->selectedItems()) {
                QVariantMap user;
                QString id = item->data(Qt::UserRole).toString();
                if (id.contains(QLatin1String("acct:"))) {
                    user.insert(QLatin1String("objectType"), QLatin1String("person"));
                } else {
                    user.insert(QLatin1String("objectType"), QLatin1String("collection"));
                }
                user.insert(QLatin1String("id"), id);
                cc.append(user);
            }

            microblog->createPost(acc, post, to, cc);
        }
    }
}

void PumpIOMessageDialog::attachMedia()
{
    qCDebug(CHOQOK);
    d->mediumToAttach = QFileDialog::getOpenFileName(this, i18n("Select Media to Upload"),
                                                     QString(), QStringLiteral("Images"));
    if (d->mediumToAttach.isEmpty()) {
        qCDebug(CHOQOK) << "No file selected";
        return;
    }
    const QString fileName = QUrl(d->mediumToAttach).fileName();
    if (!d->mediumName) {
        d->mediumName = new QLabel(this);
        d->btnCancel = new QPushButton(this);
        d->btnCancel->setIcon(QIcon::fromTheme(QLatin1String("list-remove")));
        d->btnCancel->setToolTip(i18n("Discard Attachment"));
        d->btnCancel->setMaximumWidth(d->btnCancel->height());
        connect(d->btnCancel, &QPushButton::clicked, this, &PumpIOMessageDialog::cancelAttach);

        horizontalLayout->insertWidget(1, d->mediumName);
        horizontalLayout->insertWidget(2, d->btnCancel);
    }
    d->mediumName->setText(i18n("Attaching <b>%1</b>", fileName));
    txtMessage->setFocus();
}

void PumpIOMessageDialog::cancelAttach()
{
    qCDebug(CHOQOK);
    delete d->mediumName;
    d->mediumName = nullptr;
    delete d->btnCancel;
    d->btnCancel = nullptr;
    d->mediumToAttach.clear();
}

#include "moc_pumpiomessagedialog.cpp"
