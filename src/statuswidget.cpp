/*
    This file is part of choqoK, the KDE micro-blogging client

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
#include <KNotification>
#include <QProcess>

#include <QInputDialog>

#include <KDE/KLocale>
#include <QLayout>

#define _15SECS 15000
#define _MINUTE 60000
#define _HOUR (60 * _MINUTE)

const QString StatusWidget::baseText("<table dir=\"%1\" width=\"100%\"><tr><td rowspan=\"2\"\
width=\"48\">%2</td><td>%3</td></tr><tr><td style=\"font-size:small;\" align=\"right\">%4</td></tr></table>");
const QString StatusWidget::baseStyle("QFrame.StatusWidget {border: 1px solid rgb(150,150,150);\
border-radius:5px;}\
QFrame.StatusWidget[read=false] {color: %1; background-color: %2}\
QFrame.StatusWidget[read=true] {color: %3; background-color: %4}");

QString StatusWidget::style;

void StatusWidget::setStyle(const QColor& color, const QColor& back, const QColor& read, const QColor& readBack) {
  style = baseStyle.arg(getColorString(color),getColorString(back),getColorString(read),getColorString(readBack));
}

StatusWidget::StatusWidget( const Account *account, QWidget *parent )
        : KTextBrowser( parent ),mIsRead(true),mCurrentAccount(account)
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setupUi();

    this->setOpenExternalLinks( true );

    timer.start( _MINUTE );
    connect( &timer, SIGNAL( timeout() ), this, SLOT( updateSign() ) );
}

void StatusWidget::setupUi() {
    QVBoxLayout * vl = new QVBoxLayout(this);
    QHBoxLayout * l = new QHBoxLayout();

    l->setSpacing(0);
    l->setMargin(0);
    vl->setSpacing(0);
    vl->setMargin(0);

    vl->addStretch();
    vl->addLayout(l);

    btnReply = getButton( "btnReply",i18nc( "@info:tooltip", "Reply" ), "edit-undo" );
    btnRemove = getButton( "btnRemove",i18nc( "@info:tooltip", "Remove" ), "edit-delete" );
    btnFavorite = getButton( "btnFavorite",i18nc( "@info:tooltip", "Favorite" ), "rating" );
    btnFavorite->setCheckable(true);

    l->addWidget(btnReply);
    l->addWidget(btnRemove);
    l->addWidget(btnFavorite);
    l->addStretch();

    connect( btnReply, SIGNAL( clicked( bool ) ), this, SLOT( requestReply() ) );
    connect( btnFavorite, SIGNAL( clicked( bool ) ), this, SLOT( setFavorite( bool ) ) );
    connect( btnRemove, SIGNAL( clicked( bool ) ), this, SLOT( requestDestroy() ) );

    connect(this,SIGNAL(textChanged()),this,SLOT(setHeight()));
}

void StatusWidget::enterEvent(QEvent* event) {
  if ( !mCurrentStatus.isDMessage )
      btnFavorite->setVisible( true );
  if ( mCurrentStatus.user.userId != mCurrentAccount->userId() )
      btnReply->setVisible( true );
  else
      btnRemove->setVisible( true );

  QWidget::enterEvent(event);
}

void StatusWidget::leaveEvent(QEvent* event) {
  btnRemove->setVisible(false);
  btnFavorite->setVisible(false);
  btnReply->setVisible(false);

  QWidget::leaveEvent(event);
}


KPushButton * StatusWidget::getButton(const QString & objName, const QString & toolTip, const QString & icon) {
    KPushButton * button = new KPushButton(KIcon(icon),QString());
    button->setObjectName(objName);
    button->setToolTip(toolTip);
    button->setIconSize(QSize(8,8));
    button->setMinimumSize(QSize(20, 20));
    button->setMaximumSize(QSize(20, 20));
    button->setFlat(true);
    button->setVisible(false);
    return button;
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
    mDir = ( mCurrentAccount->direction() == Qt::RightToLeft ) ? "rtl" : "ltr";
    mStatus = prepareStatus(mCurrentStatus.content);
    mSign = generateSign();
    setUserImage();
    updateSign();
    setUiStyle();
    updateFavoriteUi();
}

void StatusWidget::setHeight() {
    document()->setTextWidth(width()-2);
    QSize s = document()->size().toSize();
    setMinimumHeight(s.height()+2);
    setMaximumHeight(s.height()+2);
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
        return i18np( "1 sec ago", "%1 secs ago", seconds );
    }

    int minutes = ( seconds - 45 + 59 ) / 60;
    if ( minutes <= 45 ) {
        timer.setInterval( _MINUTE );
        return i18np( "1 min ago", "%1 mins ago", minutes );
    }

    int hours = ( seconds - 45 * 60 + 3599 ) / 3600;
    if ( hours <= 18 ) {
        timer.setInterval( _MINUTE * 15 );
        return i18np( "1 hour ago", "%1 hours ago", hours );
    }

    timer.setInterval( _HOUR );
    int days = ( seconds - 18 * 3600 + 24 * 3600 - 1 ) / ( 24 * 3600 );
    return i18np( "1 day ago", "%1 days ago", days );
}

void StatusWidget::requestReply()
{
    kDebug();
    emit sigReply( mCurrentStatus.user.screenName, mCurrentStatus.statusId, currentStatus().isDMessage );
}

QString StatusWidget::generateSign()
{
    QString sign;
    if ( mCurrentAccount->serviceType() == Account::Identica ) {
        sign = "<b><a href=\"http://identi.ca/" + mCurrentStatus.user.screenName + "\" title=\"" +
                     mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName + "</a> - </b>";
        sign += "<a href=\"http://identi.ca/notice/" + QString::number( mCurrentStatus.statusId ) +
        "\" title=\"" + mCurrentStatus.creationDateTime.toString() + "\">%1</a>";
        if ( !mCurrentStatus.isDMessage ) {
            sign += " - " + mCurrentStatus.source;
            if ( mCurrentStatus.replyToStatusId > 0 ) {
                QString link = "http://identi.ca/notice/" + QString::number( mCurrentStatus.replyToStatusId );
                sign += " - <a href=\"" + link + "\" title=\"" + link + "\">in reply to</a>";
            }
        }
    } else {
        sign = "<b><a href=\"http://twitter.com/" + mCurrentStatus.user.screenName + "\" title=\"" +
                     mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName + "</a> - </b>";
        sign += "<a href=\"http://twitter.com/" + mCurrentStatus.user.screenName + "/statuses/" +
                     QString::number( mCurrentStatus.statusId ) + "\" title=\"" +
                     mCurrentStatus.creationDateTime.toString() + "\">%1</a>";
        if ( !mCurrentStatus.isDMessage ) {
            sign += " - " + mCurrentStatus.source;
            if ( mCurrentStatus.replyToStatusId > 0 ) {
                QString link = "http://twitter.com/" + mCurrentStatus.replyToUserScreenName + "/statuses/"
                + QString::number( mCurrentStatus.replyToStatusId );
                sign += " - <a href=\"" + link + "\" title=\"" + link + "\">in reply to</a>";
            }
        }
    }
    return sign;
}

void StatusWidget::updateSign()
{ 
    setHtml( baseText.arg( mDir, mImage, mStatus, mSign.arg( formatDateTime( mCurrentStatus.creationDateTime ) ) ) );
}

void StatusWidget::requestDestroy()
{
    emit sigDestroy( mCurrentStatus.statusId );
}

QString StatusWidget::prepareStatus( const QString &text )
{
    if(text.isEmpty() && mCurrentAccount->serviceType() == Account::Identica){
        Backend *b = new Backend(new Account(*mCurrentAccount), this);
        connect(b, SIGNAL(singleStatusReceived( Status )),
                 this, SLOT(missingStatusReceived( Status )));
        b->requestSingleStatus(mCurrentStatus.statusId);
	return text;
    }
    QString status = text;
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

    status.replace( '<', "&lt;" );
    status.replace( '>', "&gt;" );
    status.replace( " www.", " http://www." );
    if ( status.startsWith( QLatin1String("www.") ) ) 
        status.prepend( "http://" );
    status.replace(QRegExp("(https?://[^ ]+)"),"<a href='\\1' title='\\1'>\\1</a>");

    QString urlPrefix;
    if( mCurrentAccount->serviceType() == Account::Identica )
	urlPrefix = "http://identi.ca/";
    else
	urlPrefix = "http://twitter.com/";
    status.replace(QRegExp("@(\\w+)(\\s|$|\\b)", Qt::CaseInsensitive),"@<a href='"+urlPrefix+"\\1'>\\1</a>\\2");

    if ( mCurrentAccount->serviceType() == Account::Identica ) {
      status.replace(QRegExp( "#(\\w+)(\\s|$|\\b)", Qt::CaseInsensitive ),"#<a href='"+urlPrefix+"tag/\\1'>\\1</a>\\2");
      status.replace(QRegExp( "!(\\w+)(\\s|$|\\b)", Qt::CaseInsensitive ),"!<a href='"+urlPrefix+"group/\\1'>\\1</a>\\2");
    }
    return status;
}

QString StatusWidget::getColorString(const QColor& color) {
  return "rgb(" + QString::number(color.red()) + ',' + QString::number(color.green()) + ',' +
  QString::number(color.blue()) + ')';
}

void StatusWidget::setUnread( Notify notifyType )
{
    mIsRead = false;
    setUiStyle();

    if ( notifyType == WithNotify ) {
        QString iconUrl = MediaManager::self()->getImageLocalPathIfExist( mCurrentStatus.user.profileImageUrl );
        QString name = mCurrentStatus.user.screenName;
        QString msg = mCurrentStatus.content;
        if ( Settings::notifyType() == SettingsBase::KNotify ) {
            KNotification *notify = new KNotification( "new-status-arrived", parentWidget() );
            notify->setText( QString( "<qt><b>" + name + ":</b><br/>" + msg + "</qt>" ) );
            notify->setPixmap( QPixmap( iconUrl ) );
            notify->setFlags( KNotification::RaiseWidgetOnActivation | KNotification::Persistent );
            notify->setActions( i18n( "Reply" ).split( ',' ) );
            connect( notify, SIGNAL( action1Activated() ), this , SLOT( requestReply() ) );
            notify->sendEvent();
            QTimer::singleShot( Settings::notifyInterval()*1000, notify, SLOT( close() ) );
        } else if ( Settings::notifyType() == SettingsBase::LibNotify ) {
            QString libnotifyCmd = QString( "notify-send -t " ) + QString::number( Settings::notifyInterval() * 1000 )
            + QString( " -u low -i " + iconUrl + " \"" ) + name + QString( "\" \"" ) + msg + QString( "\"" );
            QProcess::execute( libnotifyCmd );
        }
    }
}

void StatusWidget::setRead(bool read)
{
    mIsRead = read;
    setUiStyle();
}

void StatusWidget::setUiStyle()
{
    this->setStyleSheet( style );
}

void StatusWidget::updateFavoriteUi()
{
  btnFavorite->setChecked(mCurrentStatus.isFavorited);
}

bool StatusWidget::isRead()
{
    return mIsRead;
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
      mImage = "<img src='"+localPath+"' width=\"48\" height=\"48\" />";
      updateSign();
        disconnect( MediaManager::self(), SIGNAL( imageFetched( const QString &, const QString & ) ),
                    this, SLOT( userImageLocalPathFetched( const QString&, const QString& ) ) );
    }
}

void StatusWidget::missingStatusReceived( Status status )
{
    if( mCurrentStatus.statusId == mCurrentStatus.statusId ){
        mCurrentStatus = status;
        updateUi();
        sender()->deleteLater();
    }
}


void StatusWidget::resizeEvent(QResizeEvent* event) {
  setHeight();
  KTextBrowser::resizeEvent(event);
}

#include "statuswidget.moc"
