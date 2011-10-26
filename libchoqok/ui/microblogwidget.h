/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <QtGui/QWidget>
#include <QtCore/QMap>
#include <choqoktypes.h>
#include "choqok_export.h"
#include "microblog.h"
#include "choqoktabbar.h"

class QLabel;
class KTabWidget;

namespace Choqok {
class Account;

namespace UI {
class ComposerWidget;
class TimelineWidget;

/**
 * \brief MicroBlogWidget class.
 * Every MicroBlog plugin can use this or a drived class!
 *
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 */
class CHOQOK_EXPORT MicroBlogWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MicroBlogWidget( Account *account, QWidget* parent = 0);
    virtual ~MicroBlogWidget();
    virtual void initUi();
    /**
    Set a @ref Choqok::ComposerWidget on read/write accounts!
    */
    Account * currentAccount() const;

    /**
    @return Current active timeline widget
    */
    TimelineWidget * currentTimeline();

public Q_SLOTS:
    void removeOldPosts();
    /**
    @brief Manage changed settings on this timeline and forward it to all posts.
    */
    virtual void settingsChanged();

    /**
    @brief Call markAllAsRead() on all timelines
    */
    virtual void markAllAsRead();

    /**
    Call for @ref MicroBlog::updateTimelines() to update timelines!

    @see newTimelineDataRecieved()
    */
    virtual void updateTimelines();

    /**
    Using to give focus to composer()->editot() on tab change
    */
    virtual void setFocus();

Q_SIGNALS:
    /**
    @brief Emit to tell MainWindow to show this MicroBlog
    */
    void showMe();
    /**
    Emit to show a message on MainWindow::StatusBar
    */
//     void showStatusMessage( const QString & message, bool isPermanent = false );

    /**
    @brief Emit to inform MicroBlogWidget about changes on count of unread posts

    @param change changes of unread Count, can be positive or negative.
    positive means addition, and negative means subtraction
    @param sum of unread count on this blog
    */
    void updateUnreadCount( int change, int sum );

    /**
    Emitted when all timelines are loaded fine!
    @note This will use for splash screen management!
    */
    void loaded();

protected Q_SLOTS:
    /**
    Connected to @ref MicroBlog::timelineDataReceived() to update timelines

    @see updateTimelines()
    */
    virtual void newTimelineDataRecieved( Choqok::Account *theAccount, const QString& type,
                                          QList< Choqok::Post* > data );
    void slotUpdateUnreadCount( int change, TimelineWidget * widget = 0 );
    void error(Choqok::Account* theAccount, Choqok::MicroBlog::ErrorType errorType,
                                const QString &errorMsg, Choqok::MicroBlog::ErrorLevel level);
    void errorPost(Choqok::Account* theAccount, Choqok::Post*, Choqok::MicroBlog::ErrorType errorType,
                                    const QString &errorMsg, Choqok::MicroBlog::ErrorLevel level);
    void slotAbortAllJobs();

    virtual void keyPressEvent(QKeyEvent* );

    void slotAccountModified(Choqok::Account *theAccount);
protected:
    virtual QLayout *createToolbar();
    virtual TimelineWidget* addTimelineWidgetToUi( const QString &name);
    void initTimelines();

    void setComposerWidget(ComposerWidget* widget);
    ComposerWidget *composer();
    QMap<QString, TimelineWidget*> &timelines();
    Choqok::UI::ChoqokTabBar *timelinesTabWidget();
    QLabel *latestUpdate();

private:
    class Private;
    Private * const d;
};
}
}
#endif // MICROBLOGWIDGET_H
