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
#include "choqokbehaviorsettings.h"
#include <choqokuiglobal.h>
#include "quickpost.h"
#include <KProcess>
#include <KToolInvocation>
#include <KMessageBox>
#include <choqokappearancesettings.h>
#include <kaction.h>
#include <QApplication>
#include <QClipboard>
#include <QContextMenuEvent>
#include <QMenu>
#include <kmenu.h>

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
const QString PostWidget::webIconText("&#9794;");

PostWidget::PostWidget( Account* account, const Choqok::Post& post, QWidget* parent/* = 0*/ )
    :KTextBrowser(parent), mCurrentPost(post), mCurrentAccount(account), mRead(false)
{
    setAttribute(Qt::WA_DeleteOnClose);
    if(currentAccount()->username() == currentPost().author.userName.toLower())
        mRead = true;
    setupUi();
    mTimer.start( _MINUTE );
    connect( &mTimer, SIGNAL( timeout() ), this, SLOT( updateUi()) );
    setOpenLinks(false);
    connect(this,SIGNAL(anchorClicked(QUrl)),this,SLOT(checkAnchor(QUrl)));
}

void PostWidget::checkAnchor(const QUrl & url)
{
    if( Choqok::BehaviorSettings::useCustomBrowser() ) {
        QStringList args = Choqok::BehaviorSettings::customBrowser().split(' ');
        args.append(url.toString());
        if( KProcess::startDetached( args ) == 0 ) {
            KToolInvocation::invokeBrowser(url.toString());
        }
    } else {
        KToolInvocation::invokeBrowser(url.toString());
    }
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
    ss = "<b><a href='"+ mCurrentAccount->microblog()->profileUrl( mCurrentAccount, mCurrentPost.author.userName )
         +"' title=\"" +
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
                             mImage = "<img src=\"img://profileImage\" title=\""+ mCurrentPost.author.realName +
                             "\" width=\"48\" height=\"48\" />";

    KPushButton *btnResend = addButton("btnResend", i18nc( "@info:tooltip", "ReSend" ), "retweet" );
    connect(btnResend, SIGNAL(clicked(bool)), SLOT(slotResendPost()));

    if(mCurrentAccount->username() == mCurrentPost.author.userName.toLower()) {
        KPushButton *btnRemove = addButton("btnRemove", i18nc( "@info:tooltip", "Remove" ), "edit-delete" );
        connect(btnRemove, SIGNAL(clicked(bool)), SLOT(removeCurrentPost()));
    }
}

void PostWidget::initUi()
{
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
    if( !read && !mRead && currentAccount()->username() == currentPost().author.userName.toLower()) {
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
        kDebug()<<"Emitting postReaded()";
        emit postReaded();
        setRead();
    }
    KTextBrowser::mousePressEvent(ev);
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

    if(AppearanceSettings::isEmoticonsEnabled())
        text = MediaManager::self()->parseEmoticons(text);
    //TODO Add support for PLUGINS! e.g. UnTiny, ...

    return text;
}

void PostWidget::setDirection()
{
    QString txt = mCurrentPost.content;
    txt.remove(QRegExp("RT|RD"));
    txt.remove(QRegExp("@([^\\s\\W]+)"));
    txt.remove(QRegExp("#([^\\s\\W]+)"));
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
    if ( KMessageBox::warningYesNo( this, i18n( "Are you sure to remove this post from server?" ) ) == KMessageBox::Yes ) {
        connect(mCurrentAccount->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)) );
        connect( mCurrentAccount->microblog(),
                SIGNAL(errorPost(Choqok::Account*, Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)),
                this, SLOT(slotPostError(Choqok::Account*, Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)) );
        mCurrentAccount->microblog()->removePost(mCurrentAccount, &mCurrentPost);
    }
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

    if((BehaviorSettings::resendWithQuickPost() || currentAccount()->isReadOnly()) && Global::quickPostWidget())
        Global::quickPostWidget()->setText(text);
    else
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

void PostWidget::slotPostError(Account* theAccount, Choqok::Post* post,
                               MicroBlog::ErrorType , const QString& errorMessage)
{
    if( theAccount == currentAccount() && post == &mCurrentPost) {
        kError()<<errorMessage;
        disconnect(mCurrentAccount->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                  this, SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)) );
        disconnect( mCurrentAccount->microblog(),
                    SIGNAL(errorPost(Account*,Post*,Choqok::MicroBlog::ErrorType,QString)),
                    this, SLOT(slotPostError(Account*,Post*,Choqok::MicroBlog::ErrorType,QString)) );
    }
}

void PostWidget::contextMenuEvent(QContextMenuEvent* event)
{
    KAction *copy = new KAction( i18n("Copy Post Text"), this );
    connect( copy, SIGNAL(triggered(bool)), SLOT(slotCopyPostContent()) );
    KMenu *menu = new KMenu(this);
    menu->addAction(copy);
    menu->addSeparator();
    menu->addActions( createStandardContextMenu()->actions() );
    menu->popup(event->globalPos());
}

void PostWidget::slotCopyPostContent()
{
    QApplication::clipboard()->setText( currentPost().content );
}

#include "postwidget.moc"
