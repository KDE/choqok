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
        : mCurrentPost(post), mCurrentAccount(account), dir("ltr"), timeline(0)
    {
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
    QString imageUrl;
    QString dir;
    QPixmap originalImage;
    //END UI contents;

    QStringList detectedUrls;

    TimelineWidget *timeline;
};

const QString PostWidget::ownText("<table width=\"100%\" ><tr><td width=\"90%\" dir=\"%4\">%2</td><td  rowspan=\"2\" align=\"right\">%1</td></tr><tr>%5</tr><tr><td colspan=\"2\"  style=\"font-size:small;\" dir=\"ltr\" align=\"right\" width=\"100%\" valign=\"bottom\">%3</td></tr></table>");

const QString PostWidget::otherText("<table height=\"100%\" width=\"100%\"><tr><td rowspan=\"2\" width=\"48\">%1</td><td width=\"5\"><!-- EMPTY HAHA --></td><td colspan=\"2\" dir=\"%4\"><p>%2</p></td></tr><tr><td></td>%5</tr><tr><td ><!-- empty --></td><td></td><td colspan=\"2\" style=\"font-size:small;\" dir=\"ltr\" align=\"right\" width=\"100%\" valign=\"bottom\">%3</td></tr></table>");

const QString PostWidget::baseStyle("QTextBrowser {border: 1px solid rgb(150,150,150);\
border-radius:5px; color:%1; background-color:%2; %3}\
QPushButton{border:0px}");

const QString PostWidget::hrefTemplate("<a href='%1' title='%1' target='_blank'>%2</a>");

const QRegExp PostWidget::dirRegExp("(RT|RD)|(@([^\\s\\W]+))|(#([^\\s\\W]+))|(!([^\\s\\W]+))");

QString PostWidget::readStyle;
QString PostWidget::unreadStyle;
QString PostWidget::ownStyle;
const QString PostWidget::webIconText("&#9755;");

PostWidget::PostWidget(Account *account, Choqok::Post *post, QWidget *parent/* = 0*/)
    : QWidget(parent), _mainWidget(new TextBrowser(this)), d(new Private(account, post))
{
    setAttribute(Qt::WA_DeleteOnClose);
    _mainWidget->setFrameShape(QFrame::NoFrame);
    if (isOwnPost()) {
        d->mCurrentPost->isRead = true;
    }
    d->mTimer.start(_MINUTE);
    connect(&d->mTimer, SIGNAL(timeout()), this, SLOT(updateUi()));
    connect(_mainWidget, SIGNAL(clicked(QMouseEvent*)), SLOT(mousePressEvent(QMouseEvent*)));
    connect(_mainWidget, SIGNAL(anchorClicked(QUrl)), this, SLOT(checkAnchor(QUrl)));

    d->timeline = qobject_cast<TimelineWidget *>(parent);
    d->mCurrentPost->owners++;

    if (!d->mCurrentPost->media.isEmpty()) {
        d->imageUrl = d->mCurrentPost->media;
    }

    setHeight();
}

void PostWidget::checkAnchor(const QUrl &url)
{
    if (url.scheme() == "choqok") {
        if (url.host() == "showoriginalpost") {
            setContent(prepareStatus(currentPost()->content).replace("<a href", "<a style=\"text-decoration:none\" href", Qt::CaseInsensitive));
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
    QString ss;
    ss = "<b><a href='" + d->mCurrentAccount->microblog()->profileUrl(d->mCurrentAccount,
            d->mCurrentPost->author.userName)
         + "' title=\"" +
         d->mCurrentPost->author.description + "\">" + d->mCurrentPost->author.userName +
         "</a> - </b>";

    ss += "<a href=\"" + d->mCurrentPost->link +
          "\" title=\"" + d->mCurrentPost->creationDateTime.toString(Qt::DefaultLocaleLongDate) + "\">%1</a>";

    if (!d->mCurrentPost->source.isNull()) {
        ss += " - " + d->mCurrentPost->source;
    }

    return ss;
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
    connect(_mainWidget, SIGNAL(textChanged()), this, SLOT(setHeight()));

}

void PostWidget::initUi()
{
    setupUi();
    _mainWidget->document()->addResource(QTextDocument::ImageResource, QUrl("img://profileImage"),
                                         MediaManager::self()->defaultImage());

    if (isOwnPost()) {
        baseText = &ownText;
    } else {
        baseText = &otherText;
    }

    if (isRemoveAvailable()) {
        QPushButton *btnRemove = addButton("btnRemove", i18nc("@info:tooltip", "Remove"), "edit-delete");
        connect(btnRemove, SIGNAL(clicked(bool)), SLOT(removeCurrentPost()));
        baseText = &ownText;
    }

    if (isResendAvailable()) {
        QPushButton *btnResend = addButton("btnResend", i18nc("@info:tooltip", "ReSend"), "retweet");
        connect(btnResend, SIGNAL(clicked(bool)), SLOT(slotResendPost()));
        baseText = &otherText;
    }

    /*
    if(d->mCurrentAccount->username().compare( d->mCurrentPost->author.userName, Qt::CaseInsensitive ) == 0
        || currentPost()->isPrivate) {
        QPushButton *btnRemove = addButton("btnRemove", i18nc( "@info:tooltip", "Remove" ), "edit-delete" );
        connect(btnRemove, SIGNAL(clicked(bool)), SLOT(removeCurrentPost()));
        baseText = &ownText;
    } else {
        QPushButton *btnResend = addButton("btnResend", i18nc( "@info:tooltip", "ReSend" ), "retweet" );
        connect(btnResend, SIGNAL(clicked(bool)), SLOT(slotResendPost()));
        baseText = &otherText;
    }*/

    d->mProfileImage = "<img src=\"img://profileImage\" title=\"" + d->mCurrentPost->author.realName + "\" width=\"48\" height=\"48\" />";
    if (!d->imageUrl.isEmpty()) {
        d->mImage = QString("<td width=\"%1\" height=\"%2\" style=\"padding-left: 5px; padding-left: 5px;\"><img src=\"img://postImage\"  /></td>").arg(d->mCurrentPost->mediaSizeWidth, d->mCurrentPost->mediaSizeHeight);
    }
    d->mContent = prepareStatus(d->mCurrentPost->content);
    d->mSign = generateSign();
    setupAvatar();
    fetchImage();
    setDirection();
    setUiStyle();

    d->mContent.replace("<a href", "<a style=\"text-decoration:none\" href", Qt::CaseInsensitive);
    d->mContent.replace("\n", "<br/>");

    d->mSign.replace("<a href", "<a style=\"text-decoration:none\" href", Qt::CaseInsensitive);

    updateUi();
}

void PostWidget::updateUi()
{
    _mainWidget->setHtml(baseText->arg(d->mProfileImage, d->mContent,
                                       d->mSign.arg(formatDateTime(d->mCurrentPost->creationDateTime)),
                                       d->dir,
                                       d->mImage
                                      ));
}

void PostWidget::setStyle(const QColor &color, const QColor &back, const QColor &read, const QColor &readBack, const QColor &own, const QColor &ownBack, const QFont &font)
{
    QString fntStr = "font-family:\"" + font.family() + "\"; font-size:" + QString::number(font.pointSize()) + "pt;";
    fntStr += (font.bold() ? " font-weight:bold;" : QString()) + (font.italic() ? " font-style:italic;" : QString());
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
    // only scale if image is present
    if (!d->originalImage.isNull()) {

        // TODO: Find a way to calculate the difference we need to subtract.
        int w = event->size().width() - 76;

        QPixmap newPixmap = d->originalImage.scaledToWidth(w, Qt::SmoothTransformation);
        int newW = newPixmap.width();
        int newH = newPixmap.height();

        const QUrl url("img://postImage");
        // only use scaled image if it's smaller than the original one
        if (newW <= d->originalImage.width() && newH <= d->originalImage.height()) { // never scale up
            d->mImage = QString("<td width=\"%1\" height=\"%2\"><img src=\"img://postImage\"  /></td>").arg(newW, newH);
            _mainWidget->document()->addResource(QTextDocument::ImageResource, url, newPixmap);
        } else {
            d->mImage = QString("<td width=\"%1\" height=\"%2\"><img src=\"img://postImage\"  /></td>").arg(d->mCurrentPost->mediaSizeWidth, d->mCurrentPost->mediaSizeHeight);
            _mainWidget->document()->addResource(QTextDocument::ImageResource, url, d->originalImage);
        }
    }
    setHeight();
    updateUi();
    QWidget::resizeEvent(event);
}

void PostWidget::enterEvent(QEvent *event)
{
    Q_FOREACH (QPushButton *btn, buttons()) {
        if (btn) { //A crash happens here :/
            btn->show();
        }
    }
    QWidget::enterEvent(event);
}

void PostWidget::leaveEvent(QEvent *event)
{
    Q_FOREACH (QPushButton *btn, buttons()) {
        if (btn) {
            btn->hide();
        }
    }
    QWidget::enterEvent(event);
}

QString PostWidget::prepareStatus(const QString &txt)
{
    QString text = txt;
//     text.replace( "&amp;", "&amp;amp;" );
    text = removeTags(text);

    d->detectedUrls = UrlUtils::detectUrls(text);
    Q_FOREACH (const QString &url, d->detectedUrls) {
        QString httpUrl(url);
        if (!httpUrl.startsWith(QLatin1String("http"), Qt::CaseInsensitive) &&
                !httpUrl.startsWith(QLatin1String("ftp"), Qt::CaseInsensitive)) {
            httpUrl.prepend("http://");
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

    txt.replace('<', "&lt;");
    txt.replace('>', "&gt;");

    return txt;
}
void PostWidget::setDirection()
{
    QString txt = d->mCurrentPost->content;
    txt.remove(dirRegExp);
    txt = txt.trimmed();
    if (txt.isRightToLeft()) {
        d->dir = "rtl";
    }
}

QString PostWidget::formatDateTime(const QDateTime &time)
{
    if (!time.isValid()) {
        return tr("Invalid Time");
    }
    int seconds = time.secsTo(QDateTime::currentDateTime());
    if (seconds <= 15) {
        d->mTimer.setInterval(_15SECS);
        return i18n("Just now");
    }

    if (seconds <= 45) {
        d->mTimer.setInterval(_15SECS);
        return i18np("1 sec ago", "%1 secs ago", seconds);
    }

    int minutes = (seconds - 45 + 59) / 60;
    if (minutes <= 45) {
        d->mTimer.setInterval(_MINUTE);
        return i18np("1 min ago", "%1 mins ago", minutes);
    }

    int hours = (seconds - 45 * 60 + 3599) / 3600;
    if (hours <= 18) {
        d->mTimer.setInterval(_MINUTE * 15);
        return i18np("1 hour ago", "%1 hours ago", hours);
    }

    d->mTimer.setInterval(_HOUR);
    int days = (seconds - 18 * 3600 + 24 * 3600 - 1) / (24 * 3600);
    return i18np("1 day ago", "%1 days ago", days);
}

void PostWidget::removeCurrentPost()
{
    if (KMessageBox::warningYesNo(this, i18n("Are you sure you want to remove this post from the server?")) == KMessageBox::Yes) {
        connect(d->mCurrentAccount->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)));
        connect(d->mCurrentAccount->microblog(),
                SIGNAL(errorPost(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)),
                this, SLOT(slotPostError(Choqok::Account*,Choqok::Post*,Choqok::MicroBlog::ErrorType,QString)));
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
        return QString(BehaviorSettings::customRT()) + " @" + currentPost()->author.userName + ": " + currentPost()->content;
    } else {
        QChar re(0x267B);
        return QString(re) + " @" + currentPost()->author.userName + ": " + currentPost()->content;
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
        connect(MediaManager::self(), SIGNAL(imageFetched(QString,QPixmap)),
                this, SLOT(slotImageFetched(QString,QPixmap)));
    }
}

void PostWidget::setupAvatar()
{
    QPixmap pix = MediaManager::self()->fetchImage(d->mCurrentPost->author.profileImageUrl,
                  MediaManager::Async);
    if (!pix.isNull()) {
        avatarFetched(d->mCurrentPost->author.profileImageUrl, pix);
    } else {
        connect(MediaManager::self(), SIGNAL(imageFetched(QString,QPixmap)),
                this, SLOT(avatarFetched(QString,QPixmap)));
        connect(MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                this, SLOT(avatarFetchError(QString,QString)));
    }
}

void PostWidget::avatarFetched(const QString &remoteUrl, const QPixmap &pixmap)
{
    if (remoteUrl == d->mCurrentPost->author.profileImageUrl) {
        const QUrl url("img://profileImage");
        _mainWidget->document()->addResource(QTextDocument::ImageResource, url, pixmap);
        updateUi();
        disconnect(MediaManager::self(), SIGNAL(imageFetched(QString,QPixmap)),
                   this, SLOT(avatarFetched(QString,QPixmap)));
        disconnect(MediaManager::self(), SIGNAL(fetchError(QString,QString)),
                   this, SLOT(avatarFetchError(QString,QString)));
    }
}

void PostWidget::avatarFetchError(const QString &remoteUrl, const QString &errMsg)
{
    Q_UNUSED(errMsg);
    if (remoteUrl == d->mCurrentPost->author.profileImageUrl) {
        ///Avatar fetching is failed! but will not disconnect to get the img if it fetches later!
        const QUrl url("img://profileImage");
        _mainWidget->document()->addResource(QTextDocument::ImageResource,
                                             url, QIcon::fromTheme("image-missing").pixmap(48));
        updateUi();
    }
}

void PostWidget::slotImageFetched(const QString &remoteUrl, const QPixmap &pixmap)
{

    if (remoteUrl == d->imageUrl) {
        const QUrl url("img://postImage");
        QPixmap newPixmap = pixmap.scaled(d->mCurrentPost->mediaSizeWidth, d->mCurrentPost->mediaSizeHeight);
        _mainWidget->document()->addResource(QTextDocument::ImageResource, url, newPixmap);
        d->originalImage = pixmap;
        updateUi();
        disconnect(MediaManager::self(), SIGNAL(imageFetched(QString,QPixmap)), this, SLOT(slotImageFetched(QString,QPixmap)));
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
        disconnect(d->mCurrentAccount->microblog(), SIGNAL(postRemoved(Choqok::Account*,Choqok::Post*)),
                   this, SLOT(slotCurrentPostRemoved(Choqok::Account*,Choqok::Post*)));
        disconnect(d->mCurrentAccount->microblog(),
                   SIGNAL(errorPost(Account*,Post*,Choqok::MicroBlog::ErrorType,QString)),
                   this, SLOT(slotPostError(Account*,Post*,Choqok::MicroBlog::ErrorType,QString)));
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

