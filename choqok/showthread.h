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

#ifndef SHOWTHREAD_H
#define SHOWTHREAD_H

#include <QString>
#include <QObject>
#include <QMap>
#include <QPair>
#include <KUrl>
#include <QWidget>
#include <QVBoxLayout>
#include <QScrollArea>

#include "datacontainers.h"

#include "ui_showthread_base.h"

class Account;
class Backend;

/**
    Base class for thread feature.
    @author Tejas Dinkar <tejas@gja.in>
*/
class ShowThread : public QWidget
{
    Q_OBJECT
public:
    explicit ShowThread(Account* account, const qulonglong &finalStatus, QWidget *parent=0 );
    virtual ~ShowThread();

public slots:
    void startPopulate();

protected slots:
    void newStatusReceived(const Status &status);

signals:
    void finishedPopulate();
    void forwardReply( const QString &username, qulonglong statusId, bool dMsg );
    void forwardFavorited( qulonglong statusId, bool isFavorite );
    void forwardReTweet( const QString &text );
    void forwardSigSearch(int, const QString &text );

protected:
    void addStatusToThread(const qulonglong &status);

    Account *mAccount;

    Backend *backend;

    qulonglong mStatus;

    Ui::showthread_base ui;
};

#endif
