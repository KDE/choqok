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
#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QtGui/QWidget>
#include "choqok_export.h"
#include <choqoktypes.h>
#include <QMap>

class QLabel;
class QHBoxLayout;
class QVBoxLayout;

namespace Choqok {
class Account;

namespace UI {

class PostWidget;
/**
@brief Choqok base Timeline Widget

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    TimelineWidget(Account *account, const QString &timelineName, QWidget* parent = 0);
    virtual ~TimelineWidget();
    void setTimelineName(const QString &type);

    /**
    @brief Return Timeline name
    Related to whatever sets previouslly by @ref setTimelineName()
    */
    QString timelineName();

    /**
     @brief Add new posts to UI.
    */
    virtual void addNewPosts( QList< Choqok::Post* >& postList );

    /**
    @brief Return count of unread posts on this timeline.
    */
    int unreadCount() const;

    /**
    @brief remove old posts, about to user selected count of posts on timelines
    */
    void removeOldPosts();

    /**
    @return list of all widgets available on this timeline
    */
    QList<PostWidget*> postWidgets();

public slots:
    /**
    @brief Mark all posts as read
    */
    virtual void markAllAsRead();
    /**
    @brief Manage changed settings on this timeline and forward it to all posts.
    */
    virtual void settingsChanged();

signals:
    void forwardResendPost( const QString &post );
    void forwardReply(const QString &txt, const QString &replyToId);
    /**
    @brief Emit to inform MicroBlogWidget about changes on count of unread posts

    @param change changes of unread Count, can be positive or negative.
        positive means addition, and negative means subtraction
    */
    void updateUnreadCount(int change);

protected slots:
    void slotOnePostReaded();
    virtual void saveTimeline();
    virtual void loadTimeline();
    void postWidgetDestroyed( QObject* );

protected:
    /**
    Add a PostWidget to UI
    @Note This will call @ref PostWidget::initUi()
    */
    virtual void addPostWidgetToUi(PostWidget *widget);
    Account *currentAccount();
    QMap<ChoqokId, PostWidget *> &posts() const;

    QVBoxLayout *mainLayout();
    QHBoxLayout *titleBarLayout();
    QLabel *timelineDescription();

private:
    void setupUi();
    class Private;
    Private *d;
};
}
}
#endif // TIMELINEWIDGET_H
