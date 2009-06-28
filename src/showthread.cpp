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

#include "showthread.h"
#include "statuswidget.h"
#include "backend.h"

ShowThread::ShowThread(Account* account, const qulonglong &finalStatus, QWidget *parent )
    : QWidget( parent ), mAccount(account), mStatus(finalStatus)
{
	setMinimumWidth(300);
	setMinimumHeight(300);

	setSizePolicy(QSizePolicy::Minimum, QSizePolicy::MinimumExpanding);

	layout = new QVBoxLayout;
	layout->setDirection(QBoxLayout::BottomToTop);
	layout->setSpacing(1);

	verticalSpacer = new QSpacerItem(20, 0, QSizePolicy::Minimum, QSizePolicy::Expanding);
	layout->addItem(verticalSpacer);

	setLayout(layout);

	backend = new Backend(account, this);
	connect(backend, SIGNAL(singleStatusReceived(Status)), this, SLOT(newStatusReceived(Status)));

	connect(this, SIGNAL(close()), this, SLOT(deleteLater()));
}

ShowThread::~ShowThread()
{
	delete backend;
}

void ShowThread::addStatusToThread(const qulonglong &status)
{
	backend->requestSingleStatus(status);
}

void ShowThread::startPopulate()
{
	addStatusToThread(mStatus);
}

void ShowThread::newStatusReceived(const Status &status)
{

        StatusWidget *wt = new StatusWidget( mAccount, this );

        connect( wt, SIGNAL( sigReply( const QString&, qulonglong, bool ) ),
                 this, SIGNAL( forwardReply( const QString&, qulonglong, bool ) ) );
        connect( wt, SIGNAL(sigReTweet(const QString&)), SIGNAL(forwardReTweet(const QString&)));
        connect( wt, SIGNAL( sigFavorite( qulonglong, bool ) ),
                 this, SIGNAL( forwardFavorited( qulonglong, bool ) ) );
        connect (wt,SIGNAL(sigSearch(int,QString)),this,SLOT(updateSearchArea(int,QString)));

        wt->setAttribute( Qt::WA_DeleteOnClose );
        wt->setCurrentStatus( status );
        wt->setUnread( StatusWidget::WithoutNotify );

        layout->addWidget( wt );

	if(status.replyToStatusId)
		addStatusToThread(status.replyToStatusId);
	else
		emit finishedPopulate();
}
