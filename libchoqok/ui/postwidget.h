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
#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <QtCore/QTimer>
#include <KDE/KDateTime>
#include <kicon.h>
#include <account.h>
#include <choqoktypes.h>
#include <microblog.h>

class KAction;
class QGridLayout;
class KPushButton;

namespace Choqok {
namespace UI {

class TextBrowser;

/**
Post Widget!
Attribute "Qt::WA_DeleteOnClose" is enabled at construtor! So please use close() for deleting an object, instead of deleteLater() or delete

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT PostWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY (bool read READ isRead)
public:
    explicit PostWidget( Account *account, const Post &post, QWidget *parent = 0 );
    virtual ~PostWidget();
    const Post &currentPost() const;
    virtual void setRead(bool read = true);

    /**
     Sets post widget as read, and emits postReaded() signal
     */
    void setReadWithSignal();

    virtual bool isRead() const;

    Account *currentAccount();
    /**
    Setup UI elements
    @Note Should call from outside of class, To initialize drived class items too!
    */
    virtual void initUi();

    /**
    Set stylesheet data with new color data! to use later.

    @see setUiStyle()
    */
    static void setStyle( const QColor& unreadColor, const QColor& unreadBack,
                          const QColor& readColor, const QColor& readBack,
                          const QColor& ownColor, const QColor& ownBack,
                          const QFont& font);

    /**
    @brief Set current post

    @note Use with care!

    After changing current post, Don't forget to call @ref initUi() to update post UI.
    */
    void setCurrentPost( const Post &post );

    /**
    @brief Sets Post sign
    sign is the text that showed as sign of this post under post content.
    */
    void setSign( const QString &sign );

    /**
    @return post sign
    sign is an html text that showed as sign of this post under post content.
    */
    QString sign() const;

    /**
    @brief Sets post content
    Post content is an html text that showed as post text.
    */
    void setContent( const QString &content );

    /**
    @return post content
    Post content is an html text that showed as post text.
    */
    QString content() const;

    void deleteLater();

    TextBrowser * mainWidget();

    QStringList urls();

    TimelineWidget *timelineWidget() const;

    /**
     * Plugins can add status specific actions and process them internally
     * 
     */
    static void addAction( KAction *action );

public Q_SLOTS:
    /**
    Set Style sheet of widget to corresponding data->
    @see setStyle()
    */
    void setUiStyle();

Q_SIGNALS:
    /**
    Emit and contain text to resend.
    */
    void resendPost(const QString &text);
    /**
    Emit when this post has been readed by pressing mouse on it, And to notify TimelineWidget about it.
    */
    void postReaded();
    /**
    Carry reply information, to reply to a post.
    */
    void reply(const QString &txt, const QString &replyToId);

    /**
    Emitted when this widget is about to close!
    postId and this returned!
    */
    void aboutClosing( const ChoqokId &postId, PostWidget *widget);

protected Q_SLOTS:

    virtual void checkAnchor(const QUrl & url);
    /**
    Set height of widget related to text contents
    */
    virtual void setHeight();

    /**
    Update UI after changes, such as timestamp
    */
    virtual void updateUi();

    /**
    Call microblog() to remove this post!
    */
    virtual void removeCurrentPost();
    /**
    Prepare text to send for resending this post.
    */
    virtual void slotResendPost();
    /**
    Internal slot to remove/close/destroy this post after bing deleted
    */
    void slotCurrentPostRemoved( Choqok::Account *theAccount, Choqok::Post *post );

    virtual void slotPostError( Choqok::Account *theAccount, Choqok::Post *post,
                                Choqok::MicroBlog::ErrorType error, const QString &errorMessage);

    void avatarFetchError( const QString &remoteUrl, const QString &errMsg );
    void avatarFetched( const QString &remoteUrl, const QPixmap &pixmap );

    virtual void mousePressEvent(QMouseEvent* ev);
protected:
    virtual void setupUi();
    virtual void closeEvent(QCloseEvent* event);
    virtual void setupAvatar();
    virtual void wheelEvent(QWheelEvent* );
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void enterEvent ( QEvent * event );
    virtual void leaveEvent ( QEvent * event );
    virtual QString prepareStatus( const QString &text );
    virtual void setDirection();
    virtual QString generateSign();
    virtual QString formatDateTime( const QDateTime &time );
    virtual QString formatDateTime( const KDateTime &time );
    static QString getColorString(const QColor& color);
    /**
    @brief Create and Add a new button to widget
    This function will add button to UI!
    @return added button, for some managements such as connect to a slot
    */
    KPushButton * addButton(const QString & objName, const QString & toolTip, const QString & icon);
    KPushButton * addButton(const QString & objName, const QString & toolTip, const KIcon & icon);
    QMap<QString, KPushButton*> &buttons();

protected:
    TextBrowser *_mainWidget;
    const QString *baseText;
    static const QString baseStyle;
    static QString readStyle;
    static QString unreadStyle;
    static QString ownStyle;
    static const QRegExp mUrlRegExp;
    static const QString webIconText;
    static const QString ownText;
    static const QString otherText;
    static const QRegExp dirRegExp;

    void setAvatarText( const QString &text );
    QString avatarText() const;
    virtual QString generateResendText();

private:
    class Private;
    Private *const d;
};

class CHOQOK_EXPORT PostWidgetUserData : public QObjectUserData
{
public:
    PostWidgetUserData( PostWidget *postWidget );
    virtual ~PostWidgetUserData();
    PostWidget *postWidget();
    void setPostWidget(PostWidget *widget);

private:
    class Private;
    Private * const d;
};

}
}
#endif // POSTWIDGET_H
