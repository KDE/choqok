/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MICROBLOGWIDGET_H
#define MICROBLOGWIDGET_H

#include <QMap>
#include <QWidget>

#include "choqoktabbar.h"
#include "choqoktypes.h"
#include "microblog.h"
#include "choqok_export.h"

class QLabel;

namespace Choqok
{
class Account;

namespace UI
{
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
    explicit MicroBlogWidget(Account *account, QWidget *parent = nullptr);
    virtual ~MicroBlogWidget();
    virtual void initUi();
    /**
    Set a @ref Choqok::ComposerWidget on read/write accounts!
    */
    Account *currentAccount() const;

    /**
    @return Current active timeline widget
    */
    TimelineWidget *currentTimeline();

    /**
     * @return the number of unread posts between all timelines.
     */
    uint unreadCount() const;

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
    void updateUnreadCount(int change, int sum);

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
    virtual void newTimelineDataRecieved(Choqok::Account *theAccount, const QString &type,
                                         QList< Choqok::Post * > data);
    void slotUpdateUnreadCount(int change, TimelineWidget *widget = nullptr);
    void error(Choqok::Account *theAccount, Choqok::MicroBlog::ErrorType errorType,
               const QString &errorMsg, Choqok::MicroBlog::ErrorLevel level);
    void errorPost(Choqok::Account *theAccount, Choqok::Post *, Choqok::MicroBlog::ErrorType errorType,
                   const QString &errorMsg, Choqok::MicroBlog::ErrorLevel level);
    void slotAbortAllJobs();

    virtual void keyPressEvent(QKeyEvent *) override;

    void slotAccountModified(Choqok::Account *theAccount);
protected:
    virtual QLayout *createToolbar();
    virtual TimelineWidget *addTimelineWidgetToUi(const QString &name);
    void initTimelines();

    void setComposerWidget(ComposerWidget *widget);
    ComposerWidget *composer();
    QMap<QString, TimelineWidget *> &timelines();
    Choqok::UI::ChoqokTabBar *timelinesTabWidget();
    QLabel *latestUpdate();

private:
    class Private;
    Private *const d;
};
}
}
#endif // MICROBLOGWIDGET_H
