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

#include "mastodondmessagedialog.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QLabel>
#include <QHBoxLayout>
#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KSharedConfig>

#include "choqoktextedit.h"
#include "microblog.h"
#include "notifymanager.h"

#include "mastodonaccount.h"
#include "mastodondebug.h"
#include "mastodonmicroblog.h"

class MastodonDMessageDialog::Private
{
public:
    Private(MastodonAccount *theAccount)
        : account(theAccount)
    {}
    QComboBox *comboFriendsList;
    Choqok::UI::TextEdit *editor;
    MastodonAccount *account;
    Choqok::Post *sentPost;
};

MastodonDMessageDialog::MastodonDMessageDialog(MastodonAccount *theAccount, QWidget *parent,
        Qt::WindowFlags flags)
    : QDialog(parent, flags), d(new Private(theAccount))
{
    setWindowTitle(i18n("Send Private Message"));
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    KConfigGroup grp(KSharedConfig::openConfig(), "Mastodon");
    resize(grp.readEntry("DMessageDialogSize", QSize(300, 200)));
    QStringList list = theAccount->followers();
    if (list.isEmpty()) {
        reloadFriendslist();
    } else {
        list.sort(Qt::CaseInsensitive);
        d->comboFriendsList->addItems(list);
    }
}

MastodonDMessageDialog::~MastodonDMessageDialog()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "Mastodon");
    grp.writeEntry("DMessageDialogSize", size());
    grp.sync();
    delete d;
}

void MastodonDMessageDialog::setupUi(QWidget *mainWidget)
{
    QLabel *lblTo = new QLabel(i18nc("Send message to", "To:"), this);
    d->comboFriendsList = new QComboBox(this);
    d->comboFriendsList->setDuplicatesEnabled(false);

    QPushButton *btnReload = new QPushButton(this);
    btnReload->setToolTip(i18n("Reload friends list"));
    btnReload->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    btnReload->setMaximumWidth(25);
    connect(btnReload, &QPushButton::clicked, this, &MastodonDMessageDialog::reloadFriendslist);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);

    QHBoxLayout *toLayout = new QHBoxLayout;
    toLayout->addWidget(lblTo);
    toLayout->addWidget(d->comboFriendsList);
    toLayout->addWidget(btnReload);
    mainLayout->addLayout(toLayout);

    d->editor = new Choqok::UI::TextEdit(d->account->postCharLimit());
    connect(d->editor, &Choqok::UI::TextEdit::returnPressed, this, &MastodonDMessageDialog::submitPost);
    mainLayout->addWidget(d->editor);
    d->editor->setFocus();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18nc("Send private message", "Send"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &MastodonDMessageDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &MastodonDMessageDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void MastodonDMessageDialog::setFriends(const QStringList friends)
{
    d->comboFriendsList->clear();
    d->comboFriendsList->addItems(friends);
}

Choqok::UI::TextEdit *MastodonDMessageDialog::editor()
{
    return d->editor;
}

MastodonAccount *MastodonDMessageDialog::account()
{
    return d->account;
}

void MastodonDMessageDialog::reloadFriendslist()
{
    d->comboFriendsList->clear();
    MastodonMicroBlog *blog = qobject_cast<MastodonMicroBlog *>(d->account->microblog());
    if (blog) {
        connect(blog, &MastodonMicroBlog::followersUsernameListed,
                this, &MastodonDMessageDialog::followersUsernameListed);
        blog->fetchFollowers(d->account, true);
        d->comboFriendsList->setCurrentText(i18n("Please wait..."));
    }
}

void MastodonDMessageDialog::accept()
{
    submitPost(d->editor->toPlainText());
}

void MastodonDMessageDialog::submitPost(QString text)
{
    if (d->account->following().isEmpty() || text.isEmpty() || d->comboFriendsList->currentText().isEmpty()) {
        return;
    }
    hide();
    connect(d->account->microblog(), &Choqok::MicroBlog::errorPost,
            this, &MastodonDMessageDialog::errorPost);
    connect(d->account->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
            this, SLOT(postCreated(Choqok::Account*,Choqok::Post*)));
    d->sentPost = new Choqok::Post;
    d->sentPost->isPrivate = true;
    d->sentPost->replyToUser.userName = d->comboFriendsList->currentText();
    d->sentPost->content = text;
    d->account->microblog()->createPost(d->account, d->sentPost);
}

void MastodonDMessageDialog::followersUsernameListed(MastodonAccount *theAccount, QStringList list)
{
    if (theAccount == d->account) {
        d->comboFriendsList->clear();
        list.sort(Qt::CaseInsensitive);
        d->comboFriendsList->addItems(list);
    }
}

void MastodonDMessageDialog::errorPost(Choqok::Account *theAccount, Choqok::Post *thePost,
        Choqok::MicroBlog::ErrorType , QString ,
        Choqok::MicroBlog::ErrorLevel)
{
    if (theAccount == d->account && thePost == d->sentPost) {
        qCDebug(CHOQOK);
        show();
    }
}

void MastodonDMessageDialog::setTo(const QString &username)
{
    d->comboFriendsList->setCurrentText(username);
}

