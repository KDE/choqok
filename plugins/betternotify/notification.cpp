/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "notification.h"

#include <QIcon>
#include <QMouseEvent>
#include <QVBoxLayout>

#include <KLocalizedString>

#include "choqokappearancesettings.h"
#include "choqoktools.h"
#include "mediamanager.h"

#include "notifysettings.h"

const QRegExp Notification::dirRegExp(QLatin1String("(RT|RD)|(@([^\\s\\W]+))|(#([^\\s\\W]+))|(!([^\\s\\W]+))"));

Notification::Notification(Choqok::UI::PostWidget *postWidget)
    : QWidget(), post(postWidget), dir(QLatin1String("ltr"))
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
//     setAttribute(Qt::WA_TranslucentBackground);
    setWindowOpacity(0.8);
    setWindowFlags(Qt::ToolTip);
    setDirection();
    mainWidget.viewport()->setAutoFillBackground(false);
    mainWidget.setFrameShape(QFrame::NoFrame);
    mainWidget.setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainWidget.setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    mainWidget.setOpenExternalLinks(false);
    mainWidget.setOpenLinks(false);
    setMouseTracking(true);
    resize(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);

    NotifySettings set(this);
    QFont fnt = set.font();
    QColor color = set.foregroundColor();
    QColor back = set.backgroundColor();

    QString fntStr = QLatin1String("font-family:\"") + fnt.family() + QLatin1String("\"; font-size:") + QString::number(fnt.pointSize()) + QLatin1String("pt;");
    fntStr += (fnt.bold() ? QLatin1String(" font-weight:bold;") : QString()) + (fnt.italic() ? QLatin1String(" font-style:italic;") : QString());
    QString style = Choqok::UI::PostWidget::getBaseStyle().arg(Choqok::getColorString(color), Choqok::getColorString(back), fntStr);

    setStyleSheet(style);

    init();
    connect(&mainWidget, &MyTextBrowser::anchorClicked, this, &Notification::slotProcessAnchor);
}

Notification::~Notification()
{
}

QSize Notification::sizeHint() const
{
    return QSize(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);
}

void Notification::init()
{
    QPixmap pix = Choqok::MediaManager::self()->fetchImage(post->currentPost()->author.profileImageUrl);
    if (!pix) {
        pix = QPixmap(Choqok::MediaManager::self()->defaultImage());
    }
    mainWidget.document()->addResource(QTextDocument::ImageResource, QUrl(QLatin1String("img://profileImage")), pix);
    mainWidget.document()->addResource(QTextDocument::ImageResource, QUrl(QLatin1String("icon://close")),
                                       QIcon::fromTheme(QLatin1String("dialog-close")).pixmap(16));
    mainWidget.setText(baseText.arg(post->currentPost()->author.userName)
                       .arg(post->currentPost()->content)
                       .arg(dir)
                       .arg(i18n("Ignore")));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);
    l->setSpacing(0);
    l->addWidget(&mainWidget);

    setHeight();

    connect(&mainWidget, &MyTextBrowser::clicked, this, &Notification::slotClicked);
    connect(&mainWidget, &MyTextBrowser::mouseEntered, this, &Notification::mouseEntered);
    connect(&mainWidget, &MyTextBrowser::mouseLeaved, this, &Notification::mouseLeaved);

    //TODO Show remaining post count to notify
}

void Notification::slotProcessAnchor(const QUrl &url)
{
    if (url.scheme() == QLatin1String("choqok")) {
        if (url.host() == QLatin1String("close")) {
            Q_EMIT ignored();
        }
    }
}

void Notification::slotClicked()
{
    post->setReadWithSignal();
    Q_EMIT postReaded();
}

void Notification::setHeight()
{
    mainWidget.document()->setTextWidth(mainWidget.width() - 2);
    int h = mainWidget.document()->size().toSize().height() + 30;
    setMinimumHeight(h);
    setMaximumHeight(h);
}

void Notification::setDirection()
{
    QString txt = post->currentPost()->content;
    txt.remove(dirRegExp);
    txt = txt.trimmed();
    if (txt.isRightToLeft()) {
        dir = QLatin1String("rtl");
    }
}

void Notification::mouseMoveEvent(QMouseEvent *e)
{
    e->accept();
}

#include "moc_notification.cpp"
