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

#ifndef MICROBLOGWIDGET_H
#define MICROBLOGWIDGET_H

#include <QWidget>
#include "choqok_export.h"
#include <choqoktypes.h>
#include <QMap>

class KTabWidget;

namespace Choqok {
class Account;

namespace UI {
class ComposerWidget;
class TimelineWidget;

/**
 * \brief MicroBlogWidget class.
 * Every MicroBlog plugin can use this or a drived class!
 * ComposerWidget should set on @ref MicroBlog::createMicroBlogWidget if account is not read only
 *
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 */
class CHOQOK_EXPORT MicroBlogWidget : public QWidget
{
    Q_OBJECT
public:
    MicroBlogWidget( Account *account, QWidget* parent = 0, Qt::WindowFlags f = 0);
    virtual ~MicroBlogWidget();
    /**
    Set a @ref Choqok::ComposerWidget on read/write accounts!
    */
    void setComposerWidget(ComposerWidget* widget);
    Account * currentAccount() const;

public slots:
    void removeOldPosts();
    /**
    @brief Manage changed settings on this timeline and forward it to all posts.
    */
    virtual void settingsChanged();
    /**
    Call for @ref MicroBlog::updateTimelines() to update timelines!

    @see newTimelineDataRecieved()
    */
    virtual void updateTimelines();

protected slots:
    /**
    Connected to @ref MicroBlog::timelineDataReceived() to update timelines

    @see updateTimelines()
    */
    virtual void newTimelineDataRecieved( Account *theAccount, const QString& type, QList< Choqok::Post* > data );
    void slotUpdateUnreadCount( int change );
    void slotMarkAllAsRead();

signals:
    /**
    @brief Emit to tell MainWindow to show this MicroBlog
    */
    void showMe();
    /**
    @brief Emit to tell timelines to mark all posts as read
    */
    void markAllAsRead();
    /**
    Emit to show a message on MainWindow::StatusBar
    */
    void showStatusMessage( const QString & message, bool isPermanent = false );

    /**
    @brief Emit to inform MicroBlogWidget about changes on count of unread posts

    @param change changes of unread Count, can be positive or negative.
    positive means addition, and negative means subtraction
    @param sum of unread count on this blog
    */
    void updateUnreadCount( int change, int sum );

protected:
    virtual void setupUi();
    virtual TimelineWidget* addTimelineWidgetToUi( const QString &name);
    void initTimelines();

    ComposerWidget *composer();
    QMap<QString, TimelineWidget*> timelines();
    QMap<TimelineWidget*, int> timelineUnreadCount();
    KTabWidget *timelinesTabWidget();

private:
    class Private;
    Private *d;
};
}
}
#endif // MICROBLOGWIDGET_H
