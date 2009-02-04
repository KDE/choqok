/*
    This file is part of choqoK, the KDE Twitter client

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
#include "statuswidget.h"
#include "settings.h"
#include "mediamanager.h"
#include "backend.h"
#include <knotification.h>
#include <QProcess>
#define _15SECS 15000
#define _MINUTE 60000
#define _HOUR (60 * _MINUTE)
#define COLOROFFSET 20

StatusWidget::StatusWidget( const Account *account, QWidget *parent )
        : QFrame( parent )
{
    setupUi( this );
    mIsReaded = true;
    timer.start( _MINUTE );
    mCurrentAccount = account;
    btnFavorite->setIcon( KIcon( "rating" ) );
    btnReply->setIcon( KIcon( "edit-undo" ) );
    btnRemove->setIcon( KIcon( "edit-delete" ) );

    this->setMaximumHeight( 110 );

    connect( btnReply, SIGNAL( clicked( bool ) ), this, SLOT( requestReply() ) );
    connect( &timer, SIGNAL( timeout() ), this, SLOT( updateSign() ) );
    connect( btnFavorite, SIGNAL( clicked( bool ) ), this, SLOT( setFavorite( bool ) ) );
    connect( btnRemove, SIGNAL( clicked( bool ) ), this, SLOT( requestDestroy() ) );
}

StatusWidget::~StatusWidget()
{
}

void StatusWidget::setFavorite( bool isFavorite )
{
    emit sigFavorite( mCurrentStatus.statusId, isFavorite );
}

Status StatusWidget::currentStatus() const
{
    return mCurrentStatus;
}

void StatusWidget::setCurrentStatus( const Status newStatus )
{
    mCurrentStatus = newStatus;
    updateUi();
}

void StatusWidget::updateUi()
{
    if ( mCurrentStatus.isDMessage ) {
        btnFavorite->setVisible( false );
    } else if ( mCurrentStatus.user.userId == mCurrentAccount->userId() ) {
        btnReply->setVisible( false );
    } else {
        btnRemove->setVisible( false );
    }
    lblSign->setHtml( generateSign() );
    lblStatus->setHtml( prepareStatus( mCurrentStatus.content ) );
    lblImage->setToolTip( mCurrentStatus.user.name );
    setUiStyle();
    updateFavoriteUi();
    setUserImage();
}

QString StatusWidget::formatDateTime( const QDateTime &time )
{
    int seconds = time.secsTo( QDateTime::currentDateTime() );
    if ( seconds <= 15 ) {
        timer.setInterval( _15SECS );
        return i18n( "Just now" );
    }

    if ( seconds <= 45 ) {
        timer.setInterval( _15SECS );
        return i18np( "about 1 second ago", "about %1 seconds ago", seconds );
    }

    int minutes = ( seconds - 45 + 59 ) / 60;
    if ( minutes <= 45 ) {
        timer.setInterval( _MINUTE );
        return i18np( "about 1 minute ago", "about %1 minutes ago", minutes );
    }

    int hours = ( seconds - 45 * 60 + 3599 ) / 3600;
    if ( hours <= 18 ) {
        timer.setInterval( _MINUTE * 15 );
        return i18np( "about 1 hour ago", "about %1 hours ago", hours );
    }

    timer.setInterval( _HOUR );
    int days = ( seconds - 18 * 3600 + 24 * 3600 - 1 ) / ( 24 * 3600 );
    return i18np( "about 1 day ago", "about %1 days ago", days );
}

void StatusWidget::setUserImage( const QString & imgPath )
{
    lblImage->setPixmap( QPixmap( imgPath ) );
}

void StatusWidget::requestReply()
{
    kDebug();
    emit sigReply( mCurrentStatus.user.screenName, mCurrentStatus.statusId );
}

QString StatusWidget::generateSign()
{
    if ( mCurrentAccount->serviceType() == Account::Identica ) {
        signPrefix = "<b><a href=\"http://identi.ca/" + mCurrentStatus.user.screenName + "\" title=\"" +
                     mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName + "</a> - </b> ";
        signPrefix += "<a href=\"http://identi.ca/notice/" + QString::number( mCurrentStatus.statusId ) + "\" title=\"" +
                      mCurrentStatus.creationDateTime.toString() + "\">";
        if ( !mCurrentStatus.isDMessage ) {
            signPostfix = " - " + mCurrentStatus.source;
            if ( mCurrentStatus.replyToStatusId > 0 ) {
                signPostfix += " - <a href=\"http://identi.ca/notice/" +
                               QString::number( mCurrentStatus.replyToStatusId ) + "\">in reply to</a>";
            }
        }
    } else {
        signPrefix = "<b><a href=\"http://twitter.com/" + mCurrentStatus.user.screenName + "\" title=\"" +
                     mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName + "</a> - </b> ";
        signPrefix += "<a href=\"http://twitter.com/" + mCurrentStatus.user.screenName + "/statuses/" +
                      QString::number( mCurrentStatus.statusId ) + "\" title=\"" +
                      mCurrentStatus.creationDateTime.toString() + "\">";
        if ( !mCurrentStatus.isDMessage ) {
            signPostfix = " - " + mCurrentStatus.source;
            if ( mCurrentStatus.replyToStatusId > 0 ) {
                signPostfix += " - <a href=\"http://twitter.com/" + mCurrentStatus.replyToUserScreenName + "/statuses/"
                               + QString::number( mCurrentStatus.replyToStatusId ) + "\">in reply to</a>";
            }
        }
    }
    return regenerateSign();
}

QString StatusWidget::regenerateSign()
{
    QString sign = signPrefix;
    sign += formatDateTime( mCurrentStatus.creationDateTime ) + "</a>";
    sign += signPostfix;
    return sign;
}

void StatusWidget::updateSign()
{
    lblSign->setText( regenerateSign() );
}

void StatusWidget::requestDestroy()
{
    emit sigDestroy( mCurrentStatus.statusId );
}

QString StatusWidget::prepareStatus( const QString &text )
{
    if(text.isEmpty() && mCurrentAccount->serviceType() == Account::Identica){
        Backend *b = new Backend(new Account(*mCurrentAccount), this);
        connect(b, SIGNAL(singleStatusReceived( uint, Status )),
                 this, SLOT(missingStatusReceived( uint, Status )));
        b->requestSingleStatus(mCurrentStatus.statusId);
        return text;
    }
    QString s = text;
    int i = 0, j = 0;
    ///TODO Adding smile support!
    /*  if(Settings::isSmilysEnabled()){
            while((j = s.indexOf(':', i)) != -1){
                if(s[j+1]==')' && s[j+2]==')')
                    ;
                else
                    switch(s[j+1]){
                        case 'D':
                            break;
                        case ')':
                            break;
                        case '(':
                            break;
                        case 'o':
                        case 'O':
                            break;
                        case '*':
                        case 'x':
                            break;
                        case '|':
                            break;
                        case '/':
                            break;

                    };
            }
        }*/

    s.replace( " www.", " http://www." );
    if ( s.startsWith( "www." ) ) 
        s.prepend( "http://" );
    QString t = "";
    i = j = 0;
    while (( j = s.indexOf( QRegExp( "(https?://)" ), i ) ) != -1 ) {
        t += s.mid( i, j - i );
        int k = s.indexOf( ' ', j );
        if ( k == -1 ) k = s.length();
        QString url = s.mid( j, k - j );
        t += "<a href='" + url + "'>" + url + "</a>";
        i = k;
    }
    t += s.mid( i );

    i = j = 0;
    s = t;
    t.clear();
    QRegExp usrRx( "(^|\\s)(@\\w+)(\\s|$|\\b)", Qt::CaseInsensitive );
    while (( j = usrRx.indexIn( s , i ) ) != -1 ) {
        t += s.mid( i, j - i );
        int k = usrRx.cap(2).length();
        QString username = usrRx.cap(2).remove(0, 1);
        QString url;
        url = mCurrentAccount->userProfilePath();
        t += " @<a href='" + url + "' title='" + url + "'>" + username + "</a> ";
        i = k + j + 1;
    }
    t += s.mid( i );

    if ( mCurrentAccount->serviceType() == Account::Identica ) {
        ///To cover Identica TAGs:
        i = j = 0;
        s = t;
        t.clear();
        QRegExp tagRx( "(^|\\s)(#\\w+)(\\s|$)", Qt::CaseInsensitive );
        while( ( j = tagRx.indexIn( s, i ) ) != -1 ){
//             kDebug()<<tagRx.capturedTexts()<<i<<j;
            t += s.mid( i, j - i );
            int k = tagRx.cap(2).length();
            QString tag = tagRx.cap(2).remove(0, 1);
            QString url = "http://identi.ca/tag/" + tag;
            t += " #<a href='" + url + "' title='" + url + "'>" + tag + "</a>";
            i += k + j + 1;
        }
        t += s.mid( i );

        ///To cover Identica Groups:
        i = j = 0;
        s = t;
        t.clear();
        QRegExp grpRx( "(^|\\s)(!\\w+)(\\s|$|\\b)", Qt::CaseInsensitive );
        while (( j = grpRx.indexIn( s, i ) ) != -1 ) {
//             kDebug()<<grpRx.capturedTexts()<<i<<j;
            t += s.mid( i, j - i );
            int k = grpRx.cap(2).length();
            QString group = grpRx.cap(2).remove(0, 1);
            QString url = "http://identi.ca/group/" + group;
            t += " !<a href='" + url + "' title='" + url + "'>" + group + "</a> ";
            i = k + j + 1;
        }
        t += s.mid( i );
    }
    if ( mCurrentAccount->direction() == Qt::RightToLeft ) {
        s = "<div dir='rtl'>";
    } else {
        s = "<div dir='ltr'>";
    }
    s += t;
    s += "</div>";
    return s;
}

void StatusWidget::setUnread( Notify notifyType )
{
    mIsReaded = false;
    QColor backColor;
    QString sheet;
    if ( Settings::isCustomUi() ) {
        backColor = Settings::newStatusBackColor();
        sheet += " color:" + Settings::newStatusForeColor().name() + ';';
    } else {
        backColor = this->palette().window().color();
        backColor.setBlue( backColor.blue() + COLOROFFSET );
        backColor.setGreen( backColor.green() + COLOROFFSET );
        backColor.setRed( backColor.red() + COLOROFFSET );
    }
    sheet += "background-color: rgb(" + QString::number( backColor.red() ) + ','
             + QString::number( backColor.green() ) + ',' + QString::number( backColor.blue() ) + ");";
    this->setStyleSheet( sheet );

    if ( notifyType == WithNotify ) {
        QString iconUrl = MediaManager::self()->getImageLocalPathIfExist( mCurrentStatus.user.profileImageUrl );
        QString name = mCurrentStatus.user.screenName;
        QString msg = mCurrentStatus.content;
        if ( Settings::notifyType() == 1 ) {
            KNotification *notify = new KNotification( "new-status-arrived", parentWidget() );
            notify->setText( QString( "<qt><b>" + name + ":</b><br/>" + msg + "</qt>" ) );
            notify->setPixmap( QPixmap( iconUrl ) );
            notify->setFlags( KNotification::RaiseWidgetOnActivation | KNotification::Persistent );
            notify->setActions( i18n( "Reply" ).split( ',' ) );
            connect( notify, SIGNAL( action1Activated() ), this , SLOT( requestReply() ) );
            notify->sendEvent();
            QTimer::singleShot( Settings::notifyInterval()*1000, notify, SLOT( close() ) );
        } else if ( Settings::notifyType() == 2 ) {
            QString libnotifyCmd = QString( "notify-send -t " ) + QString::number( Settings::notifyInterval() * 1000 )
            + QString( " -u low -i " + iconUrl + " \"" ) + name + QString( "\" \"" ) + msg + QString( "\"" );
            QProcess::execute( libnotifyCmd );
        }
    }
}

void StatusWidget::setRead()
{
    mIsReaded = true;
    QColor backColor;
    QString sheet;
    if ( Settings::isCustomUi() ) {
        backColor = Settings::defaultBackColor();
        sheet += " color:" + Settings::defaultForeColor().name() + ';';
    } else {
        backColor = this->palette().window().color();
        backColor.setBlue( backColor.blue() - COLOROFFSET );
        backColor.setGreen( backColor.green() - COLOROFFSET );
        backColor.setRed( backColor.red() - COLOROFFSET );
    }
    sheet += "background-color: rgb(" + QString::number( backColor.red() ) + ','
             + QString::number( backColor.green() ) + ", " + QString::number( backColor.blue() ) + ");";
    this->setStyleSheet( sheet );
}

void StatusWidget::setUiStyle()
{
    QColor backColor;
    QString sheet;
    if ( Settings::isCustomUi() ) {
        backColor = Settings::defaultBackColor();
        sheet += " color:" + Settings::defaultForeColor().name() + ';';
    } else {
        backColor = this->palette().window().color();
    }
    sheet += "background-color: rgb(" + QString::number( backColor.red() ) + ','
             + QString::number( backColor.green() ) + ',' + QString::number( backColor.blue() ) + ");";
    this->setStyleSheet( sheet );
}

void StatusWidget::updateFavoriteUi()
{
    if ( mCurrentStatus.isFavorited ) {
        btnFavorite->setChecked( true );
    } else {
        btnFavorite->setChecked( false );
    }
}

bool StatusWidget::isReaded()
{
    return mIsReaded;
}

void StatusWidget::setUserImage( const QPixmap * image )
{
    lblImage->setPixmap( *image );
}

void StatusWidget::setUserImage()
{
    connect( MediaManager::self(), SIGNAL( imageFetched( const QString &, const QString & ) ),
             this, SLOT( userImageLocalPathFetched( const QString&, const QString& ) ) );
    MediaManager::self()->getImageLocalPathDownloadAsyncIfNotExists( mCurrentAccount->serviceName() +
            mCurrentStatus.user.screenName , mCurrentStatus.user.profileImageUrl );
}

void StatusWidget::userImageLocalPathFetched( const QString &remotePath, const QString &localPath )
{
    if ( remotePath == mCurrentStatus.user.profileImageUrl ) {
        lblImage->setPixmap( QPixmap( localPath ) );
        disconnect( MediaManager::self(), SIGNAL( imageFetched( const QString &, const QString & ) ),
                    this, SLOT( userImageLocalPathFetched( const QString&, const QString& ) ) );
    }
}

void StatusWidget::missingStatusReceived( uint statusId, Status status )
{
//     if( statusId == mCurrentStatus.statusId ){
        mCurrentStatus = status;
        lblStatus->setHtml( prepareStatus( mCurrentStatus.content ) );
        sender()->deleteLater();
//     }
}

#include "statuswidget.moc"
