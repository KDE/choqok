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
#include "postwidget.h"

#include <QCloseEvent>
#include <QGridLayout>
#include <QTimer>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>

#include "choqokappearancesettings.h"
#include "choqokbehaviorsettings.h"
#include "choqoktools.h"
#include "choqokuiglobal.h"
#include "libchoqokdebug.h"
#include "mediamanager.h"
#include "quickpost.h"
#include "timelinewidget.h"
#include "textbrowser.h"
#include "urlutils.h"

static const int _15SECS = 15000;
static const int _MINUTE = 60000;
static const int _HOUR = 60 * _MINUTE;

using namespace Choqok;
using namespace Choqok::UI;

class PostWidget::Private
{
public:
    Private(Account *account, Choqok::Post *post)
        : mCurrentPost(post), mCurrentAccount(account), dir(QLatin1String("ltr")), timeline(nullptr)
    {
        mCurrentPost->owners++;

        if (!mCurrentPost->media.isEmpty()) {
            imageUrl = mCurrentPost->media;
        }
    }
    QGridLayout *buttonsLayout;
    QMap<QString, QPushButton *> mUiButtons; //<Object name, Button>
    Post *mCurrentPost;
    Account *mCurrentAccount;
//         bool mRead;
    QTimer mTimer;

    //BEGIN UI contents:
    QString mSign;
    QString mContent;
    QString mProfileImage;
    QString mImage;
    QUrl imageUrl;
    QString dir;
    QPixmap originalImage;
    QString extraContents;
    //END UI contents;

    QStringList detectedUrls;

    TimelineWidget *timeline;
    
    static const QLatin1String resourceImageUrl;
};

const QString mImageTemplate(QLatin1String("<div style=\"padding-top:5px;padding-bottom:3px;\"><img width=\"%1\" height=\"%2\" src=\"%3\"/></div>"));

const QLatin1String PostWidget::Private::resourceImageUrl("img://postImage");

const QString PostWidget::baseTextTemplate(QLatin1String("<table height=\"100%\" width=\"100%\"><tr><td width=\"48\" style=\"padding-right: 5px;\">%1</td><td dir=\"%4\" style=\"padding-right:3px;\"><p>%2</p></td></tr>%5%6<tr><td></td><td style=\"font-size:small;\" dir=\"ltr\" align=\"right\" valign=\"bottom\">%3</td></tr></table>"));

const QString PostWidget::baseStyle(QLatin1String("QTextBrowser {border: 1px solid rgb(150,150,150);\
border-radius:5px; color:%1; background-color:%2; %3}\
QPushButton{border:0px} QPushButton::menu-indicator{image:none;}"));

const QString PostWidget::hrefTemplate(QLatin1String("<a href='%1' title='%1' target='_blank'>%2</a>"));

const QRegExp PostWidget::dirRegExp(QLatin1String("(RT|RD)|(@([^\\s\\W]+))|(#([^\\s\\W]+))|(!([^\\s\\W]+))"));

QString PostWidget::readStyle;
QString PostWidget::unreadStyle;
QString PostWidget::ownStyle;
const QString PostWidget::webIconText(QLatin1String("&#9755;"));

PostWidget::PostWidget(Account *account, Choqok::Post *post, QWidget *parent/* = 0*/)
    : QWidget(parent), _mainWidget(new TextBrowser(this)), d(new Private(account, post))
{
    setAttribute(Qt::WA_DeleteOnClose);
    _mainWidget->setFrameShape(QFrame::NoFrame);
    if (isOwnPost()) {
        d->mCurrentPost->isRead = true;
    }
    d->mTimer.start(_MINUTE);
    connect(&d->mTimer, &QTimer::timeout, this, &PostWidget::updateUi);
    connect(_mainWidget, &TextBrowser::clicked, this, &PostWidget::mousePressEvent);
    connect(_mainWidget, &TextBrowser::anchorClicked, this, &PostWidget::checkAnchor);

    d->timeline = qobject_cast<TimelineWidget *>(parent);

    setHeight();
}

void PostWidget::checkAnchor(const QUrl &url)
{
    if (url.scheme() == QLatin1String("choqok")) {
        if (url.host() == QLatin1String("showoriginalpost")) {
            setContent(prepareStatus(currentPost()->content).replace(QLatin1String("<a href"), QLatin1String("<a style=\"text-decoration:none\" href"), Qt::CaseInsensitive));
            updateUi();
        }
    } else {
        Choqok::openUrl(url);
    }
}

PostWidget::~PostWidget()
{
    if (d->mCurrentPost->owners < 2) {
        delete d->mCurrentPost;
    } else {
        d->mCurrentPost->owners--;
    }
    delete d;
}

Account *PostWidget::currentAccount()
{
    return d->mCurrentAccount;
}

QString PostWidget::generateSign()
{
    QString ss = QStringLiteral("<b>%1 - </b>").arg(getUsernameHyperlink(d->mCurrentPost->author));

    QDateTime time;
    if (d->mCurrentPost->repeatedDateTime.isNull()) {
        time = d->mCurrentPost->creationDateTime;
    } else {
        time = d->mCurrentPost->repeatedDateTime;
    }

    ss += QStringLiteral("<a href=\"%1\" title=\"%2\">%3</a>").arg(d->mCurrentPost->link.toDisplayString())
            .arg(time.toString(Qt::DefaultLocaleLongDate)).arg(QStringLiteral("%1"));

    if (!d->mCurrentPost->source.isEmpty()) {
        ss += QLatin1String(" - ") + d->mCurrentPost->source;
    }

    return ss;
}

QString PostWidget::getUsernameHyperlink(const Choqok::User &user) const
{
    return QStringLiteral("<a href=\"%1\" title=\"%2\">%3</a>")
            .arg(d->mCurrentAccount->microblog()->profileUrl(d->mCurrentAccount, user.userName).toDisplayString())
            .arg(user.description.isEmpty() ? user.realName : user.description)
            .arg(user.userName);
}

void PostWidget::setupUi()
{
    setLayout(new QVBoxLayout);
    layout()->setMargin(0);
    layout()->setContentsMargins(0, 0, 0, 0);
    layout()->addWidget(_mainWidget);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    _mainWidget->setFocusProxy(this);

    d->buttonsLayout = new QGridLayout(_mainWidget);
    d->buttonsLayout->setRowStretch(0, 100);
    d->buttonsLayout->setColumnStretch(5, 100);
    d->buttonsLayout->setMargin(0);
    d->buttonsLayout->setSpacing(0);

    _mainWidget->setLayout(d->buttonsLayout);
    connect(_mainWidget, &TextBrowser::textChanged, this, &PostWidget::setHeight);

}

void PostWidget::initUi()
{
    setupUi();
    _mainWidget->document()->addResource(QTextDocument::ImageResource, QUrl(QLatin1String("img://profileImage")),
                                         MediaManager::self()->defaultImage());

    if (isRemoveAvailable()) {
        QPushButton *btnRemove = addButton(QLatin1String("btnRemove"), i18nc("@info:tooltip", "Remove"), QLatin1String("edit-delete"));
        connect(btnRemove, &QPushButton::clicked, this, &PostWidget::removeCurrentPost);
    }

    if (isResendAvailable()) {
        QPushButton *btnResend = addButton(QLatin1String("btnResend"), i18nc("@info:tooltip", "ReSend"), QLatin1String("retweet"));
        connect(btnResend, &QPushButton::clicked, this, &PostWidget::slotResendPost);
    }

    d->mProfileImage = QLatin1String("<img src=\"img://profileImage\" title=\"") + d->mCurrentPost->author.realName + QLatin1String("\" width=\"48\" height=\"48\" />");
    d->mContent = prepareStatus(d->mCurrentPost->content);
    d->mSign = generateSign();
    setupAvatar();
    fetchImage();
    d->dir = getDirection(d->mCurrentPost->content);
    setUiStyle();

    d->mContent.replace(QLatin1String("<a href"), QLatin1String("<a style=\"text-decoration:none\" href"), Qt::CaseInsensitive);
    d->mContent.replace(QLatin1String("\n"), QLatin1String("<br/>"));
    d->extraContents.replace(QLatin1String("<a href"), QLatin1String("<a style=\"text-decoration:none\" href"), Qt::CaseInsensitive);
    d->mSign.replace(QLatin1String("<a href"), QLatin1String("<a style=\"text-decoration:none\" href"), Qt::CaseInsensitive);

    updateUi();
}

void PostWidget::updateUi()
{
    QDateTime time;
    if (currentPost()->repeatedDateTime.isNull()) {
        time = currentPost()->creationDateTime;
    } else {
        time = currentPost()->repeatedDateTime;
    }

    _mainWidget->setHtml(baseTextTemplate.arg( d->mProfileImage,                     /*1*/
                                               d->mContent,                          /*2*/
                                               d->mSign.arg(formatDateTime(time)),   /*3*/
                                               d->dir,                               /*4*/
                                               d->mImage,                            /*5*/
                                               d->extraContents                      /*6*/
                                               ));
}

void PostWidget::setStyle(const QColor &color, const QColor &back, const QColor &read, const QColor &readBack, const QColor &own, const QColor &ownBack, const QFont &font)
{
    QString fntStr = QLatin1String("font-family:\"") + font.family() + QLatin1String("\"; font-size:") + QString::number(font.pointSize()) + QLatin1String("pt;");
    fntStr += (font.bold() ? QLatin1String(" font-weight:bold;") : QString()) + (font.italic() ? QLatin1String(" font-style:italic;") : QString());
    unreadStyle = baseStyle.arg(getColorString(color), getColorString(back), fntStr);
    readStyle = baseStyle.arg(getColorString(read), getColorString(readBack), fntStr);
    ownStyle = baseStyle.arg(getColorString(own), getColorString(ownBack), fntStr);
}

QPushButton *PostWidget::addButton(const QString &objName, const QString &toolTip, const QString &icon)
{
    return addButton(objName, toolTip, QIcon::fromTheme(icon));

}

QPushButton *PostWidget::addButton(const QString &objName, const QString &toolTip, const QIcon &icon)
{
    QPushButton *button = new QPushButton(icon, QString(), _mainWidget);
    button->setObjectName(objName);
    button->setToolTip(toolTip);
    button->setIconSize(QSize(16, 16));
    button->setMinimumSize(QSize(20, 20));
    button->setMaximumSize(QSize(20, 20));
    button->setFlat(true);
    button->setVisible(false);
    button->setCursor(Qt::PointingHandCursor);

    d->mUiButtons.insert(objName, button);
    d->buttonsLayout->addWidget(button, 1, d->mUiButtons.count());
    return button;
}

Post *PostWidget::currentPost() const
{
    return d->mCurrentPost;
}

void PostWidget::setCurrentPost(Post *post)
{
    d->mCurrentPost = post;
}

void PostWidget::setRead(bool read/* = true*/)
{
    if (!read && !currentPost()->isRead &&
            currentAccount()->username().compare(currentPost()->author.userName, Qt::CaseInsensitive) == 0) {
        d->mCurrentPost->isRead = true; ///Always Set own posts as read.
        setUiStyle();
    } else if (currentPost()->isRead != read) {
        d->mCurrentPost->isRead = read;
        setUiStyle();
    }
}

void PostWidget::setReadWithSignal()
{
    if (!isRead()) {
        setRead();
        Q_EMIT postReaded();
    }
}

bool PostWidget::isRead() const
{
    return currentPost()->isRead;
}

void PostWidget::setUiStyle()
{
    if (isOwnPost()) {
        setStyleSheet(ownStyle);
    } else {
        if (currentPost()->isRead) {
            setStyleSheet(readStyle);
        } else {
            setStyleSheet(unreadStyle);
        }
    }
    setHeight();
}

bool PostWidget::isOwnPost()
{
    return currentAccount()->username().compare(currentPost()->author.userName, Qt::CaseInsensitive) == 0;
}

void PostWidget::setHeight()
{
    _mainWidget->document()->setTextWidth(width() - 2);
    int h = _mainWidget->document()->size().toSize().height() + 2;
    setFixedHeight(h);
}

void PostWidget::closeEvent(QCloseEvent *event)
{
    clearFocus();
    if (!isRead()) {
        setReadWithSignal();
    }
    Q_EMIT aboutClosing(currentPost()->postId, this);
    event->accept();
}

void PostWidget::mousePressEvent(QMouseEvent *ev)
{
    if (!isRead()) {
        setReadWithSignal();
    }
    QWidget::mousePressEvent(ev);
}

void PostWidget::resizeEvent(QResizeEvent *event)
{
    updatePostImage( event->size().width() );
    setHeight();
    updateUi();
    QWidget::resizeEvent(event);
}

void PostWidget::enterEvent(QEvent *event)
{
    for (QPushButton *btn: buttons()) {
        if (btn) { //A crash happens here :/
            btn->show();
        }
    }
    QWidget::enterEvent(event);
}

void PostWidget::leaveEvent(QEvent *event)
{
    for (QPushButton *btn: buttons()) {
        if (btn) {
            btn->hide();
        }
    }
    QWidget::enterEvent(event);
}

void PostWidget::updatePostImage(int width)
{
    if ( !d->originalImage.isNull() ) {
        // TODO: Find a way to calculate the difference we need to subtract.
        width -= 76;
        
        QPixmap newPixmap = d->originalImage.scaledToWidth(width, Qt::SmoothTransformation);
        auto newW = newPixmap.width();
        auto newH = newPixmap.height();
        auto origW = d->originalImage.width();
        auto origH = d->originalImage.height();
        
        const QUrl url(d->resourceImageUrl);
        // only use scaled image if it's smaller than the original one
        if (newW <= origW && newH <= origH) { // never scale up
            d->mImage = mImageTemplate.arg(QString::number(newW), QString::number(newH), d->resourceImageUrl);
            _mainWidget->document()->addResource(QTextDocument::ImageResource, url, newPixmap);
        } else {
            d->mImage = mImageTemplate.arg(QString::number(origW), QString::number(origH), d->resourceImageUrl);
            _mainWidget->document()->addResource(QTextDocument::ImageResource, url, d->originalImage);
        }
    }
}

QString PostWidget::prepareStatus(const QString &txt)
{
    QString text = removeTags(txt);

    d->detectedUrls = UrlUtils::detectUrls(text);
    for (const QString &url: d->detectedUrls) {
        QString httpUrl(url);
        if (!httpUrl.startsWith(QLatin1String("http"), Qt::CaseInsensitive) &&
                !httpUrl.startsWith(QLatin1String("ftp"), Qt::CaseInsensitive)) {
            httpUrl.prepend(QLatin1String("http://"));
            text.replace(url, httpUrl);
        }

        text.replace(url, hrefTemplate.arg(httpUrl, url));
    }

    text = UrlUtils::detectEmails(text);

    if (AppearanceSettings::isEmoticonsEnabled()) {
        text = MediaManager::self()->parseEmoticons(text);
    }

    return text;
}

QString PostWidget::removeTags(const QString &text) const
{
    QString txt(text);

    txt.replace(QLatin1Char('<'), QLatin1String("&lt;"));
    txt.replace(QLatin1Char('>'), QLatin1String("&gt;"));

    return txt;
}
QLatin1String PostWidget::getDirection(QString txt)
{
    txt.remove(dirRegExp);
    txt = txt.trimmed();
    if (txt.isRightToLeft()) {
        return QLatin1String("rtl");
    }
    else {
        return QLatin1String("ltr");
    }
}

QString PostWidget::formatDateTime(const QDateTime &time)
{
    if (!time.isValid()) {
        return tr("Invalid Time");
    }
    auto seconds = time.secsTo(QDateTime::currentDateTime());
    if (seconds <= 15) {
        d->mTimer.setInterval(_15SECS);
        return i18n("Just now");
    }

    if (seconds <= 45) {
        d->mTimer.setInterval(_15SECS);
        return i18np("1 sec ago", "%1 secs ago", seconds);
    }

    auto minutes = (seconds - 45 + 59) / 60;
    if (minutes <= 45) {
        d->mTimer.setInterval(_MINUTE);
        return i18np("1 min ago", "%1 mins ago", minutes);
    }

    auto hours = (seconds - 45 * 60 + 3599) / 3600;
    if (hours <= 18) {
        d->mTimer.setInterval(_MINUTE * 15);
        return i18np("1 hour ago", "%1 hours ago", hours);
    }

    d->mTimer.setInterval(_HOUR);
    auto days = (seconds - 18 * 3600 + 24 * 3600 - 1) / (24 * 3600);
    return i18np("1 day ago", "%1 days ago", days);
}

void PostWidget::removeCurrentPost()
{
    if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to remove this post from the server?")) == KMessageBox::Yes) {
        connect(d->mCurrentAccount->microblog(), &MicroBlog::postRemoved,
                this, &PostWidget::slotCurrentPostRemoved);
        connect(d->mCurrentAccount->microblog(), &MicroBlog::errorPost, this, &PostWidget::slotPostError);
        setReadWithSignal();
        d->mCurrentAccount->microblog()->removePost(d->mCurrentAccount, d->mCurrentPost);
    }
}

void PostWidget::slotCurrentPostRemoved(Account *theAccount, Post *post)
{
    if (theAccount == currentAccount() && post == d->mCurrentPost) {
        this->close();
    }
}

void PostWidget::slotResendPost()
{
    QString text = generateResendText();
    setReadWithSignal();
    if ((BehaviorSettings::resendWithQuickPost() || currentAccount()->isReadOnly()) && Global::quickPostWidget()) {
        Global::quickPostWidget()->setText(text);
    } else {
        Q_EMIT resendPost(text);
    }
}

QString PostWidget::generateResendText()
{
    if (BehaviorSettings::useCustomRT()) {
        return QString(BehaviorSettings::customRT()) + QLatin1String(" @") + currentPost()->author.userName + QLatin1String(": ") + currentPost()->content;
    } else {
        QChar re(0x267B);
        return QString(re) + QLatin1String(" @") + currentPost()->author.userName + QLatin1String(": ") + currentPost()->content;
    }
}

void PostWidget::fetchImage()
{
    if (d->imageUrl.isEmpty()) {
        return;
    }

    QPixmap pix = MediaManager::self()->fetchImage(d->imageUrl, MediaManager::Async);

    if (!pix.isNull()) {
        slotImageFetched(d->imageUrl, pix);
    } else {
        connect(MediaManager::self(), &MediaManager::imageFetched, this,
                &PostWidget::slotImageFetched);
    }
}

void PostWidget::slotImageFetched(const QUrl &remoteUrl, const QPixmap &pixmap)
{
    if (remoteUrl == d->imageUrl) {
        disconnect(MediaManager::self(), &MediaManager::imageFetched, this, &PostWidget::slotImageFetched);
        d->originalImage = pixmap;
        updatePostImage( width() );
        updateUi();
    }
}

void PostWidget::setupAvatar()
{
    QPixmap pix = MediaManager::self()->fetchImage(d->mCurrentPost->author.profileImageUrl,
                  MediaManager::Async);
    if (!pix.isNull()) {
        avatarFetched(d->mCurrentPost->author.profileImageUrl, pix);
    } else {
        connect(MediaManager::self(), &MediaManager::imageFetched, this, &PostWidget::avatarFetched);
        connect(MediaManager::self(), &MediaManager::fetchError, this, &PostWidget::avatarFetchError);
    }
}

void PostWidget::avatarFetched(const QUrl &remoteUrl, const QPixmap &pixmap)
{
    if (remoteUrl == d->mCurrentPost->author.profileImageUrl) {
        const QUrl url(QLatin1String("img://profileImage"));
        _mainWidget->document()->addResource(QTextDocument::ImageResource, url, pixmap);
        updateUi();
        disconnect(MediaManager::self(), &MediaManager::imageFetched, this, &PostWidget::avatarFetched);
        disconnect(MediaManager::self(), &MediaManager::fetchError, this, &PostWidget::avatarFetchError);
    }
}

void PostWidget::avatarFetchError(const QUrl &remoteUrl, const QString &errMsg)
{
    Q_UNUSED(errMsg);
    if (remoteUrl == d->mCurrentPost->author.profileImageUrl) {
        ///Avatar fetching is failed! but will not disconnect to get the img if it fetches later!
        const QUrl url(QLatin1String("img://profileImage"));
        _mainWidget->document()->addResource(QTextDocument::ImageResource,
                                             url, QIcon::fromTheme(QLatin1String("image-missing")).pixmap(48));
        updateUi();
    }
}

QMap<QString, QPushButton * > &PostWidget::buttons()
{
    return d->mUiButtons;
}

void PostWidget::slotPostError(Account *theAccount, Choqok::Post *post,
                               MicroBlog::ErrorType , const QString &errorMessage)
{
    if (theAccount == currentAccount() && post == d->mCurrentPost) {
        qCDebug(CHOQOK) << errorMessage;
        disconnect(d->mCurrentAccount->microblog(), &MicroBlog::postRemoved,
                   this, &PostWidget::slotCurrentPostRemoved);
        disconnect(d->mCurrentAccount->microblog(), &MicroBlog::errorPost,
                   this, &PostWidget::slotPostError);
    }
}

QString PostWidget::avatarText() const
{
    return d->mProfileImage;
}

void PostWidget::setAvatarText(const QString &text)
{
    d->mProfileImage = text;
    updateUi();
}

QString PostWidget::content() const
{
    return d->mContent;
}

void PostWidget::setContent(const QString &content)
{
    d->mContent = content;
    updateUi();
}

QStringList PostWidget::urls()
{
    return d->detectedUrls;
}

QString PostWidget::sign() const
{
    return d->mSign;
}

void PostWidget::setSign(const QString &sign)
{
    d->mSign = sign;
    updateUi();
}

void PostWidget::deleteLater()
{
    close();
}

TextBrowser *PostWidget::mainWidget()
{
    return _mainWidget;
}

void PostWidget::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}

void PostWidget::addAction(QAction *action)
{
    TextBrowser::addAction(action);
}

TimelineWidget *PostWidget::timelineWidget() const
{
    return d->timeline;
}

class PostWidgetUserData::Private
{
public:
    Private(PostWidget *postwidget)
        : postWidget(postwidget)
    {}
    PostWidget *postWidget;
};

PostWidgetUserData::PostWidgetUserData(PostWidget *postWidget)
    : QObjectUserData(), d(new Private(postWidget))
{

}

PostWidgetUserData::~PostWidgetUserData()
{
    delete d;
}

PostWidget *PostWidgetUserData::postWidget()
{
    return d->postWidget;
}

void PostWidgetUserData::setPostWidget(PostWidget *widget)
{
    d->postWidget = widget;
}

QString PostWidget::getBaseStyle()
{
    return baseStyle;
}

bool PostWidget::isRemoveAvailable()
{
    return d->mCurrentAccount->username().compare(d->mCurrentPost->author.userName, Qt::CaseInsensitive) == 0;
}

bool PostWidget::isResendAvailable()
{
    return d->mCurrentAccount->username().compare(d->mCurrentPost->author.userName, Qt::CaseInsensitive) != 0;
}

void PostWidget::setExtraContents(const QString& text)
{
    d->extraContents = text;
}

QString PostWidget::extraContents() const
{
    return d->extraContents;
}

