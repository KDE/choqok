/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "twitterapidmessagedialog.h"

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

#include "twitterapiaccount.h"
#include "twitterapidebug.h"
#include "twitterapimicroblog.h"

class TwitterApiDMessageDialog::Private
{
public:
    Private(TwitterApiAccount *theAccount)
        : account(theAccount)
    {}
    QComboBox *comboFriendsList;
    Choqok::UI::TextEdit *editor;
    TwitterApiAccount *account;
    Choqok::Post *sentPost;
};

TwitterApiDMessageDialog::TwitterApiDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent,
        Qt::WindowFlags flags)
    : QDialog(parent, flags), d(new Private(theAccount))
{
    setWindowTitle(i18n("Send Private Message"));
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi(this);
    KConfigGroup grp(KSharedConfig::openConfig(), "TwitterApi");
    resize(grp.readEntry("DMessageDialogSize", QSize(300, 200)));
    QStringList list = theAccount->followersList();
    if (list.isEmpty()) {
        reloadFriendslist();
    } else {
        list.sort(Qt::CaseInsensitive);
        d->comboFriendsList->addItems(list);
    }
}

TwitterApiDMessageDialog::~TwitterApiDMessageDialog()
{
    KConfigGroup grp(KSharedConfig::openConfig(), "TwitterApi");
    grp.writeEntry("DMessageDialogSize", size());
    grp.sync();
    delete d;
}

void TwitterApiDMessageDialog::setupUi(QWidget *mainWidget)
{
    QLabel *lblTo = new QLabel(i18nc("Send message to", "To:"), this);
    d->comboFriendsList = new QComboBox(this);
    d->comboFriendsList->setDuplicatesEnabled(false);

    QPushButton *btnReload = new QPushButton(this);
    btnReload->setToolTip(i18n("Reload friends list"));
    btnReload->setIcon(QIcon::fromTheme(QLatin1String("view-refresh")));
    btnReload->setMaximumWidth(25);
    connect(btnReload, &QPushButton::clicked, this, &TwitterApiDMessageDialog::reloadFriendslist);

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);

    QHBoxLayout *toLayout = new QHBoxLayout;
    toLayout->addWidget(lblTo);
    toLayout->addWidget(d->comboFriendsList);
    toLayout->addWidget(btnReload);
    mainLayout->addLayout(toLayout);

    d->editor = new Choqok::UI::TextEdit(d->account->postCharLimit());
    connect(d->editor, &Choqok::UI::TextEdit::returnPressed, this, &TwitterApiDMessageDialog::submitPost);
    mainLayout->addWidget(d->editor);
    d->editor->setFocus();

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18nc("Send private message", "Send"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &TwitterApiDMessageDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &TwitterApiDMessageDialog::reject);
    mainLayout->addWidget(buttonBox);
}

void TwitterApiDMessageDialog::setFriends(const QStringList friends)
{
    d->comboFriendsList->clear();
    d->comboFriendsList->addItems(friends);
}

Choqok::UI::TextEdit *TwitterApiDMessageDialog::editor()
{
    return d->editor;
}

TwitterApiAccount *TwitterApiDMessageDialog::account()
{
    return d->account;
}

void TwitterApiDMessageDialog::reloadFriendslist()
{
    d->comboFriendsList->clear();
    TwitterApiMicroBlog *blog = qobject_cast<TwitterApiMicroBlog *>(d->account->microblog());
    if (blog) {
        connect(blog, &TwitterApiMicroBlog::followersUsernameListed, this,
                &TwitterApiDMessageDialog::followersUsernameListed);
        blog->listFollowersUsername(d->account);
        d->comboFriendsList->setCurrentText(i18n("Please wait..."));
    }
}

void TwitterApiDMessageDialog::accept()
{
    submitPost(d->editor->toPlainText());
}

void TwitterApiDMessageDialog::submitPost(QString text)
{
    if (d->account->friendsList().isEmpty() || text.isEmpty() || d->comboFriendsList->currentText().isEmpty()) {
        return;
    }
    hide();
    connect(d->account->microblog(), &Choqok::MicroBlog::errorPost, this,
            &TwitterApiDMessageDialog::errorPost);
    connect(d->account->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
           this, SLOT(postCreated(Choqok::Account*,Choqok::Post*)));
    d->sentPost = new Choqok::Post;
    d->sentPost->isPrivate = true;
    d->sentPost->replyToUser.userName = d->comboFriendsList->currentText();
    d->sentPost->content = text;
    d->account->microblog()->createPost(d->account, d->sentPost);
}

void TwitterApiDMessageDialog::followersUsernameListed(TwitterApiAccount *theAccount, QStringList list)
{
    if (theAccount == d->account) {
        d->comboFriendsList->clear();
        list.sort(Qt::CaseInsensitive);
        d->comboFriendsList->addItems(list);
    }
}

void TwitterApiDMessageDialog::errorPost(Choqok::Account *theAccount, Choqok::Post *thePost,
        Choqok::MicroBlog::ErrorType , QString ,
        Choqok::MicroBlog::ErrorLevel)
{
    if (theAccount == d->account && thePost == d->sentPost) {
        qCDebug(CHOQOK);
        show();
    }
}

void TwitterApiDMessageDialog::setTo(const QString &username)
{
    d->comboFriendsList->setCurrentText(username);
}

