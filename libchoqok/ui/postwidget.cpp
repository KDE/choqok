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
#include "postwidget.h"
#include <qboxlayout.h>
#include <KLocale>
#include <KPushButton>
#include <QGridLayout>
#include <KDebug>
#include <mediamanager.h>

static const int _15SECS = 15000;
static const int _MINUTE = 60000;
static const int _HOUR = 60*_MINUTE;

using namespace Choqok;
using namespace Choqok::UI;

const QString PostWidget::baseText ( "<table width=\"100%\"><tr><td rowspan=\"2\"\
width=\"48\">%1</td><td><p>%2</p></td></tr><tr><td style=\"font-size:small;\" align=\"right\">%3</td></tr></table>");
const QString PostWidget::baseStyle ("KTextBrowser {border: 1px solid rgb(150,150,150);\
border-radius:5px;} KTextBrowser {color:%1; background-color:%2}\
KPushButton{border:0px}");
const QRegExp PostWidget::mUrlRegExp("((ftps?|https?)://[^\\s<>\"]+[^!,\\.\\s<>'\"\\)\\]])");
QString PostWidget::readStyle;
QString PostWidget::unreadStyle;

PostWidget::PostWidget( Account* account, const Choqok::Post& post, QWidget* parent/* = 0*/ )
    :KTextBrowser(parent), mCurrentPost(post), mCurrentAccount(account), mRead(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setupUi();
    mTimer.start( _MINUTE );
    connect( &mTimer, SIGNAL( timeout() ), this, SLOT( updateUi()) );
}

PostWidget::~PostWidget()
{
}

Account* PostWidget::currentAccount()
{
    return mCurrentAccount;
}

QString PostWidget::generateSign()
{
    QString ss;
    ss = "<b><a href='"+ mCurrentAccount->microblog()->profileUrl( mCurrentPost.author.userName ) +"' title=\"" +
    mCurrentPost.author.description + "\">" + mCurrentPost.author.userName +
    "</a> - </b>";

    ss += "<a href=\"" + mCurrentPost.link +
    "\" title=\"" + mCurrentPost.creationDateTime.toString() + "\">%1</a>";

    if( !mCurrentPost.source.isNull() )
        ss += " - " + mCurrentPost.source;

    return ss;
}

void PostWidget::setupUi()
{
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    buttonsLayout = new QGridLayout(this);
    buttonsLayout->setRowStretch(0,100);
    buttonsLayout->setColumnStretch(5,100);
    buttonsLayout->setMargin(0);
    buttonsLayout->setSpacing(0);

    this->setLayout(buttonsLayout);
    connect(this,SIGNAL(textChanged()),this,SLOT(setHeight()));

    document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"),
                             MediaManager::self()->defaultImage() );
                             mImage = "<img src=\"img://profileImage\" title=\""+ mCurrentPost.author.realName + "\" width=\"48\" height=\"48\" />";
}

void PostWidget::initUi()
{
    KPushButton *btnResend = addButton("btnResend", i18nc( "@info:tooltip", "ReSend" ), "retweet" );
    connect(btnResend, SIGNAL(clicked(bool)), SLOT(slotResendPost()));

    if(mCurrentAccount->username() == mCurrentPost.author.userName) {
        KPushButton *btnRemove = addButton("btnRemove", i18nc( "@info:tooltip", "Remove" ), "edit-delete" );
        connect(btnRemove, SIGNAL(clicked(bool)), SLOT(removeCurrentPost()));
    }

    mImage = "<img src=\"img://profileImage\" title=\""+ mCurrentPost.author.realName +"\" width=\"48\" height=\"48\" />";
    mContent = prepareStatus(mCurrentPost.content);
    mSign = generateSign();
    setupAvatar();
    setDirection();
    setUiStyle();
    updateUi();
}

void PostWidget::updateUi()
{
    setHtml( baseText.arg( mImage, mContent, mSign.arg( formatDateTime( mCurrentPost.creationDateTime ) ) ) );
}

void PostWidget::setStyle(const QColor& color, const QColor& back, const QColor& read, const QColor& readBack)
{
    unreadStyle = baseStyle.arg( getColorString(color), getColorString(back) );
    readStyle = baseStyle.arg( getColorString(read), getColorString(readBack) );
}

QString PostWidget::getColorString(const QColor& color)
{
    return "rgb(" + QString::number(color.red()) + ',' + QString::number(color.green()) + ',' +
    QString::number(color.blue()) + ')';
}

KPushButton * PostWidget::addButton(const QString & objName, const QString & toolTip, const QString & icon)
{
    KPushButton * button = new KPushButton(KIcon(icon), QString(), this);
    button->setObjectName(objName);
    button->setToolTip(toolTip);
    button->setIconSize(QSize(16,16));
    button->setMinimumSize(QSize(20, 20));
    button->setMaximumSize(QSize(20, 20));
    button->setFlat(true);
    button->setVisible(false);
    button->setCursor(Qt::PointingHandCursor);

    mUiButtons.append(button);
    buttonsLayout->addWidget( button, 1, mUiButtons.count() );
    return button;
}

KPushButton * PostWidget::addButton(const QString & objName, const QString & toolTip, const KIcon & icon)
{
    KPushButton * button = new KPushButton(icon, QString(), this);
    button->setObjectName(objName);
    button->setToolTip(toolTip);
    button->setIconSize(QSize(16,16));
    button->setMinimumSize(QSize(20, 20));
    button->setMaximumSize(QSize(20, 20));
    button->setFlat(true);
    button->setVisible(false);
    button->setCursor(Qt::PointingHandCursor);

    mUiButtons.append(button);
    buttonsLayout->addWidget( button, 1, mUiButtons.count() );
    return button;
}

const Post &PostWidget::currentPost() const
{
    return mCurrentPost;
}

void PostWidget::setRead(bool read/* = true*/)
{
    if(currentAccount()->username() == currentPost().author.userName && !mRead) {
        mRead = true; ///Always Set own posts as read.
        setUiStyle();
    } else if( mRead != read ) {
        mRead = read;
        setUiStyle();
    }
}

bool PostWidget::isRead() const
{
    return mRead;
}

void PostWidget::setUiStyle()
{
    if(mRead)
        setStyleSheet(readStyle);
    else
        setStyleSheet(unreadStyle);
}

void PostWidget::setHeight()
{
    document()->setTextWidth(width()-2);
    int h = document()->size().toSize().height()+2;
    setMinimumHeight(h);
    setMaximumHeight(h);
}

void PostWidget::mousePressEvent(QMouseEvent* ev)
{
    if(!isRead()) {
        kDebug();
        if(mCurrentPost.author.userName != mCurrentAccount->username()) {
            kDebug()<<"Emitting postReaded()";
            emit postReaded();
        }
        setRead();
    }
    QTextBrowser::mousePressEvent(ev);
}

void PostWidget::resizeEvent ( QResizeEvent * event )
{
    setHeight();
    KTextBrowser::resizeEvent(event);
}

void PostWidget::enterEvent ( QEvent * event )
{
    foreach(KPushButton *btn, buttons()){
        btn->show();
    }
    KTextBrowser::enterEvent(event);
}

void PostWidget::leaveEvent ( QEvent * event )
{
    foreach(KPushButton *btn, buttons()){
        btn->hide();
    }
    KTextBrowser::enterEvent(event);
}

QString PostWidget::prepareStatus( const QString &txt )
{
    QString text = txt;
    text.replace( '<', "&lt;" );
    text.replace( '>', "&gt;" );
    text.replace( " www.", " http://www." );
    if ( text.startsWith( QLatin1String("www.") ) )
        text.prepend( "http://" );
    text.replace(mUrlRegExp,"<a href='\\1' title='\\1'>\\1</a>");

    //TODO Add support for PLUGINS! e.g. Smilie, UnTiny, ...

    return text;
}

void PostWidget::setDirection()
{
    QString txt = mCurrentPost.content;
    QChar re(0x267B);
    txt.remove(re);
    txt.remove(QRegExp("RT|RD"));
    if(txt.startsWith('@'))
        txt.remove(QRegExp("@([^\\s\\W]+)"));
    if(txt.startsWith('#'))
        txt.remove(QRegExp("#([^\\s\\W]+)"));
    if(txt.startsWith('!'))
        txt.remove(QRegExp("!([^\\s\\W]+)"));
    txt.prepend(' ');
    if( txt.isRightToLeft() ) {
        QTextOption options(document()->defaultTextOption());
        options.setTextDirection( Qt::RightToLeft );
        document()->setDefaultTextOption(options);
    }
}

QString PostWidget::formatDateTime( const QDateTime& time )
{
    int seconds = time.secsTo( QDateTime::currentDateTime() );
    if ( seconds <= 15 ) {
        mTimer.setInterval( _15SECS );
        return i18n( "Just now" );
    }

    if ( seconds <= 45 ) {
        mTimer.setInterval( _15SECS );
        return i18np( "1 sec ago", "%1 secs ago", seconds );
    }

    int minutes = ( seconds - 45 + 59 ) / 60;
    if ( minutes <= 45 ) {
        mTimer.setInterval( _MINUTE );
        return i18np( "1 min ago", "%1 mins ago", minutes );
    }

    int hours = ( seconds - 45 * 60 + 3599 ) / 3600;
    if ( hours <= 18 ) {
        mTimer.setInterval( _MINUTE * 15 );
        return i18np( "1 hour ago", "%1 hours ago", hours );
    }

    mTimer.setInterval( _HOUR );
    int days = ( seconds - 18 * 3600 + 24 * 3600 - 1 ) / ( 24 * 3600 );
    return i18np( "1 day ago", "%1 days ago", days );
}

void PostWidget::removeCurrentPost()
{
    connect(mCurrentAccount->microblog(), SIGNAL(postRemoved(Account*,Post*)),
            SLOT(slotCurrentPostRemoved(Account*,Post*)) );
    connect( mCurrentAccount->microblog(),
             SIGNAL(errorPost(Account*,Choqok::MicroBlog::ErrorType,QString,const Post*)),
             this, SLOT(slotPostError(Account*,Choqok::MicroBlog::ErrorType,QString,const Post*)) );
    mCurrentAccount->microblog()->removePost(mCurrentAccount, &mCurrentPost);
}

void PostWidget::slotCurrentPostRemoved( Account* theAccount, Post* post )
{
    if( theAccount == currentAccount() && post == &mCurrentPost )
        this->close();
}

void PostWidget::slotResendPost()
{
    QChar re(0x267B);
    QString text =  QString(re) + " @" + currentPost().author.userName + ": " + currentPost().content;
    emit resendPost(text);
}

void PostWidget::setupAvatar()
{
    QPixmap *pix = MediaManager::self()->fetchImage( mCurrentPost.author.profileImageUrl,
                                      MediaManager::Async );
    if(pix)
        avatarFetched(mCurrentPost.author.profileImageUrl, *pix);
    else {
        connect( MediaManager::self(), SIGNAL( imageFetched(QString,QPixmap)),
                this, SLOT(avatarFetched(QString, QPixmap) ) );
        connect( MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                this, SLOT(avatarFetchError(QString,QString)) );
    }
}

void PostWidget::avatarFetched(const QString& remoteUrl, const QPixmap& pixmap)
{
    if ( remoteUrl == mCurrentPost.author.profileImageUrl ) {
        QString url = "img://profileImage";
        document()->addResource( QTextDocument::ImageResource, url, pixmap );
        updateUi();
        disconnect( MediaManager::self(), SIGNAL( imageFetched(QString,QPixmap)),
                    this, SLOT(avatarFetched(QString, QPixmap) ) );
        disconnect( MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                    this, SLOT(avatarFetchError(QString,QString))  );
    }
}

void PostWidget::avatarFetchError(const QString& remoteUrl, const QString& errMsg)
{
    Q_UNUSED(errMsg);
    if( remoteUrl == mCurrentPost.author.profileImageUrl ){
        ///Avatar fetching is failed! but will not disconnect to get the img if it fetches later!
        QString url = "img://profileImage";
        document()->addResource( QTextDocument::ImageResource, url, KIcon("image-missing").pixmap(48) );
        updateUi();
    }
}

QList< KPushButton* >& PostWidget::buttons()
{
    return mUiButtons;
}

void PostWidget::slotPostError(Account* theAccount, MicroBlog::ErrorType error,
                               const QString& errorMessage, const Choqok::Post* post)
{
    if( theAccount == currentAccount() && post == &mCurrentPost) {
        kError()<<errorMessage;
    }
}


#include "postwidget.moc"
