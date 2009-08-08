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
#ifndef POSTWIDGET_H
#define POSTWIDGET_H

#include <ktextbrowser.h>
#include <account.h>
#include <choqoktypes.h>
#include <microblog.h>
#include <QTimer>
#include <kicon.h>

class QGridLayout;
class KPushButton;

namespace Choqok
{

class CHOQOK_EXPORT PostWidget : public KTextBrowser
{
    Q_OBJECT
    Q_PROPERTY (bool read READ isRead)
public:
    explicit PostWidget( Account *account, const Post &post, QWidget *parent = 0 );
    virtual ~PostWidget();
    const Post &currentPost() const;
    void setRead(bool read = true);
    bool isRead() const;

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
    static void setStyle(const QColor& unreadColor, const QColor& unreadBack,
                          const QColor& readColor, const QColor& readBack);

public slots:
    /**
    Set Style sheet of widget to corresponding data->
    @see setStyle()
    */
    void setUiStyle();

signals:
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

protected slots:
    /**
    Set height of widget related to text contents
    */
    void setHeight();

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
    void slotCurrentPostRemoved( Account *theAccount, Post *post );

    virtual void slotPostError( Account *theAccount, Choqok::MicroBlog::ErrorType error,
                                const QString &errorMessage, const Post *post);

    void avatarFetchError( const QString &remoteUrl, const QString &errMsg );
    void avatarFetched( const QString &remoteUrl, const QPixmap &pixmap );

protected:
    virtual void setupAvatar();
    virtual void mousePressEvent(QMouseEvent* ev);
    virtual void resizeEvent ( QResizeEvent * event );
    virtual void enterEvent ( QEvent * event );
    virtual void leaveEvent ( QEvent * event );
    virtual QString prepareStatus( const QString &text );
    virtual void setDirection();
    virtual QString generateSign();
    virtual QString formatDateTime( const QDateTime &time );
    static QString getColorString(const QColor& color);
    /**
    @brief Create and Add a new button to widget
    This function will add button to UI!
    @return added button, for some managements such as connect to a slot
    */
    KPushButton * addButton(const QString & objName, const QString & toolTip, const QString & icon);
    KPushButton * addButton(const QString & objName, const QString & toolTip, const KIcon & icon);
    QList<KPushButton*> &buttons();

protected:
    Post mCurrentPost;
    Account *mCurrentAccount;
    bool mRead;
    QTimer mTimer;

    //BEGIN UI contents:
    QString mSign;
    QString mContent;
    QString mImage;
    //END UI contents;

    static const QString baseText;
    static const QString baseStyle;
    static QString readStyle;
    static QString unreadStyle;
    static const QRegExp mUrlRegExp;

private:
    void setupUi();
    QGridLayout *buttonsLayout;
    QList<KPushButton*> mUiButtons;
};

}
#endif // POSTWIDGET_H
