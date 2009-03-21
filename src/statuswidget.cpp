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

#include "mainwindow.h"

#include <KMenu>

#include <KDE/KLocale>
#include <QLayout>
#include <KToolInvocation>
#include "identicasearch.h"
#include "twittersearch.h"
#include <KAction>

static const int _15SECS = 15000;
static const int _MINUTE = 60000;
static const int _HOUR = 60*_MINUTE;

const QString StatusWidget::baseText("<table width=\"100%\"><tr><td rowspan=\"2\"\
 width=\"48\">%2</td><td>%3</td></tr><tr><td style=\"font-size:small;\" align=\"right\">%4</td></tr></table>");
const QString StatusWidget::baseStyle("QFrame.StatusWidget {border: 1px solid rgb(150,150,150);\
border-radius:5px;}\
QFrame.StatusWidget[read=false] {color: %1; background-color: %2}\
QFrame.StatusWidget[read=true] {color: %3; background-color: %4}");

QString StatusWidget::style;

QRegExp StatusWidget::mUrlRegExp("(https?://[^\\s<>'\"]+[^!,\\.\\s<>'\"\\]])"); // "borrowed" from microblog plasmoid
QRegExp StatusWidget::mUserRegExp("([\\s]|^)@([^\\s\\W]+)", Qt::CaseInsensitive);
QRegExp StatusWidget::mHashtagRegExp("([\\s]|^)#([^\\s\\W]+)", Qt::CaseInsensitive);
QRegExp StatusWidget::mGroupRegExp("([\\s]|^)!([^\\s\\W]+)", Qt::CaseInsensitive);

void StatusWidget::setStyle(const QColor& color, const QColor& back, const QColor& read, const QColor& readBack)
{
  style = baseStyle.arg(getColorString(color),getColorString(back),getColorString(read),getColorString(readBack));
}

StatusWidget::StatusWidget( const Account *account, QWidget *parent )
        : KTextBrowser( parent ),mIsRead(true),mCurrentAccount(account),isBaseStatusShowed(false)
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    setupUi();
    setOpenLinks(false);

    timer.start( _MINUTE );
    connect( &timer, SIGNAL( timeout() ), this, SLOT( updateSign() ) );
    connect(this,SIGNAL(anchorClicked(QUrl)),this,SLOT(checkAnchor(QUrl)));
}

void StatusWidget::checkAnchor(const QUrl & url)
{
  QString scheme = url.scheme();
  Account::Service s = mCurrentAccount->serviceType();
  int type = 0;
  if( scheme == "group" && ( s == Account::Identica || s == Account::Laconica ) ) {
    type = IdenticaSearch::ReferenceGroup;
  } else if(scheme == "tag") {
    switch(s) {
    case Account::Identica:
    case Account::Laconica:
      type = IdenticaSearch::ReferenceHashtag;
    break;
    case Account::Twitter:
      type = TwitterSearch::ReferenceHashtag;
    }
  } else if(scheme == "user") {
    KMenu menu;
//     menu.addTitle(i18n("Search"));
    KAction * from = new KAction(KIcon("edit-find-user"),i18n("from %1",url.host()),&menu);
    KAction * to = new KAction(KIcon("meeting-attending"),i18n("replies to %1",url.host()),&menu);
    menu.addAction(from);
    menu.addAction(to);
    QAction * ret;
    KAction *cont;
    switch(s) {
    case Account::Identica:
    case Account::Laconica:
      from->setData(IdenticaSearch::FromUser);
      to->setData(IdenticaSearch::ToUser);
    break;
    case Account::Twitter:
      cont = new KAction(KIcon("user-properties"),i18n("including %1",url.host()),&menu);
      menu.addAction(cont);
      from->setData(TwitterSearch::FromUser);
      to->setData(TwitterSearch::ToUser);
      cont->setData(TwitterSearch::ReferenceUser);
    }
    ret = menu.exec(QCursor::pos());
    if(ret == 0) return;
    type = ret->data().toInt();
  } else if( scheme == "status" ) {
      if(isBaseStatusShowed) {
          updateUi();
          isBaseStatusShowed = false;
          return;
      }
      Backend *b = new Backend(new Account(*mCurrentAccount), this);
      connect( b, SIGNAL( singleStatusReceived( Status ) ),
               this, SLOT( baseStatusReceived(Status) ) );
      b->requestSingleStatus( url.host().toInt() );
      return;
  } else {
    KToolInvocation::invokeBrowser(url.toString());
    return;
  }
  emit sigSearch(type,url.host());
}

void StatusWidget::setupUi()
{
    QGridLayout * buttonGrid = new QGridLayout;

    btnReply = getButton( "btnReply",i18nc( "@info:tooltip", "Reply" ), "edit-undo" );
    btnRemove = getButton( "btnRemove",i18nc( "@info:tooltip", "Remove" ), "edit-delete" );
    btnFavorite = getButton( "btnFavorite",i18nc( "@info:tooltip", "Favorite" ), "rating" );
    btnFavorite->setCheckable(true);

    buttonGrid->setRowStretch(0,100);
    buttonGrid->setColumnStretch(4,100);
    buttonGrid->setMargin(0);
    buttonGrid->setSpacing(0);

    buttonGrid->addWidget(btnReply,1,0);
    buttonGrid->addWidget(btnRemove,1,1);
    buttonGrid->addWidget(btnFavorite,1,2);

    document()->addResource(QTextDocument::ImageResource,QUrl("icon://web"),KIcon("applications-internet").pixmap(8));

    setLayout(buttonGrid);

    connect( btnReply, SIGNAL( clicked( bool ) ), this, SLOT( requestReply() ) );
    connect( btnFavorite, SIGNAL( clicked( bool ) ), this, SLOT( setFavorite( bool ) ) );
    connect( btnRemove, SIGNAL( clicked( bool ) ), this, SLOT( requestDestroy() ) );

    connect(this,SIGNAL(textChanged()),this,SLOT(setHeight()));
}

void StatusWidget::enterEvent(QEvent* event)
{
  if ( !mCurrentStatus.isDMessage )
      btnFavorite->setVisible( true );
  if ( mCurrentStatus.user.userId != mCurrentAccount->userId() )
      btnReply->setVisible( true );
  else
      btnRemove->setVisible( true );
  KTextBrowser::enterEvent(event);
}

void StatusWidget::leaveEvent(QEvent* event)
{
  btnRemove->setVisible(false);
  btnFavorite->setVisible(false);
  btnReply->setVisible(false);

  KTextBrowser::leaveEvent(event);
}

KPushButton * StatusWidget::getButton(const QString & objName, const QString & toolTip, const QString & icon)
{
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
    mStatus = prepareStatus(mCurrentStatus.content);
    mSign = generateSign();
//     if( mCurrentAccount->direction() ) {
    QTextOption options(document()->defaultTextOption());
    options.setTextDirection( mCurrentAccount->direction() );
    document()->setDefaultTextOption(options);
//     }
    setUserImage();
    setUiStyle();
    updateSign();
    updateFavoriteUi();
}

void StatusWidget::setHeight()
{
    document()->setTextWidth(width()-2);
    int h = document()->size().toSize().height()+2;
    setMinimumHeight(h);
    setMaximumHeight(h);
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
    sign = "<b><a href='user://"+mCurrentStatus.user.screenName+"' title=\"" +
    mCurrentStatus.user.description + "\">" + mCurrentStatus.user.screenName +
    "</a> <a href=\"" + mCurrentAccount->homepage() + mCurrentStatus.user.screenName + "\" title=\"" +
                    mCurrentStatus.user.description + "\"><img src=\"icon://web\" /></a> - </b>";
    sign += "<a href=\"" + mCurrentAccount->statusUrl( mCurrentStatus.statusId, mCurrentStatus.user.screenName ) +
    "\" title=\"" + mCurrentStatus.creationDateTime.toString() + "\">%1</a>";
    if ( mCurrentStatus.isDMessage ) {
        if( mCurrentStatus.replyToUserId == mCurrentAccount->userId() ) {
            sign.prepend( "From " );
        } else {
            sign.prepend( "To " );
        }
    } else {
        if( !mCurrentStatus.source.isNull() )
            sign += " - " + mCurrentStatus.source;
        if ( mCurrentStatus.replyToStatusId > 0 ) {
            QString link = mCurrentAccount->statusUrl( mCurrentStatus.replyToStatusId, mCurrentStatus.user.screenName );
            sign += " - <a href='status://" + QString::number( mCurrentStatus.replyToStatusId ) + "'>" +
            i18n("in reply to")+ "</a> <a href=\"" + link + "\"><img src=\"icon://web\" /></a>";
        }
    }
    return sign;
}

void StatusWidget::updateSign()
{
    setHtml( baseText.arg( mImage, mStatus, mSign.arg( formatDateTime( mCurrentStatus.creationDateTime ) ) ) );
}

void StatusWidget::requestDestroy()
{
    emit sigDestroy( mCurrentStatus.statusId );
}

QString StatusWidget::prepareStatus( const QString &text )
{
    if(text.isEmpty() && ( mCurrentAccount->serviceType() == Account::Identica || mCurrentAccount->serviceType() == Account::Laconica ) ){
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
    status.replace(mUrlRegExp,"<a href='\\1' title='\\1'>\\1</a>");

    status.replace(mUserRegExp,"\\1@<a href='user://\\2'>\\2</a> <a href='"+ mCurrentAccount->homepage() +"\\2'><img src=\"icon://web\" /></a>");
    if ( mCurrentAccount->serviceType() == Account::Identica || mCurrentAccount->serviceType() == Account::Laconica ) {
        status.replace(mGroupRegExp,"\\1!<a href='group://\\2'>\\2</a> <a href='"+ mCurrentAccount->homepage() +"group/\\2'><img src=\"icon://web\" /></a>");
        status.replace(mHashtagRegExp,"\\1#<a href='tag://\\2'>\\2</a> <a href='"+ mCurrentAccount->homepage() +"tag/\\1'><img src=\"icon://web\" /></a>");
      } else {
          status.replace(mHashtagRegExp,"\\1#<a href='tag://\\2'>\\2</a>");
    }
    return status;
}

QString StatusWidget::getColorString(const QColor& color)
{
  return "rgb(" + QString::number(color.red()) + ',' + QString::number(color.green()) + ',' +
  QString::number(color.blue()) + ')';
}

void StatusWidget::setUnread( Notify notifyType )
{
    mIsRead = false;

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
    setStyleSheet( style );
}

void StatusWidget::updateFavoriteUi()
{
  btnFavorite->setChecked(mCurrentStatus.isFavorited);
}

bool StatusWidget::isRead() const
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
      mImage = "<img src='"+localPath+"' title='"+ mCurrentStatus.user.name +"' width=\"48\" height=\"48\" />";
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

void StatusWidget::resizeEvent(QResizeEvent* event)
{
  setHeight();
  KTextBrowser::resizeEvent(event);
}

void StatusWidget::baseStatusReceived( Status status )
{
    if(isBaseStatusShowed)
        return;
    isBaseStatusShowed = true;
    QString color;
    if( Settings::isCustomUi() ) {
        color = Settings::defaultForeColor().lighter().name();
    } else {
        color = this->palette().dark().color().name();
    }
    QString baseStatusText = "<p style=\"margin-top:10px; margin-bottom:10px; margin-left:20px;\
    margin-right:20px; -qt-block-indent:0; text-indent:0px\"><span style=\" color:" + color + ";\">";
    baseStatusText += "<b><a href='user://"+ status.user.screenName +"'>" + status.user.screenName + "</a> :</b> ";
    baseStatusText += prepareStatus( status.content ) + "</p>";
    mStatus.prepend( baseStatusText );
    updateSign();
}

#include "statuswidget.moc"
