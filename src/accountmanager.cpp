/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/
#include "accountmanager.h"
#include <kdebug.h>
#include <KConfig>
#include <KConfigGroup>
// #include <kjob.h>
// #include <kio/job.h>
// #include <kio/deletejob.h>
// #include <kio/netaccess.h>
#include <kwallet.h>
#include <kstandarddirs.h>
#include "backend.h"

AccountManager::AccountManager(QObject* parent):
		QObject(parent), mWallet(0)
{
	kDebug();
	mWallet = KWallet::Wallet::openWallet( "kdewallet", 0 );
    if ( mWallet ) {
        if(!mWallet->setFolder( "choqok" )){
            mWallet->createFolder( "choqok" );
            mWallet->setFolder( "choqok" );
        }
		kDebug() << "Wallet successfully opened.";
	}
    conf = new KConfig( );
    loadAccounts();
}


AccountManager::~AccountManager()
{
    mSelf = 0L;
    conf->sync();
    delete conf;
}

AccountManager * AccountManager::mSelf = 0L;

AccountManager * AccountManager::self()
{
    if( !mSelf )
        mSelf = new AccountManager;
    return mSelf;
}

const QList< Account > & AccountManager::accounts() const
{
    return mAccounts;
}

Account AccountManager::findAccount( QString &alias )
{
    kDebug()<<"Finding: "<<alias;
    int count = mAccounts.count();
    for ( int i=0; i<count; ++i )
    {
        if ( mAccounts[i].alias() == alias ){
            mAccounts[i].setIsError( false );
            return mAccounts[i];
        }
    }
    Account a;
    a.setIsError ( true );
    return a;
}

bool AccountManager::removeAccount(const QString &alias)
{
    kDebug()<<"Removing "<<alias;
    int count = mAccounts.count();
    for(int i=0; i<count; ++i){
        if(mAccounts[i].alias() == alias){
            conf->deleteGroup( QString::fromLatin1("Account%1").arg(alias) );
            conf->sync();
            mAccounts.removeAt(i);
            if(mWallet){
                if(mWallet->removeEntry(alias)==0)
                    kDebug()<<"Password successfully removed from kde wallet";
            }
//             QString tmpFile;
//             tmpFile = KStandardDirs::locate("data", QString::fromLatin1("%1_homestatuslistrc").arg(alias));
//             kDebug()<<"Will remove "<<tmpFile;
//             const KUrl homePath(tmpFile);
//             DeleteJob * delJob = KIO::del(homePath, KIO::HideProgressInfo);
//             KIO::NetAccess::synchronousRun(delJob, 0);
//             tmpFile = KStandardDirs::locate("data", QString::fromLatin1("%1_replystatuslistrc").arg(alias));
//             kDebug()<<"Will remove "<<tmpFile;
//             const KUrl replyPath(tmpFile);
//             delJob = KIO::del(replyPath, KIO::HideProgressInfo);
//             KIO::NetAccess::synchronousRun(delJob, 0);
			///TODO Remove statuslist rc files.
            emit accountRemoved(alias);
            return true;
        }
    }
    return false;
}

Account & AccountManager::addAccount(Account & account)
{
    kDebug()<<"Adding: "<<account.alias();
    
    if( account.alias().isEmpty() )
    {
        account.setIsError( true );
        return account;
    }
    
    // If this account already exists, do nothing
    QListIterator<Account> it( mAccounts );
    while ( it.hasNext() )
    {
        Account curracc = it.next();
        if ( account.alias() == curracc.alias() )
        {
            account.setIsError( true );
            return account;
        }
    }
    mAccounts.append( account );
    KConfigGroup acConf ( conf, QString::fromLatin1("Account%1").arg(account.alias()) );
    acConf.writeEntry ( "alias", account.alias() );
    acConf.writeEntry ( "username", account.username() );
    acConf.writeEntry ( "userId", account.userId() );
    acConf.writeEntry ( "service", account.serviceName() );
    acConf.writeEntry ( "api_path", account.apiPath() );
    acConf.writeEntry ( "direction", ( account.direction() == Qt::RightToLeft ) ? "rtl" : "ltr" );
    if(mWallet && mWallet->writePassword(account.serviceName()+'_'+account.username(), account.password())==0){
        kDebug()<<"Password stored to kde wallet";
    } else {
        acConf.writeEntry ( "password", account.password() );
        kDebug()<<"Password stored to config file";
    }
    conf->sync();
    emit accountAdded(account);
    account.setIsError( false );
    return account;
}

Account & AccountManager::modifyAccount(Account & account, const QString & previousAlias)
{
    kDebug()<<"Modifying: "<<previousAlias;
    
    if(removeAccount(previousAlias))
        return addAccount(account);
    
    account.setIsError ( true );
    return account;
}

void AccountManager::loadAccounts()
{
    kDebug();
    QStringList list = conf->groupList();
    int count = list.count();
    for(int i=0; i<count; ++i){
        if(list[i].contains("Account")){
            Account a;
            KConfigGroup accountGrp(conf, list[i]);
            a.setUsername( accountGrp.readEntry("username", QString()) );
            a.setUserId( accountGrp.readEntry( "userId", uint(-1) ) );
            a.setAlias( accountGrp.readEntry("alias", QString()) );
            a.setServiceName( accountGrp.readEntry("service", QString()) );
            a.setApiPath( accountGrp.readEntry("api_path", QString()) );
            a.setDirection( (accountGrp.readEntry("direction", "ltr") == "rtl") ? Qt::RightToLeft : Qt::LeftToRight );
            QString buffer;
            if(mWallet && mWallet->readPassword( a.serviceName()+'_'+a.username(), buffer )==0 && !buffer.isEmpty()){
                a.setPassword( buffer );
                kDebug()<<"Password loaded from kde wallet.";
            } else {
                a.setPassword( accountGrp.readEntry("password", QString()) );
                kDebug()<<"Password loaded from config file.";
            }
            a.setIsError( false );
            if(a.userId() == -1){///Just for compatibility with previous versions
                Account *account = new Account(a);
                Backend *b = new Backend(account);
                connect(b, SIGNAL(userVerified(Account*)), this, SLOT(userVerified(Account*)));
                b->verifyCredential();
            }
            mAccounts.append(a);
        }
    }
    kDebug()<<mAccounts.count()<<" accounts loaded.";
}

void AccountManager::userVerified(Account * userAccount)
{
    this->modifyAccount(*userAccount, userAccount->alias());
}

QStringList AccountManager::listFriends(const QString & alias)
{
    KConfigGroup accountGrp( conf, "Account" + alias );
    QStringList list = accountGrp.readEntry( "friends", QStringList() );
    return list;
}

void AccountManager::saveFriendsList(const QString & alias, const QStringList & list)
{
    KConfigGroup accountGrp( conf, "Account" + alias );
    accountGrp.writeEntry( "friends", list );
    accountGrp.sync();
}

#include "accountmanager.moc"
