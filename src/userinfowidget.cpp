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

#include "userinfowidget.h"
#include "mediamanager.h"
#include <KTextBrowser>
#include <QLayout>
#include <KUrl>
#include <kicon.h>
#include <settings.h>
#include <KNotification>
#include <KProcess>
#include <KToolInvocation>
#include "statuswidget.h"
#include <KApplication>
#include <QDesktopWidget>

UserInfoWidget::UserInfoWidget(const User& user, QWidget* parent)
    : QFrame(parent), mUser(user)
{
    w = new KTextBrowser(this);
    this->setFrameShape(StyledPanel);
    this->setFrameShadow(Sunken);
    w->setFrameShape(QFrame::NoFrame);
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0,0,0,0);
    layout->addWidget(w);
    this->setLayout(layout);
    this->setWindowFlags(Qt::Popup);// | Qt::FramelessWindowHint | Qt::Ta);
    setAttribute(Qt::WA_DeleteOnClose);
    w->setOpenLinks(false);
    connect(w,SIGNAL(anchorClicked(const QUrl)),this,SLOT(checkAnchor(const QUrl)));
    setupUi();
}

UserInfoWidget::~UserInfoWidget()
{}

void UserInfoWidget::show(QPoint pos)
{
    w->resize(270, 200);
    w->document()->setTextWidth(width()-2);
    int h = w->document()->size().toSize().height()+2;
    w->setMinimumHeight(h);
    w->setMaximumHeight(h);
    this->resize(270,h+2);
    int desktopHeight = KApplication::desktop()->height();
    int desktopWidth = KApplication::desktop()->width();
    if( (pos.x() + this->width()) > desktopWidth )
        pos.setX(desktopWidth - width());
    if( (pos.y() + this->height()) > desktopHeight )
        pos.setY(desktopHeight - height());
    move(pos);
    QWidget::show();
}

void UserInfoWidget::checkAnchor( const QUrl url )
{
    if(url.scheme()=="choqok"){
        if(url.host()=="close")
            this->close();
    } else {
        if( Settings::useCustomBrowser() ) {
            QStringList args = Settings::customBrowser().split(' ');
            args.append(url.toString());
            if( KProcess::startDetached( args ) == 0 ) {
                KNotification *notif = new KNotification( "notify", this );
                notif->setText( i18n("Could not launch custom browser.\nUsing KDE default browser.") );
                notif->sendEvent();
                KToolInvocation::invokeBrowser(url.toString());
            }
        } else {
            KToolInvocation::invokeBrowser(url.toString());
        }
        close();
    }
}

void UserInfoWidget::setupUi()
{
    //<tr><td style=\"font-size:small;\" align=\"right\">%4</td></tr>
    QString info = i18n( "<table width=\"100%\">\
    <tr><td><b><i>Who is %5?</i></b><a href='choqok://close'><img src='icon://close' align='right' /></a></td></tr>\
    <tr><td><table><tr><td width=\"48\"><img width=48 height=48 src='img://profileImage'/></td>\
    <td><p align='left'><b>Name:</b> %1<br/>\
    <b>Location:</b> %2<br/>\
    <b>Web:</b> %3<br/>\
    <b>Bio:</b> %4\
    </p></td></tr></table></td></tr></table>" );
    w->document()->addResource( QTextDocument::ImageResource, QUrl("img://profileImage"),
                             *(MediaManager::self()->getAvatarIfExist( KUrl( mUser.profileImageUrl ) )) );
    w->document()->addResource( QTextDocument::ImageResource, QUrl("icon://close"),
                            KIcon("dialog-close").pixmap(16) );
    QString url = mUser.homePageUrl.isEmpty() ?
                  QString() : QString("<a title='%1' href='%1'>%1</a>").arg(mUser.homePageUrl);
    w->setHtml( info.arg( mUser.name ).arg( mUser.location ).arg( url ).arg( mUser.description ).arg(mUser.screenName) );

    QString style = "color: %1; background-color: %2";
    if ( Settings::isCustomUi() ) {
        setStyleSheet( style.arg( Settings::defaultForeColor().name()).arg(Settings::defaultBackColor().name()) );
    } else {
        QPalette p = window()->palette();
        setStyleSheet( style.arg(p.color(QPalette::WindowText).name()).arg(p.color(QPalette::Window).name()) );
    }
}

#include "userinfowidget.moc"