/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TIMELINEWIDGET_H
#define TIMELINEWIDGET_H

#include <QIcon>
#include <QMap>
#include <QWidget>

#include "choqoktypes.h"
#include "choqok_export.h"

class QLabel;
class QHBoxLayout;
class QVBoxLayout;

namespace Choqok
{
class Account;

namespace UI
{

class PostWidget;
/**
@brief Choqok base Timeline Widget

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT TimelineWidget : public QWidget
{
    Q_OBJECT
public:
    TimelineWidget(Account *account, const QString &timelineName, QWidget *parent = nullptr);
    virtual ~TimelineWidget();
    void setTimelineName(const QString &type);

    /**
    @brief Return Timeline name
    Related to whatever sets previouslly by @ref setTimelineName()
    */
    QString timelineName();

    /**
    @brief Return Timeline name for UI
    */
    QString timelineInfoName();

    /**
    @brief Return Timeline icon name
    */
    QString timelineIconName();

    /**
    @brief Return Timeline icon
    If timelineIconName() is empty this can be useful
     */
    QIcon &timelineIcon() const;
    void setTimelineIcon(const QIcon &icon);

    /**
     @brief Add new posts to UI.
    */
    virtual void addNewPosts(QList< Choqok::Post * > &postList);

    /**
     @brief Adds a message in place of an empty timeline. Placeholder is removed when a post gets added.
    */
    virtual void addPlaceholderMessage(const QString &message);

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
    QList<PostWidget *> postWidgets();

    /**
     * @return true if this timeline is closable!
     */
    bool isClosable() const;

    void setClosable(bool isClosable = true);

public Q_SLOTS:
    /**
    @brief Mark all posts as read
    */
    virtual void markAllAsRead();
    /**
    @brief Manage changed settings on this timeline and forward it to all posts.
    */
    virtual void settingsChanged();
    /**
    @brief Scroll to the bottom of the timeline
    */
    virtual void scrollToBottom();

Q_SIGNALS:
    void forwardResendPost(const QString &post);
    void forwardReply(const QString &txt, const QString &replyToId, const QString &replyToUsername);
    /**
    @brief Emit to inform MicroBlogWidget about changes on count of unread posts

    @param change changes of unread Count, can be positive or negative.
        positive means addition, and negative means subtraction
    */
    void updateUnreadCount(int change);

protected Q_SLOTS:
    void slotOnePostReaded();
    virtual void saveTimeline();
    virtual void loadTimeline();
    void postWidgetClosed(const QString &postId, PostWidget *widget);

protected:
    /**
    Add a PostWidget to UI
    @Note This will call @ref PostWidget::initUi()
    */
    virtual void addPostWidgetToUi(PostWidget *widget);
    Account *currentAccount();
    QMap<QString, PostWidget *> &posts() const;
    QMultiMap<QDateTime, PostWidget *> &sortedPostsList() const;

    QVBoxLayout *mainLayout();
    QHBoxLayout *titleBarLayout();
    QLabel *timelineDescription();
    virtual void setUnreadCount(int unread);
    virtual void showMarkAllAsReadButton();

private:
    void setupUi();
    class Private;
    Private *const d;
};
}
}
#endif // TIMELINEWIDGET_H
