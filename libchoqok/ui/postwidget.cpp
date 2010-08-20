/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "choqoktools.h"
#include "textbrowser.h"
#include <qboxlayout.h>
#include <KLocale>
#include <KPushButton>
#include <QGridLayout>
#include <KDebug>
#include "mediamanager.h"
#include "choqokbehaviorsettings.h"
#include "choqokuiglobal.h"
#include "quickpost.h"
#include <KProcess>
#include <KToolInvocation>
#include <KMessageBox>
#include "choqokappearancesettings.h"
#include <QMenu>
#include <kmenu.h>
#include <QCloseEvent>

static const int _15SECS = 15000;
static const int _MINUTE = 60000;
static const int _HOUR = 60*_MINUTE;

using namespace Choqok;
using namespace Choqok::UI;


class PostWidget::Private
{
    public:
        Private( Account* account, const Choqok::Post& post )
        : mCurrentPost(post), mCurrentAccount(account)//, mRead(false)
        {
        }
        QGridLayout *buttonsLayout;
        QMap<QString, KPushButton*> mUiButtons;//<Object name, Button>
        Post mCurrentPost;
        Account *mCurrentAccount;
//         bool mRead;
        QTimer mTimer;

        //BEGIN UI contents:
        QString mSign;
        QString mContent;
        QString mImage;
        //END UI contents;
};


const QString PostWidget::ownText ("<table width=\"100%\" ><tr><td><p>%2</p></td><td width=\"5\"><!-- empty --></td><td width=\"48\" rowspan=\"2\" align=\"right\">%1</td></tr><tr><td style=\"font-size:small;\" dir=\"ltr\" align=\"right\" valign=\"bottom\">%3</td></tr></table>");

const QString PostWidget::otherText ( "<table height=\"100%\" width=\"100%\"><tr><td rowspan=\"2\"\
width=\"48\">%1</td><td width=\"5\"><!-- EMPTY HAHA --></td><td><p>%2</p></td></tr><tr><td><!-- EMPTY HAHA --></td><td style=\"font-size:small;\" dir=\"ltr\" align=\"right\" width=\"100%\" valign=\"bottom\">%3</td></tr></table>");

const QString PostWidget::baseStyle ("KTextBrowser {border: 1px solid rgb(150,150,150);\
border-radius:5px;}  KTextBrowser {color:%1; background-color:%2}\
KPushButton{border:0px}");

// Can't validate domains with national symbols
const QString protocols = "((https?|ftps?)://)";
const QString subdomains = "(([\\S]\\.)?)";
const QString auth = "((([\\S]{1,})((:[\\S]{1,})?)@)?)";
const QString domains = "([a-z0-9-]{1,63}\\.)+";
const QString port = "(:(6553[0-5]|655[0-2][0-9]|65[0-4][\\d]{2}|6[0-4][\\d]{3}|[1-5][\\d]{4}|[1-9][\\d]{0,3})[\\D])";
const QString zone ("((a(c|d|e|f|g|i|l|m|n|o|q|r|s|t|u|w|x|z))|(b(a|b|d|e|f|g|h|i|j|l|m|n|o|r|s|t|v|w|y|z))|(c(a|c|d|f|g|h|i|k|l|m|n|o|r|u|v|x|y|z))|\
(d(e|j|k|m|o|z))|(e(c|e|g|h|r|s|t|u))|(f(i|j|k|m|o|r))|(g(a|b|d|e|f|g|h|i|l|m|n|p|q|r|s|t|u|w|y))|(h(k|m|n|r|t|u))|(i(d|e|l|m|n|o|q|r|s|t))|\
(j(e|m|o|p))|(k(e|g|h|i|m|n|p|r|w|y|z))|(l(a|b|c|i|k|r|s|t|u|v|y))|(m(a|c|d|e|f|g|h|k|l|m|n|o|p|q|r|s|t|u|v|w|x|y|z))|(n(a|c|e|f|g|i|l|o|p|r|u|z))|\
(om)|(p(a|e|f|g|h|k|l|n|r|s|t|w|y))|(qa)|(r(e|o|s|u|w))|(s(a|b|c|d|e|g|h|i|j|k|l|m|n|o|r|t|u|v|y|z))|(t(c|d|f|g|h|j|k|m|n|o|p|r|t|v|w|z))|\
(u(a|g|k|s|y|z))|(v(a|c|e|g|i|n|u))|(w(f|s))|(ye)|(z(a|m|r|w))|(asia|com|info|net|org|biz|name|pro|aero|cat|coop|edu|jobs|mobi|museum|tel|travel|gov|int|mil|local))");
const QString ip = "(25[0-5]|[2][0-4][0-9]|[0-1]?[\\d]{1,2})(\\.(25[0-5]|[2][0-4][0-9]|[0-1]?[\\d]{1,2})){3}";
const QString params = "(((\\/)[\\w:/\\?#\\[\\]@!\\$&\\(\\)\\*%\\+,;=\\._~-]{1,}|%[0-9a-f]{2})?)";

const QRegExp PostWidget::mUrlRegExp("((((" + protocols + "?)" + auth +
                          subdomains +
                          "((" + domains +               
                          zone + "(?!(\\w)))|((to|ai)\\.)))|(" + protocols + '(' + ip + ")+))" +
                          '(' + port + "?)" + "((\\/)?)"  +
                          params + ')', Qt::CaseInsensitive);

QString PostWidget::readStyle;
QString PostWidget::unreadStyle;
QString PostWidget::ownStyle;
const QString PostWidget::webIconText("&#9755;");

PostWidget::PostWidget( Account* account, const Choqok::Post& post, QWidget* parent/* = 0*/ )
    :QWidget(parent), _mainWidget(new TextBrowser(this)), d(new Private(account, post))
{
    //     setAttribute(Qt::WA_DeleteOnClose);
    _mainWidget->setFrameShape(QFrame::NoFrame);
    if(currentAccount()->username().compare( currentPost().author.userName, Qt::CaseInsensitive ) == 0 )
        d->mCurrentPost.isRead = true;
    d->mTimer.start( _MINUTE );
    connect( &d->mTimer, SIGNAL( timeout() ), this, SLOT( updateUi()) );
    connect(_mainWidget, SIGNAL(clicked(QMouseEvent*)), SLOT(mousePressEvent(QMouseEvent*)));
    connect(_mainWidget, SIGNAL(anchorClicked(QUrl)), this, SLOT(checkAnchor(QUrl)));
//     setTextInteractionFlags( Qt::TextSelectableByKeyboard | Qt::TextSelectableByMouse );
}

void PostWidget::checkAnchor(const QUrl & url)
{
    Choqok::openUrl(url);
}

PostWidget::~PostWidget()
{
    delete d;
}

Account* PostWidget::currentAccount()
{
    return d->mCurrentAccount;
}

QString PostWidget::generateSign()
{
    QString ss;
    ss = "<b><a href='"+ d->mCurrentAccount->microblog()->profileUrl( d->mCurrentAccount,
                                                                      d->mCurrentPost.author.userName )
         +"' title=\"" +
    d->mCurrentPost.author.description + "\">" + d->mCurrentPost.author.userName +
    "</a> - </b>";

    ss += "<a href=\"" + d->mCurrentPost.link +
    "\" title=\"" + d->mCurrentPost.creationDateTime.toString(Qt::DefaultLocaleLongDate) + "\">%1</a>";

    if( !d->mCurrentPost.source.isNull() )
        ss += " - " + d->mCurrentPost.source;

    return ss;
}

void PostWidget::setupUi()
{
    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    layout()->setContentsMargins(0,0,0,0);
    layout()->addWidget(_mainWidget);
    setSizePolicy(QSizePolicy::Preferred,QSizePolicy::Fixed);
    _mainWidget->setFocusProxy(this);

    d->buttonsLayout = new QGridLayout(_mainWidget);
    d->buttonsLayout->setRowStretch(0,100);
    d->buttonsLayout->setColumnStretch(5,100);
    d->buttonsLayout->setMargin(0);
    d->buttonsLayout->setSpacing(0);

    _mainWidget->setLayout(d->buttonsLayout);
    connect(_mainWidget,SIGNAL(textChanged()),this,SLOT(setHeight()));

}

void PostWidget::initUi()
{
    setupUi();
    _mainWidget->document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"),
                             MediaManager::self()->defaultImage() );

    if(d->mCurrentAccount->username().compare( d->mCurrentPost.author.userName, Qt::CaseInsensitive ) == 0
        || currentPost().isPrivate) {
        KPushButton *btnRemove = addButton("btnRemove", i18nc( "@info:tooltip", "Remove" ), "edit-delete" );
        connect(btnRemove, SIGNAL(clicked(bool)), SLOT(removeCurrentPost()));
        baseText = &ownText;
    } else {
        KPushButton *btnResend = addButton("btnResend", i18nc( "@info:tooltip", "ReSend" ), "retweet" );
        connect(btnResend, SIGNAL(clicked(bool)), SLOT(slotResendPost()));
        baseText = &otherText;
    }
    d->mImage = "<img src=\"img://profileImage\" title=\""+ d->mCurrentPost.author.realName +"\" width=\"48\" height=\"48\" />";
    d->mContent = prepareStatus(d->mCurrentPost.content);
    d->mSign = generateSign();
    setupAvatar();
    setDirection();
    setUiStyle();

    d->mContent.replace("<a href","<a style=\"text-decoration:none\" href",Qt::CaseInsensitive);
    d->mSign.replace("<a href","<a style=\"text-decoration:none\" href",Qt::CaseInsensitive);

    updateUi();
}

void PostWidget::updateUi()
{
    _mainWidget->setHtml(baseText->arg(d->mImage, d->mContent,
                                      d->mSign.arg(formatDateTime( d->mCurrentPost.creationDateTime ))));
}

void PostWidget::setStyle(const QColor& color, const QColor& back, const QColor& read, const QColor& readBack, const QColor& own, const QColor& ownBack)
{
    unreadStyle = baseStyle.arg( getColorString(color), getColorString(back) );
    readStyle = baseStyle.arg( getColorString(read), getColorString(readBack) );
    ownStyle = baseStyle.arg( getColorString(own), getColorString(ownBack) );
}

QString PostWidget::getColorString(const QColor& color)
{
    return "rgb(" + QString::number(color.red()) + ',' + QString::number(color.green()) + ',' +
    QString::number(color.blue()) + ')';
}

KPushButton * PostWidget::addButton(const QString & objName, const QString & toolTip, const QString & icon)
{
    return addButton(objName, toolTip, KIcon(icon));

}

KPushButton * PostWidget::addButton(const QString & objName, const QString & toolTip, const KIcon & icon)
{
    KPushButton * button = new KPushButton(icon, QString(), _mainWidget);
    button->setObjectName(objName);
    button->setToolTip(toolTip);
    button->setIconSize(QSize(16,16));
    button->setMinimumSize(QSize(20, 20));
    button->setMaximumSize(QSize(20, 20));
    button->setFlat(true);
    button->setVisible(false);
    button->setCursor(Qt::PointingHandCursor);

    d->mUiButtons.insert(objName, button);
    d->buttonsLayout->addWidget( button, 1, d->mUiButtons.count() );
    return button;
}

const Post &PostWidget::currentPost() const
{
    return d->mCurrentPost;
}

void PostWidget::setCurrentPost(const Choqok::Post& post)
{
    d->mCurrentPost = post;
}

void PostWidget::setRead(bool read/* = true*/)
{
    if( !read && !currentPost().isRead &&
        currentAccount()->username().compare( currentPost().author.userName, Qt::CaseInsensitive ) == 0) {
        d->mCurrentPost.isRead = true; ///Always Set own posts as read.
        setUiStyle();
    } else if( currentPost().isRead != read ) {
        d->mCurrentPost.isRead = read;
        setUiStyle();
    }
}

void PostWidget::setReadInternal()
{
    if(!isRead()){
        setRead();
        Q_EMIT postReaded();
    }
}

bool PostWidget::isRead() const
{
    return currentPost().isRead;
}

void PostWidget::setUiStyle()
{
    if (currentAccount()->username().compare( currentPost().author.userName, Qt::CaseInsensitive ) == 0)
      setStyleSheet(ownStyle);
    else {
      if(currentPost().isRead)
        setStyleSheet(readStyle);
      else
        setStyleSheet(unreadStyle);
    }
}

void PostWidget::setHeight()
{
    _mainWidget->document()->setTextWidth(width()-2);
    int h = _mainWidget->document()->size().toSize().height()+2;
    setMinimumHeight(h);
    setMaximumHeight(h);
}

void PostWidget::closeEvent(QCloseEvent* event)
{
//     kDebug();
    Q_EMIT aboutClosing(currentPost().postId, this);
    event->ignore();
    QWidget::deleteLater();
}

void PostWidget::mousePressEvent(QMouseEvent* ev)
{
    if(!isRead()) {
        setReadInternal();
    }
    QWidget::mousePressEvent(ev);
}

void PostWidget::resizeEvent ( QResizeEvent * event )
{
    setHeight();
    QWidget::resizeEvent(event);
}

void PostWidget::enterEvent ( QEvent * event )
{
    foreach(KPushButton *btn, buttons()){
        btn->show();
    }
    QWidget::enterEvent(event);
}

void PostWidget::leaveEvent ( QEvent * event )
{
    foreach(KPushButton *btn, buttons()){
        btn->hide();
    }
    QWidget::enterEvent(event);
}

QString PostWidget::prepareStatus( const QString &txt )
{
    QString text = txt;
    text.replace( "&amp;", "&amp;amp;" );
    text.replace( '<', "&lt;" );
    text.replace( '>', "&gt;" );
    int pos = 0;
    while(((pos = mUrlRegExp.indexIn(text, pos)) != -1)) {
        QString link = mUrlRegExp.cap(0);
        text.remove( pos, link.length() );
        QString tmplink = link;
        if ( !tmplink.startsWith(QLatin1String("http"), Qt::CaseInsensitive) &&
             !tmplink.startsWith(QLatin1String("ftp"), Qt::CaseInsensitive) )
             tmplink.prepend("http://");
        static const QString hrefTemplate("<a href='%1' title='%1' target='_blank'>%2</a>");
        tmplink = hrefTemplate.arg( tmplink, link );
        text.insert( pos, tmplink );
        pos += tmplink.length();
    }

    if(AppearanceSettings::isEmoticonsEnabled())
        text = MediaManager::self()->parseEmoticons(text);

    return text;
}

void PostWidget::setDirection()
{
    QString txt = d->mCurrentPost.content;
    txt.remove(QRegExp("RT|RD"));
    txt.remove(QRegExp("@([^\\s\\W]+)"));
    txt.remove(QRegExp("#([^\\s\\W]+)"));
    txt.remove(QRegExp("!([^\\s\\W]+)"));
    txt.prepend(' ');
    if( txt.isRightToLeft() ) {
        QTextOption options(_mainWidget->document()->defaultTextOption());
        options.setTextDirection( Qt::RightToLeft );
        _mainWidget->document()->setDefaultTextOption(options);
    }
}

QString PostWidget::formatDateTime(const KDateTime& time)
{
    return formatDateTime(time.dateTime());
}

QString PostWidget::formatDateTime( const QDateTime& time )
{
    int seconds = time.secsTo( QDateTime::currentDateTime() );
    if ( seconds <= 15 ) {
        d->mTimer.setInterval( _15SECS );
        return i18n( "Just now" );
    }

    if ( seconds <= 45 ) {
        d->mTimer.setInterval( _15SECS );
        return i18np( "1 sec ago", "%1 secs ago", seconds );
    }

    int minutes = ( seconds - 45 + 59 ) / 60;
    if ( minutes <= 45 ) {
        d->mTimer.setInterval( _MINUTE );
        return i18np( "1 min ago", "%1 mins ago", minutes );
    }

    int hours = ( seconds - 45 * 60 + 3599 ) / 3600;
    if ( hours <= 18 ) {
        d->mTimer.setInterval( _MINUTE * 15 );
        return i18np( "1 hour ago", "%1 hours ago", hours );
    }

    d->mTimer.setInterval( _HOUR );
    int days = ( seconds - 18 * 3600 + 24 * 3600 - 1 ) / ( 24 * 3600 );
    return i18np( "1 day ago", "%1 days ago", days );
}

void PostWidget::removeCurrentPost()
{
    if ( KMessageBox::warningYesNo( this, i18n( "Are you sure you want to remove this post from the server?" ) ) == KMessageBox::Yes ) {
        connect(d->mCurrentAccount->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)) );
        connect( d->mCurrentAccount->microblog(),
                SIGNAL(errorPost(Choqok::Account*, Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)),
                this, SLOT(slotPostError(Choqok::Account*, Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)) );
        setReadInternal();
        d->mCurrentAccount->microblog()->removePost(d->mCurrentAccount, &d->mCurrentPost);
    }
}

void PostWidget::slotCurrentPostRemoved( Account* theAccount, Post* post )
{
    if( theAccount == currentAccount() && post == &d->mCurrentPost )
        this->close();
}

void PostWidget::slotResendPost()
{
    QString text = generateResendText();
    setReadInternal();
    if((BehaviorSettings::resendWithQuickPost() || currentAccount()->isReadOnly()) && Global::quickPostWidget())
        Global::quickPostWidget()->setText(text);
    else
        Q_EMIT resendPost(text);
}

QString PostWidget::generateResendText()
{
    if (BehaviorSettings::useCustomRT())
    {
        return QString(BehaviorSettings::customRT()) + " @" + currentPost().author.userName + ": " + currentPost().content;
    }
    else
    {
        QChar re(0x267B);
        return QString(re) + " @" + currentPost().author.userName + ": " + currentPost().content;
    }
}

void PostWidget::setupAvatar()
{
    QPixmap *pix = MediaManager::self()->fetchImage( d->mCurrentPost.author.profileImageUrl,
                                      MediaManager::Async );
    if(pix)
        avatarFetched(d->mCurrentPost.author.profileImageUrl, *pix);
    else {
        connect( MediaManager::self(), SIGNAL( imageFetched(QString,QPixmap)),
                this, SLOT(avatarFetched(QString, QPixmap) ) );
        connect( MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                this, SLOT(avatarFetchError(QString,QString)) );
    }
}

void PostWidget::avatarFetched(const QString& remoteUrl, const QPixmap& pixmap)
{
    if ( remoteUrl == d->mCurrentPost.author.profileImageUrl ) {
        QString url = "img://profileImage";
        _mainWidget->document()->addResource( QTextDocument::ImageResource, url, pixmap );
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
    if( remoteUrl == d->mCurrentPost.author.profileImageUrl ){
        ///Avatar fetching is failed! but will not disconnect to get the img if it fetches later!
        QString url = "img://profileImage";
        _mainWidget->document()->addResource( QTextDocument::ImageResource,
                                             url, KIcon("image-missing").pixmap(48) );
        updateUi();
    }
}

QMap<QString, KPushButton* >& PostWidget::buttons()
{
    return d->mUiButtons;
}

void PostWidget::slotPostError(Account* theAccount, Choqok::Post* post,
                               MicroBlog::ErrorType , const QString& errorMessage)
{
    if( theAccount == currentAccount() && post == &d->mCurrentPost) {
        kDebug()<<errorMessage;
        disconnect(d->mCurrentAccount->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                  this, SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)) );
        disconnect( d->mCurrentAccount->microblog(),
                    SIGNAL(errorPost(Account*,Post*,Choqok::MicroBlog::ErrorType,QString)),
                    this, SLOT(slotPostError(Account*,Post*,Choqok::MicroBlog::ErrorType,QString)) );
    }
}

QString PostWidget::avatarText() const
{
    return d->mImage;
}

void PostWidget::setAvatarText(const QString& text)
{
    d->mImage = text;
    updateUi();
}

QString PostWidget::content() const
{
    return d->mContent;
}

void PostWidget::setContent(const QString& content)
{
    d->mContent = content;
    updateUi();
}

QString PostWidget::sign() const
{
    return d->mSign;
}

void PostWidget::setSign(const QString& sign)
{
    d->mSign = sign;
    updateUi();
}

void PostWidget::deleteLater()
{
    close();
}

TextBrowser* PostWidget::mainWidget()
{
    return _mainWidget;
}

void PostWidget::wheelEvent(QWheelEvent* event)
{
    event->ignore();
}

#include "postwidget.moc"
