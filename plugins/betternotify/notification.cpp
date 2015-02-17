/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or
    modify it under the terms of the GNU General Public License as
    published by the Free Software Foundation; either version 2 of
    the License or (at your option) version 3 or any later version
    accepted by the membership of KDE e.V. (or its successor approved
    by the membership of KDE e.V.), which shall act as a proxy
    defined in Section 14 of version 3 of the license.


    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#include "notification.h"

#include <QMouseEvent>
#include <QVBoxLayout>

#include "choqokdebug.h"
#include <QIcon>
#include <KLocalizedString>

#include "choqokappearancesettings.h"
#include "choqoktools.h"
#include "mediamanager.h"
#include "notifysettings.h"

const QRegExp Notification::dirRegExp("(RT|RD)|(@([^\\s\\W]+))|(#([^\\s\\W]+))|(!([^\\s\\W]+))");

Notification::Notification(Choqok::UI::PostWidget* postWidget)
: QWidget(), post(postWidget), dir("ltr")
{
    qCDebug(CHOQOK);
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

    QString fntStr = "font-family:\"" + fnt.family() + "\"; font-size:" + QString::number(fnt.pointSize()) + "pt;";
    fntStr += (fnt.bold() ? " font-weight:bold;" : QString()) + (fnt.italic() ? " font-style:italic;" : QString());
    QString style = Choqok::UI::PostWidget::getBaseStyle().arg( Choqok::getColorString(color), Choqok::getColorString(back), fntStr);

    setStyleSheet( style );

    init();
    connect(&mainWidget, SIGNAL(anchorClicked(QUrl)), SLOT(slotProcessAnchor(QUrl)));
}

Notification::~Notification()
{
    qCDebug(CHOQOK);
}

QSize Notification::sizeHint() const
{
    qCDebug(CHOQOK);
    return QSize(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);
}

void Notification::init()
{
    qCDebug(CHOQOK);
    QPixmap pix = Choqok::MediaManager::self()->fetchImage(post->currentPost()->author.profileImageUrl);
    if( !pix )
        pix = QPixmap( Choqok::MediaManager::self()->defaultImage() );
    mainWidget.document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"), pix );
    mainWidget.document()->addResource( QTextDocument::ImageResource, QUrl("icon://close"),
                                        QIcon::fromTheme("dialog-close").pixmap(16) );
    mainWidget.setText(baseText.arg(post->currentPost()->author.userName)
                               .arg(post->currentPost()->content)
                               .arg(dir)
                               .arg(i18n("Ignore")));

    QVBoxLayout *l = new QVBoxLayout(this);
    l->setContentsMargins(0,0,0,0);
    l->setSpacing(0);
    l->addWidget(&mainWidget);

    setHeight();

    connect(&mainWidget, SIGNAL(clicked()), SLOT(slotClicked()));
    connect(&mainWidget, SIGNAL(mouseEntered()), SIGNAL(mouseEntered()));
    connect(&mainWidget, SIGNAL(mouseLeaved()), SIGNAL(mouseLeaved()));

    //TODO Show remaining post count to notify
}

void Notification::slotProcessAnchor(const QUrl& url)
{
    qCDebug(CHOQOK);
    if(url.scheme() == "choqok"){
        if(url.host() == "close"){
            Q_EMIT ignored();
        }
    }
}

void Notification::slotClicked()
{
    qCDebug(CHOQOK);
    post->setReadWithSignal();
    Q_EMIT postReaded();
}

void Notification::setHeight()
{
    qCDebug(CHOQOK);
    mainWidget.document()->setTextWidth(mainWidget.width()-2);
    int h = mainWidget.document()->size().toSize().height() + 30;
    setMinimumHeight(h);
    setMaximumHeight(h);
}

void Notification::setDirection()
{
    qCDebug(CHOQOK);
    QString txt = post->currentPost()->content;
    txt.remove(dirRegExp);
    txt = txt.trimmed();
    if( txt.isRightToLeft() ) {
        dir = "rtl";
    }
}

void Notification::mouseMoveEvent(QMouseEvent* e)
{
    qCDebug(CHOQOK);
    e->accept();
}

