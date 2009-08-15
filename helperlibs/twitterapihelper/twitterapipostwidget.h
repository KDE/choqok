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

#ifndef TWITTERAPIPOSTWIDGET_H
#define TWITTERAPIPOSTWIDGET_H

#include <postwidget.h>

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT TwitterApiPostWidget : public Choqok::UI::PostWidget
{
    Q_OBJECT
public:
    TwitterApiPostWidget(Choqok::Account* account, const Choqok::Post &post, QWidget* parent = 0);
    ~TwitterApiPostWidget();
    virtual void initUi();

protected slots:
    virtual void checkAnchor(const QUrl & url);
    virtual void setFavorite();
    virtual void slotSetFavorite(Choqok::Account *theAccount, const QString &postId);
    virtual void slotReply();
    void slotBasePostFetched(Choqok::Account* theAccount, Choqok::Post* post);

protected:
//     virtual void updateUi();
    virtual QString prepareStatus(const QString& text);
    virtual QString generateSign();
    void updateFavStat();

    static const QRegExp mUserRegExp;
    static const QRegExp mHashtagRegExp;
    static const KIcon unFavIcon;
private:
    class Private;
    Private *d;

};

#endif // TWITTERPOSTWIDGET_H
