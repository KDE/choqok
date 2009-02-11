/*
    This file is part of choqoK, the KDE mono-blogging client

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
#include "accountmanager.h"
#include <kdebug.h>
#include <KConfig>
#include <KConfigGroup>
#include <kio/deletejob.h>
#include <kwallet.h>
#include <kstandarddirs.h>
#include "backend.h"

AccountManager::AccountManager( QObject* parent ):
        QObject( parent ), mWallet( 0 )
{
    kDebug();
    mWallet = KWallet::Wallet::openWallet( "kdewallet", 0 );
    if ( mWallet ) {
        if ( !mWallet->setFolder( "choqok" ) ) {
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
    kDebug();
    mSelf = 0L;
    conf->sync();
    delete conf;
}

AccountManager * AccountManager::mSelf = 0L;

AccountManager * AccountManager::self()
{
    if ( !mSelf )
        mSelf = new AccountManager;
    return mSelf;
}

const QList< Account > & AccountManager::accounts() const
{
    return mAccounts;
}

Account AccountManager::findAccount( const QString &alias )
{
    kDebug() << "Finding: " << alias;
    int count = mAccounts.count();
    for ( int i = 0; i < count; ++i ) {
        if ( mAccounts[i].alias() == alias ) {
            mAccounts[i].setError( false );
            return mAccounts[i];
        }
    }
    Account a;
    a.setError( true );
    return a;
}

bool AccountManager::removeAccount( const QString &alias )
{
    kDebug() << "Removing " << alias;
    int count = mAccounts.count();
    for ( int i = 0; i < count; ++i ) {
        if ( mAccounts[i].alias() == alias ) {
            conf->deleteGroup( QString::fromLatin1( "Account%1" ).arg( alias ) );
            conf->sync();
            mAccounts.removeAt( i );
            if ( mWallet ) {
                if ( mWallet->removeEntry( alias ) == 0 ) {
                    kDebug() << "Password successfully removed from kde wallet";
                }
            }
            for ( int i = Backend::HomeTimeLine; i <= Backend::OutboxTimeLine; ++i ) {
                QString tmpFile;
                tmpFile = KStandardDirs::locate( "data", "choqok/" + generateStatusBackupFileName( alias, ( Backend::TimeLineType )i ) );
                kDebug() << "Will remove " << tmpFile;
                const KUrl path( tmpFile );
                KIO::DeleteJob * delJob = KIO::del( path, KIO::HideProgressInfo );
                delJob->start();
            }
            emit accountRemoved( alias );
            return true;
        }
    }
    return false;
}

Account & AccountManager::addAccount( Account & account )
{
    kDebug() << "Adding: " << account.alias();

    if ( account.alias().isEmpty() ) {
        account.setError( true );
        return account;
    }

    // If this account already exists, do nothing
    QListIterator<Account> it( mAccounts );
    while ( it.hasNext() ) {
        Account curracc = it.next();
        if ( account.alias() == curracc.alias() ) {
            account.setError( true );
            return account;
        }
    }
    mAccounts.append( account );
    KConfigGroup acConf( conf, QString::fromLatin1( "Account%1" ).arg( account.alias() ) );
    acConf.writeEntry( "alias", account.alias() );
    acConf.writeEntry( "username", account.username() );
    acConf.writeEntry( "userId", account.userId() );
    acConf.writeEntry( "service_type", (int)account.serviceType() );
//     acConf.writeEntry( "service", account.serviceName() );
//     acConf.writeEntry( "api_path", account.apiPath() );
    acConf.writeEntry( "direction", ( account.direction() == Qt::RightToLeft ) ? "rtl" : "ltr" );
    if ( mWallet && mWallet->writePassword( account.serviceName() + '_' + account.username(), account.password() ) == 0 ) {
        kDebug() << "Password stored to kde wallet";
    } else {
        acConf.writeEntry( "password", account.password() );
        kDebug() << "Password stored to config file";
    }
    conf->sync();
    emit accountAdded( account );
    account.setError( false );
    return account;
}

Account & AccountManager::modifyAccount( Account & account, const QString & previousAlias )
{
    kDebug() << "Modifying: " << previousAlias;

    if ( removeAccount( previousAlias ) )
        return addAccount( account );

    account.setError( true );
    return account;
}

void AccountManager::loadAccounts()
{
    kDebug();
    QStringList list = conf->groupList();
    int count = list.count();
    for ( int i = 0; i < count; ++i ) {
        if ( list[i].contains( "Account" ) ) {
            Account a;
            KConfigGroup accountGrp( conf, list[i] );
            a.setUsername( accountGrp.readEntry( "username", QString() ) );
            a.setUserId( accountGrp.readEntry( "userId", uint( -1 ) ) );
            a.setAlias( accountGrp.readEntry( "alias", QString() ) );
            int service_type = accountGrp.readEntry( "service_type", -1 );
            if(service_type == -1){///For compatibility with previous versions (e.g. 0.3.1 )
                QString service = accountGrp.readEntry( "service", QString() );
                if( service.toLower() == QString(IDENTICA_SERVICE_TEXT).toLower() ) {
                    a.setServiceType(Account::Identica);
                } else {
                    a.setServiceType(Account::Twitter);
                }
            } else {
                a.setServiceType( (Account::Service) service_type );
            }
            a.setDirection(( accountGrp.readEntry( "direction", "ltr" ) == "rtl" ) ? Qt::RightToLeft : Qt::LeftToRight );
            QString buffer;
            if ( mWallet && mWallet->readPassword( a.serviceName() + '_' + a.username(), buffer ) == 0 && !buffer.isEmpty() ) {
                a.setPassword( buffer );
                kDebug() << "Password loaded from kde wallet.";
            } else {
                a.setPassword( accountGrp.readEntry( "password", QString() ) );
                kDebug() << "Password loaded from config file.";
            }
            a.setError( false );
            if ( a.userId() == ( uint ) - 1 ) {///Just for compatibility with previous versions
                Account *account = new Account( a );
                Backend *b = new Backend( account );
                connect( b, SIGNAL( userVerified( Account* ) ), this, SLOT( userVerified( Account* ) ) );
                b->verifyCredential();
            }
            mAccounts.append( a );
        }
    }
    kDebug() << mAccounts.count() << " accounts loaded.";
}

void AccountManager::userVerified( Account * userAccount )
{
    this->modifyAccount( *userAccount, userAccount->alias() );
}

QStringList AccountManager::listFriends( const QString & alias )
{
    KConfigGroup accountGrp( conf, "Account" + alias );
    QStringList list = accountGrp.readEntry( "friends", QStringList() );
    return list;
}

void AccountManager::saveFriendsList( const QString & alias, const QStringList & list )
{
//     if(findAccount( alias ).isError())
//         return;
    KConfigGroup accountGrp( conf, "Account" + alias );
    accountGrp.writeEntry( "friends", list );
    accountGrp.sync();
}

QString AccountManager::generateStatusBackupFileName( const QString &alias, Backend::TimeLineType type )
{
    QString name = alias;
    name += '_';

    switch ( type ) {
    case Backend::HomeTimeLine:
        name += "home";
        break;
    case Backend::ReplyTimeLine:
        name += "reply";
        break;
    case Backend::InboxTimeLine:
        name += "inbox";
        break;
    case Backend::OutboxTimeLine:
        name += "outbox";
        break;
    default:
        name += QString::number( type );
        break;
    };
    name += "statuslistrc";
    return name;
}

#include "accountmanager.moc"
