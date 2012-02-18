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

#ifndef LACONICAPOSTWIDGET_H
#define LACONICAPOSTWIDGET_H
#include <twitterapihelper/twitterapipostwidget.h>

class LaconicaPostWidget : public TwitterApiPostWidget
{
public:
    LaconicaPostWidget(Choqok::Account* account, const Choqok::Post& post, QWidget* parent = 0);
    ~LaconicaPostWidget();
    virtual void initUi();

protected:
    static const QRegExp mGroupRegExp;
    static const QRegExp mLaconicaUserRegExp;
    static const QRegExp mLaconicaHashRegExp;
    static const QRegExp mStatusNetUserRegExp;
    virtual QString prepareStatus(const QString& text);
    virtual void checkAnchor(const QUrl& url);
    virtual QString generateSign();
    virtual void slotReplyToAll();

protected slots:
    virtual void slotResendPost();

private:
    class Private;
    Private *d;
};

#endif // LACONICAPOSTWIDGET_H
