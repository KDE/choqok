/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "choqoktextedit.h"
#include "accountmanager.h"
#include "choqokbehaviorsettings.h"
#include <KDebug>
#include <microblog.h>
#include <QCheckBox>
#include <klocalizedstring.h>
#include <kcombobox.h>
#include <QHBoxLayout>
#include <KPluginInfo>

using namespace Choqok;

QuickPost::QuickPost( QWidget* parent )
    : KDialog( parent ), mSubmittedPost(0)
{
    kDebug();
    setupUi();
    loadAccounts();
    connect( comboAccounts, SIGNAL(currentIndexChanged(int)),
             this, SLOT(slotCurrentAccountChanged(int)) );
    connect( txtPost, SIGNAL( returnPressed( QString ) ),
             this, SLOT( submitPost( QString ) ) );
    connect( all, SIGNAL( toggled( bool ) ),
             this, SLOT( checkAll( bool ) ) );
    connect( AccountManager::self(), SIGNAL( accountAdded(Choqok::Account*)),
             this, SLOT( addAccount( Choqok::Account*)) );
    connect( AccountManager::self(), SIGNAL( accountRemoved( const QString& ) ),
             this, SLOT( removeAccount( const QString& ) ) );

    all->setChecked( Choqok::BehaviorSettings::all() );
    slotCurrentAccountChanged(comboAccounts->currentIndex());
}

void QuickPost::setupUi()
{
    QWidget *wdg = new QWidget( this );
    this->setMainWidget( wdg );

    this->resize( Choqok::BehaviorSettings::quickPostDialogSize() );
    all = new QCheckBox( i18n("All"), this);
    comboAccounts = new KComboBox(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(all);
    hLayout->addWidget(comboAccounts);
    mainLayout->addLayout(hLayout);
    txtPost = new TextEdit( 0, this );
    txtPost->setTabChangesFocus( true );
    mainLayout->addWidget( txtPost );
    if(wdg->layout())
        wdg->layout()->deleteLater();
    wdg->setLayout(mainLayout);
    txtPost->setFocus( Qt::OtherFocusReason );
    this->setCaption( i18n( "Quick Post" ) );
}

QuickPost::~QuickPost()
{
    BehaviorSettings::setAll( all->isChecked() );
    BehaviorSettings::setQuickPostDialogSize( this->size() );
    BehaviorSettings::self()->writeConfig();
    kDebug();
}

void QuickPost::show()
{
    txtPost->setFocus( Qt::OtherFocusReason );
    KDialog::show();
}

void QuickPost::slotSubmitPost( Post *post )
{
    if (post == mSubmittedPost) {
        txtPost->setEnabled(true);
        txtPost->clear();
        emit newPostSubmitted(Success);
        //TODO Notify
        delete mSubmittedPost;
        mSubmittedPost = 0L;
    }
}

void QuickPost::postError(Choqok::MicroBlog::ErrorType error, const QString &errorMessage, const Post* post)
{
    if (post == mSubmittedPost) {
        txtPost->setEnabled(true);
        emit newPostSubmitted(Fail);
        //TODO Notify
        delete mSubmittedPost;
        mSubmittedPost = 0L;
    }
}

void QuickPost::submitPost( const QString & newPost )
{
    kDebug();
    this->hide();
    if ( all->isChecked() ) {
        int count = accountsList.count();
            mSubmittedPost = new Post;
            mSubmittedPost->content = newPost;
            mSubmittedPost->isPrivate = false;
        for ( int i = 0;i < count; ++i ) {
            accountsList[i]->microblog()->createPost(accountsList[i], mSubmittedPost );
        }
    } else {
        mSubmittedPost = new Post;
        mSubmittedPost->content = newPost;
        mSubmittedPost->isPrivate = false;
        accountsList[comboAccounts->currentIndex()]->microblog()->createPost(accountsList[comboAccounts->currentIndex()],
                                                                             mSubmittedPost );
    }
}

void QuickPost::slotButtonClicked(int button)
{
    kDebug();
    if(button == KDialog::Ok) {
        QString txt = txtPost->toPlainText();
        submitPost( txt );
    } else
        KDialog::slotButtonClicked(button);
}

void QuickPost::loadAccounts()
{
    kDebug();
    QList<Account*> ac = AccountManager::self()->accounts();
    QListIterator<Account*> it( ac );
    while ( it.hasNext() ) {
        Account *current = it.next();
        accountsList.append( current );
        comboAccounts->addItem( KIcon(current->microblog()->pluginInfo().icon()), current->alias() );
        connect(current->microblog(), SIGNAL(postCreated(Post*)), SLOT(slotSubmitPost(Post*)));
        connect(current->microblog(), SIGNAL(errorPost(Choqok::MicroBlog::ErrorType,QString,const Post*)),
                 SLOT(postError(Choqok::MicroBlog::ErrorType,QString,const Post*)));
    }
}

void QuickPost::addAccount( Choqok::Account* account )
{
    kDebug();
    accountsList.append( account );
    comboAccounts->addItem( account->alias() );
}

void QuickPost::removeAccount( const QString & alias )
{
    kDebug();
    int count = accountsList.count();
    for ( int i = 0; i < count; ++i ) {
        if ( accountsList[i]->alias() == alias ) {
            accountsList.removeAt( i );
            comboAccounts->removeItem( i );
            return;
        }
    }
}

void QuickPost::checkAll( bool isAll )
{
    comboAccounts->setEnabled( !isAll );
}

void QuickPost::setText( const QString &text )
{
    txtPost->setText(text);
}

void QuickPost::slotCurrentAccountChanged(int index)
{
    txtPost->setCharLimit( accountsList[index]->microblog()->postCharLimit() );
}

#include "quickpost.moc"
