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

#ifndef TWITTERAPISHOWTHREAD_H
#define TWITTERAPISHOWTHREAD_H

#include <QtGui/QWidget>
#include <choqoktypes.h>

namespace Choqok {
class Account;
namespace UI{
class PostWidget;
}
}

class TwitterApiAccount;
class CHOQOK_HELPER_EXPORT TwitterApiShowThread : public QWidget
{
    Q_OBJECT
public:
    TwitterApiShowThread( Choqok::Account *account, const Choqok::Post &finalPost, QWidget* parent = 0);
    ~TwitterApiShowThread();

protected Q_SLOTS:
    void slotAddNewPost( Choqok::Account *theAccount, Choqok::Post *post );
    void raiseMainWindow();

Q_SIGNALS:
    void forwardResendPost( const QString &post );
    void forwardReply(const QString &txt, const QString &replyToId);

protected:
    void addPostWidgetToUi(Choqok::UI::PostWidget *widget);
private:
    void setupUi();

    class Private;
    Private * const d;
};

#endif // TWITTERAPISHOWTHREAD_H
