/*
    This file is part of Choqok, the KDE micro-blogging client
    Copyright (C) 2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include <mediamanager.h>
#include <qevent.h>
#include <QVBoxLayout>
#include <KLocalizedString>
#include <KIcon>
#include <KDebug>

const QString Notification::baseText( "<table height=\"100%\" width=\"100%\"><tr><td rowspan=\"2\"\
width=\"48\"><img src=\"img://profileImage\" width=\"48\" height=\"48\" /></td><td width=\"5\"><!-- EMPTY HAHA --></td><td><b>%1 :</b><a href='choqok://close'><img src='icon://close' title='" + i18n("Ignore notifications") + "' align='right' /></a><div dir=\"%3\">%2</div></td></tr></table>" );

const QRegExp Notification::dirRegExp("(RT|RD)|(@([^\\s\\W]+))|(#([^\\s\\W]+))|(!([^\\s\\W]+))");

Notification::Notification(Choqok::UI::PostWidget* postWidget)
: QWidget(), post(postWidget), dir("ltr")
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
    setStyleSheet( "KTextBrowser{background-color: rgba(0, 0, 0, 160); color: rgb(255, 255, 255);}"
                   "QWidget{border-radius:6px;}");
    resize(300, 70);
    init();
    connect(&mainWidget, SIGNAL(anchorClicked(QUrl)), SLOT(slotProcessAnchor(QUrl)));
}

Notification::~Notification()
{

}

QSize Notification::sizeHint() const
{
    return QSize(300, 70);
}

void Notification::init()
{
    QPixmap *pix = Choqok::MediaManager::self()->fetchImage(post->currentPost().author.profileImageUrl);
    if( !pix )
        pix = new QPixmap( Choqok::MediaManager::self()->defaultImage() );
    mainWidget.document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"), *pix );
    mainWidget.document()->addResource( QTextDocument::ImageResource, QUrl("icon://close"),
                                        KIcon("dialog-close").pixmap(16) );
    mainWidget.setText(baseText.arg(post->currentPost().author.userName).arg(post->currentPost().content).arg(dir));

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
    if(url.scheme() == "choqok"){
        if(url.host() == "close"){
            emit ignored();
        }
    }
}

void Notification::slotClicked()
{
    post->setReadWithSignal();
    emit postReaded();
}

void Notification::setHeight()
{
    mainWidget.document()->setTextWidth(mainWidget.width()-2);
    int h = mainWidget.document()->size().toSize().height() + 30;
    setMinimumHeight(h);
    setMaximumHeight(h);
}

void Notification::setDirection()
{
    QString txt = post->currentPost().content;
    txt.remove(dirRegExp);
    txt = txt.trimmed();
    if( txt.isRightToLeft() ) {
        dir = "rtl";
    }
}

void Notification::mouseMoveEvent(QMouseEvent* e)
{
    kDebug()<<e->pos();
    e->accept();
}

#include "notification.moc"
