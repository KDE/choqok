/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "twitterapidmessagedialog.h"
#include <QLabel>
#include <kcombobox.h>
#include <QVBoxLayout>
#include <choqoktextedit.h>
#include "twitterapiaccount.h"
#include <microblog.h>
#include <klocalizedstring.h>
#include <KPushButton>
#include "twitterapimicroblog.h"
#include <KDebug>
#include <notifymanager.h>
#include <KMessageBox>

class TwitterApiDMessageDialog::Private
{
public:
    Private(TwitterApiAccount *theAccount)
    :account(theAccount)
    {}
    KComboBox *comboFriendsList;
    Choqok::UI::TextEdit *editor;
    TwitterApiAccount *account;
    Choqok::Post *sentPost;
};

TwitterApiDMessageDialog::TwitterApiDMessageDialog(TwitterApiAccount *theAccount, QWidget* parent,
                                                   Qt::WFlags flags)
    : KDialog(parent, flags), d(new Private(theAccount))
{
    setWindowTitle(i18n("Send Private Message"));
    setAttribute(Qt::WA_DeleteOnClose);
    QWidget *wg = new QWidget(this, flags);
    setMainWidget(wg);
    setupUi(wg);
    KConfigGroup grp(KGlobal::config(), "TwitterApi");
    resize( grp.readEntry("DMessageDialogSize", QSize(300, 200)) );
    QStringList list = theAccount->friendsList();
    if( list.isEmpty() ){
        reloadFriendslist();
    } else {
        list.sort();
        d->comboFriendsList->addItems(list);
    }
    setButtonText(Ok, i18nc("Send private message", "Send"));
}

TwitterApiDMessageDialog::~TwitterApiDMessageDialog()
{
    KConfigGroup grp(KGlobal::config(), "TwitterApi");
    grp.writeEntry("DMessageDialogSize", size());
    grp.sync();
    delete d;
}

void TwitterApiDMessageDialog::setupUi( QWidget *mainWidget )
{
    QLabel *lblTo = new QLabel( i18nc("Send message to", "To:"), this);
    d->comboFriendsList = new KComboBox(this);
    d->comboFriendsList->setDuplicatesEnabled(false);

    KPushButton *btnReload = new KPushButton(this);
    btnReload->setToolTip(i18n("Reload friends list"));
    btnReload->setIcon(KIcon("view-refresh"));
    btnReload->setMaximumWidth(25);
    connect( btnReload, SIGNAL(clicked(bool)), SLOT(reloadFriendslist()) );

    QVBoxLayout *mainLayout = new QVBoxLayout(mainWidget);
    QHBoxLayout *toLayout = new QHBoxLayout;

    toLayout->addWidget(lblTo);
    toLayout->addWidget(d->comboFriendsList);
    toLayout->addWidget(btnReload);
    mainLayout->addLayout(toLayout);

    d->editor = new Choqok::UI::TextEdit( d->account->microblog()->postCharLimit() );
    connect( d->editor, SIGNAL(returnPressed(QString)), SLOT(submitPost(QString)) );
    mainLayout->addWidget(d->editor);
    d->editor->setFocus();
}

void TwitterApiDMessageDialog::reloadFriendslist()
{
    d->comboFriendsList->clear();
    TwitterApiMicroBlog * blog = qobject_cast<TwitterApiMicroBlog*>(d->account->microblog());
    if(blog) {
        connect( blog, SIGNAL(friendsUsernameListed(TwitterApiAccount*,QStringList)),
                 this, SLOT(friendsUsernameListed(TwitterApiAccount*,QStringList)) );
                 blog->listFriendsUsername(d->account);
                 d->comboFriendsList->setCurrentItem(i18n("Please wait..."), true);
    }
}

void TwitterApiDMessageDialog::slotButtonClicked(int button)
{
    if(button == KDialog::Ok){
        submitPost( d->editor->toPlainText() );
    } else {
        KDialog::slotButtonClicked(button);
    }
}

void TwitterApiDMessageDialog::submitPost(QString text)
{
    if( d->account->friendsList().isEmpty() || text.isEmpty() || d->comboFriendsList->currentText().isEmpty())
        return;
    hide();
    connect( d->account->microblog(),
                SIGNAL( errorPost(Choqok::Account*, Choqok::Post*,
                                Choqok::MicroBlog::ErrorType,QString, Choqok::MicroBlog::ErrorLevel) ),
                this,
                SLOT(errorPost(Choqok::Account*,Choqok::Post*,
                            Choqok::MicroBlog::ErrorType,QString,Choqok::MicroBlog::ErrorLevel)));
    connect( d->account->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
            this, SLOT(postCreated(Choqok::Account*,Choqok::Post*)) );
    d->sentPost = new Choqok::Post;
    d->sentPost->isPrivate = true;
    d->sentPost->replyToUserName = d->comboFriendsList->currentText();
    d->sentPost->content = text;
    d->account->microblog()->createPost(d->account, d->sentPost);
}

void TwitterApiDMessageDialog::friendsUsernameListed(TwitterApiAccount* theAccount, QStringList list)
{
    if(theAccount == d->account){
        d->comboFriendsList->removeItem(0);
        list.sort();
        d->comboFriendsList->addItems(list);
    }
}

void TwitterApiDMessageDialog::postCreated(Choqok::Account* theAccount, Choqok::Post* thePost)
{
    if(theAccount == d->account && thePost == d->sentPost){
        kDebug();
        accept();
    }
}

void TwitterApiDMessageDialog::errorPost(Choqok::Account* theAccount, Choqok::Post* thePost,
                                         Choqok::MicroBlog::ErrorType , QString ,
                                         Choqok::MicroBlog::ErrorLevel )
{
    if(theAccount == d->account && thePost == d->sentPost){
        kDebug();
        show();
    }
}

void TwitterApiDMessageDialog::setTo(const QString& username)
{
    d->comboFriendsList->setCurrentItem(username, true);
}

#include "twitterapidmessagedialog.moc"
