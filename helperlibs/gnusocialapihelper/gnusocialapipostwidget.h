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

#ifndef GNUSOCIALAPIPOSTWIDGET_H
#define GNUSOCIALAPIPOSTWIDGET_H

#include "twitterapipostwidget.h"

class CHOQOK_HELPER_EXPORT GNUSocialApiPostWidget : public TwitterApiPostWidget
{
public:
    GNUSocialApiPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent = nullptr);
    ~GNUSocialApiPostWidget();
    virtual void initUi() override;

protected:
    static const QRegExp mGroupRegExp;
    static const QRegExp mGNUSocialApiUserRegExp;
    static const QRegExp mGNUSocialApiHashRegExp;
    static const QRegExp mStatusNetUserRegExp;
    virtual QString prepareStatus(const QString &text) override;
    virtual void checkAnchor(const QUrl &url) override;
    virtual QString generateSign() override;
    virtual void slotReplyToAll() override;

protected Q_SLOTS:
    virtual void slotResendPost() override;

private:
    class Private;
    Private *d;
};

#endif // GNUSOCIALAPIPOSTWIDGET_H
