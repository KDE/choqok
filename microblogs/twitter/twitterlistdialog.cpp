/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twitterlistdialog.h"

#include <QGridLayout>
#include <QListWidget>

#include <KMessageBox>
#include <KLocalizedString>

#include "twitteraccount.h"
#include "twitterdebug.h"
#include "twittermicroblog.h"

TwitterListDialog::TwitterListDialog(TwitterApiAccount *theAccount, QWidget *parent)
    : QDialog(parent)
{
    if (theAccount) {
        account = qobject_cast<TwitterAccount *>(theAccount);
        if (!account) {
            qCCritical(CHOQOK) << "TwitterListDialog: ERROR, the provided account is not a valid Twitter account";
            return;
        }
    } else {
        qCCritical(CHOQOK) << "TwitterListDialog: ERROR, theAccount is NULL";
        return;
    }
    blog = qobject_cast<TwitterMicroBlog *>(account->microblog());
    mainWidget = new QWidget(this);
    ui.setupUi(mainWidget);
    connect(ui.username, SIGNAL(textChanged(QString)), SLOT(slotUsernameChanged(QString)));
    connect(ui.loadUserLists, SIGNAL(clicked(bool)), SLOT(loadUserLists()));
    QRegExp rx(QLatin1String("([a-z0-9_]){1,20}(\\/)"), Qt::CaseInsensitive);
    QValidator *val = new QRegExpValidator(rx, 0);
    ui.username->setValidator(val);
    ui.username->setFocus();
    listWidget = new QListWidget(this);
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(ui.label, 0, 0);
    layout->addWidget(ui.username, 0, 1);
    layout->addWidget(ui.loadUserLists, 0, 2);
    layout->addWidget(listWidget, 1, 0, 1, -1);
    layout->addWidget(ui.label_2, 2, 0);
    layout->addWidget(ui.listname, 2, 1, 1, -1);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18n("Add"));
    QPushButton *cancelButton = buttonBox->button(QDialogButtonBox::Cancel);
    cancelButton->setIcon(KStandardGuiItem::close().icon());
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    layout->addWidget(buttonBox, 3, 3, 1, -1);

    mainWidget->setLayout(layout);

    mainWidget->adjustSize();
}

TwitterListDialog::~TwitterListDialog()
{
}

void TwitterListDialog::accept()
{
    if (ui.listname->text().isEmpty() || ui.username->text().isEmpty()) {
        KMessageBox::error(this, i18n("You should provide both list author username and list name."));
    } else {
        blog->addListTimeline(account, ui.username->text(), ui.listname->text());
        QDialog::accept();
    }
}

void TwitterListDialog::slotUsernameChanged(const QString &name)
{
    if (name.endsWith(QLatin1Char('/'))) {
        QString n = name;
        n.chop(1);
        ui.username->setText(n);
        ui.listname->setFocus();
    }
    listWidget->clear();
    ui.listname->clear();
}

void TwitterListDialog::loadUserLists()
{
    if (ui.username->text().isEmpty()) {
        KMessageBox::error(choqokMainWindow, i18n("No user."));
        return;
    }
    connect(blog, SIGNAL(userLists(Choqok::Account*,QString,QList<Twitter::List>)),
            SLOT(slotLoadUserlists(Choqok::Account*,QString,QList<Twitter::List>)));
    blog->fetchUserLists(account, ui.username->text());
}

void TwitterListDialog::slotLoadUserlists(Choqok::Account *theAccount, QString username,
        QList<Twitter::List> list)
{
    if (theAccount == account && QString::compare(username, ui.username->text()) == 0 && !list.isEmpty()) {
        listWidget->clear();
        for (const Twitter::List &l: list) {
            QListWidgetItem *item = new QListWidgetItem(listWidget);
            QString iText;
            if (l.description.isEmpty()) {
                iText = l.fullname;
            } else {
                iText = QStringLiteral("%1 [%2]").arg(l.fullname).arg(l.description);
            }
            item->setText(iText);
            item->setData(32, l.slug);
            listWidget->addItem(item);
        }
        connect(listWidget, SIGNAL(itemClicked(QListWidgetItem*)),
                SLOT(slotListItemChanged(QListWidgetItem*)));
    }
}

void TwitterListDialog::slotListItemChanged(QListWidgetItem *item)
{
    ui.listname->setText(item->data(32).toString());
}

