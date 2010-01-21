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

#ifndef TWITTERAPIMICROBLOGWIDGET_H
#define TWITTERAPIMICROBLOGWIDGET_H

#include "microblogwidget.h"
#include "twitterapisearch.h"

class TwitterApiSearchTimelineWidget;

class CHOQOK_HELPER_EXPORT TwitterApiMicroBlogWidget : public Choqok::UI::MicroBlogWidget
{
    Q_OBJECT
public:
    TwitterApiMicroBlogWidget(Choqok::Account* account, QWidget* parent = 0);
    ~TwitterApiMicroBlogWidget();
    virtual void initUi();

public slots:
    virtual void markAllAsRead();

protected slots:
    void slotAccountModified(Choqok::Account *account);
    void slotCloseCurrentSearch();
    void slotCurrentTimelineChanged(int);
    virtual void slotSearchResultsReceived( const SearchInfo &info,
                                            QList<Choqok::Post*> &postsList );

protected:
    QMap<QString, TwitterApiSearchTimelineWidget*> mSearchTimelines;
    TwitterApiSearchTimelineWidget* addSearchTimelineWidgetToUi(const QString& name,
                                                                const SearchInfo &info);
private:
    class Private;
    Private *d;
};

#endif // TWITTERAPIMICROBLOGWIDGET_H
