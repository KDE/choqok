/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "quickpost.h"

#include <QCheckBox>
#include <QHBoxLayout>

#include <KComboBox>
#include <KDebug>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPushButton>

#include "accountmanager.h"
#include "choqokbehaviorsettings.h"
#include "choqoktextedit.h"
#include "microblog.h"
#include "notifymanager.h"
#include "shortenmanager.h"
#include "uploadmediadialog.h"

using namespace Choqok::UI;
using namespace Choqok;

class QuickPost::Private
{
public:
    Private()
    : submittedPost(0), isPostSubmitted(false)
    {}
    QCheckBox *all;
    KComboBox *comboAccounts;
    TextEdit *txtPost;

    QHash< QString, Account* > accountsList;
    Post *submittedPost;
    QList<Account*> submittedAccounts;
    bool isPostSubmitted;
    KPushButton *attach;
//     QString replyToId;
};

QuickPost::QuickPost( QWidget* parent )
    : KDialog( parent ), d(new Private)
{
    kDebug();
    setupUi();
    loadAccounts();
    connect( d->comboAccounts, SIGNAL(currentIndexChanged(int)),
             this, SLOT(slotCurrentAccountChanged(int)) );
    connect( d->txtPost, SIGNAL( returnPressed( QString ) ),
             this, SLOT( submitPost( QString ) ) );
    connect( d->all, SIGNAL( toggled( bool ) ),
             this, SLOT( checkAll( bool ) ) );
    connect( AccountManager::self(), SIGNAL( accountAdded(Choqok::Account*)),
             this, SLOT( addAccount( Choqok::Account*)) );
    connect( AccountManager::self(), SIGNAL( accountRemoved( const QString& ) ),
             this, SLOT( removeAccount( const QString& ) ) );
    connect( d->attach, SIGNAL(clicked(bool)),
             this, SLOT(slotAttachMedium()));

    d->all->setChecked( Choqok::BehaviorSettings::all() );
    slotCurrentAccountChanged(d->comboAccounts->currentIndex());
    setButtonText(Ok, i18nc("Submit post", "Submit"));
}

void QuickPost::setupUi()
{
    QWidget *wdg = new QWidget( this );
    this->setMainWidget( wdg );

    this->resize( Choqok::BehaviorSettings::quickPostDialogSize() );
    d->all = new QCheckBox( i18nc("All accounts", "All"), this);
    d->comboAccounts = new KComboBox(this);
    d->attach = new KPushButton(KIcon("mail-attachment"), QString(), this);
    d->attach->setMaximumWidth(d->attach->height());
    d->attach->setToolTip(i18n("Attach a file"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(d->all);
    hLayout->addWidget(d->comboAccounts);
    hLayout->addWidget(d->attach);
    mainLayout->addLayout(hLayout);
    d->txtPost = new TextEdit( 0, this );
    d->txtPost->setTabChangesFocus( true );
    mainLayout->addWidget( d->txtPost );
    if(wdg->layout())
        wdg->layout()->deleteLater();
    wdg->setLayout(mainLayout);
    d->txtPost->setFocus( Qt::OtherFocusReason );
    this->setCaption( i18n( "Quick Post" ) );
}

QuickPost::~QuickPost()
{
    BehaviorSettings::setAll( d->all->isChecked() );
    BehaviorSettings::setQuickPostDialogSize( this->size() );
    BehaviorSettings::self()->writeConfig();
    delete d;
    kDebug();
}

void QuickPost::show()
{
    d->txtPost->setFocus( Qt::OtherFocusReason );
    KDialog::show();
}

void QuickPost::slotSubmitPost( Account* a, Post* post )
{
    if (post == d->submittedPost && d->submittedAccounts.removeOne(a)) {
        Q_EMIT newPostSubmitted(Success, d->submittedPost);
    }
    if(d->isPostSubmitted && d->submittedAccounts.isEmpty()){
        d->txtPost->setEnabled(true);
        d->txtPost->clear();
        delete d->submittedPost;
        d->submittedPost = 0L;
        d->isPostSubmitted = false;
    }
}

void QuickPost::postError(Account* a, Choqok::Post* post,
                          Choqok::MicroBlog::ErrorType , const QString& )
{
    if (post == d->submittedPost && d->submittedAccounts.contains(a)) {
        d->txtPost->setEnabled(true);
        Q_EMIT newPostSubmitted(Fail, post); //Choqok crashes without post :(
        show();
    }
    if(d->submittedAccounts.isEmpty()){
        d->txtPost->setEnabled(true);
        delete d->submittedPost;
        d->submittedPost = 0L;
    }
}

void QuickPost::submitPost( const QString & txt )
{
    kDebug();
    QString newPost = txt;
    Choqok::Account* currentAccount = d->accountsList.value(d->comboAccounts->currentText());
    if(!currentAccount) {
        KMessageBox::error(choqokMainWindow, i18n("Please configure at least one account to be included in \"Quick Post\".\nSettings -> Configure Choqok... -> Accounts"));
        return;
    }
    this->hide();
    d->submittedAccounts.clear();
    if( currentAccount->postCharLimit() &&
        newPost.size() > (int)currentAccount->postCharLimit() )
        newPost = Choqok::ShortenManager::self()->parseText(newPost);
        delete d->submittedPost;
    if ( d->all->isChecked() ) {
            d->submittedPost = new Post;
            d->submittedPost->content = newPost;
            d->submittedPost->isPrivate = false;
        Q_FOREACH (Account* acc, d->accountsList) {
            acc->microblog()->createPost( acc, d->submittedPost );
            d->submittedAccounts<<acc;
        }
    } else {
        d->submittedPost = new Post;
        d->submittedPost->content = newPost;
        d->submittedPost->isPrivate = false;
        d->submittedAccounts<<currentAccount;
        currentAccount->microblog()->createPost(d->accountsList.value( d->comboAccounts->currentText()),
                                                                       d->submittedPost );
    }
    d->isPostSubmitted = true;
}

void QuickPost::slotButtonClicked(int button)
{
    kDebug();
    if(button == KDialog::Ok) {
        submitPost( d->txtPost->toPlainText() );
    } else
        KDialog::slotButtonClicked(button);
}

void QuickPost::loadAccounts()
{
    kDebug();
    QList<Account*> ac = AccountManager::self()->accounts();
    QListIterator<Account*> it( ac );
    while ( it.hasNext() ) {
        addAccount(it.next());
    }
}

void QuickPost::addAccount( Choqok::Account* account )
{
    kDebug();
    connect(account, SIGNAL(modified(Choqok::Account*)), SLOT(accountModified(Choqok::Account*)) );//Added for later changes
    if(account->isReadOnly() || !account->showInQuickPost())
        return;
    d->accountsList.insert( account->alias(), account );
    d->comboAccounts->addItem( KIcon(account->microblog()->pluginIcon()), account->alias() );
    connect(account->microblog(), SIGNAL(postCreated(Choqok::Account*,Choqok::Post*)),
            SLOT(slotSubmitPost(Choqok::Account*,Choqok::Post*)) );
    connect(account->microblog(),
            SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,
                                Choqok::MicroBlog::ErrorType,QString)),
            SLOT(postError(Choqok::Account*,Choqok::Post*,
                            Choqok::MicroBlog::ErrorType,QString)) );
}

void QuickPost::removeAccount( const QString & alias )
{
    kDebug();
    d->accountsList.remove( alias );
    d->comboAccounts->removeItem( d->comboAccounts->findText(alias) );
}

void QuickPost::checkAll( bool isAll )
{
    d->comboAccounts->setEnabled( !isAll );
}

void QuickPost::setText( const QString& text )
{
    d->txtPost->setPlainText(text);
    this->show();
//     if(account)
//         d->comboAccounts->setCurrentItem(account->alias());
//     if(!replyToId.isEmpty())
//         d->replyToId = replyToId;
}

void QuickPost::appendText(const QString& text)
{
    d->txtPost->appendText(text);
    this->show();
}


void QuickPost::slotCurrentAccountChanged(int index)
{
    Q_UNUSED(index)
    if( !d->accountsList.isEmpty() )
        d->txtPost->setCharLimit( d->accountsList.value(d->comboAccounts->currentText())->postCharLimit() );
}

void QuickPost::accountModified(Account* theAccount)
{
    kDebug();
    if( !theAccount->isReadOnly() && theAccount->showInQuickPost() ) {
        if( !d->accountsList.contains(theAccount->alias()) )
            addAccount(theAccount);
    } else if(d->accountsList.contains(theAccount->alias())){
        removeAccount(theAccount->alias());
    }
}

void QuickPost::slotAttachMedium()
{
    KMessageBox::information(this, i18n("Link to uploaded medium will be added here after uploading process succeed."),
                             QString(), "quickPostAttachMedium");
    QPointer<UploadMediaDialog> dlg = new UploadMediaDialog(this);
    dlg->show();
}


