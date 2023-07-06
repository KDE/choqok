/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterlistdialog.h"

#include <QGridLayout>
#include <QLineEdit>
#include <QListWidget>
#include <QPushButton>

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
    connect(ui.username, &QLineEdit::textChanged, this, &TwitterListDialog::slotUsernameChanged);
    connect(ui.loadUserLists, &QPushButton::clicked, this, &TwitterListDialog::loadUserLists);
    QRegExp rx(QLatin1String("([a-z0-9_]){1,20}(\\/)"), Qt::CaseInsensitive);
    QValidator *val = new QRegExpValidator(rx, nullptr);
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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TwitterListDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TwitterListDialog::reject);
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
    connect(blog, &TwitterMicroBlog::userLists, this, &TwitterListDialog::slotLoadUserlists);
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
        connect(listWidget, &QListWidget::itemClicked, this, &TwitterListDialog::slotListItemChanged);
    }
}

void TwitterListDialog::slotListItemChanged(QListWidgetItem *item)
{
    ui.listname->setText(item->data(32).toString());
}

#include "moc_twitterlistdialog.cpp"
