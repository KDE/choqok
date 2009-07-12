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
#ifndef STATUSWIDGET_H
#define STATUSWIDGET_H

#include <QTimer>
#include <KTextBrowser>
#include <KPushButton>
#include <KUrl>

#include "datacontainers.h"
#include "constants.h"
#include "account.h"

namespace KIO {
    class Job;
}

/**
Status Widget

    @author Mehrdad Momeny <mehrdad.momeny@gmail.com>
*/
class StatusWidget : public KTextBrowser
{
    Q_OBJECT
    Q_PROPERTY (bool read READ isRead)
public:
    enum Notify { WithNotify = 0, WithoutNotify};

    explicit StatusWidget( const Account *account, QWidget *parent = 0 );

    ~StatusWidget();

    QString formatDateTime( const QDateTime &time );

    static void setStyle(const QColor & color, const QColor & back, const QColor & read, const QColor & readBack);

    Status currentStatus() const;
    void setCurrentStatus( const Status newStatus );
    void setUnread( Notify notifyType );
    void setRead(bool read = true);
    void updateFavoriteUi();
    bool isRead() const;

//     static QString getColoredStyle() { return style; };
public slots:
    void setUiStyle();
    void slotFriendAdded(const QString &);

signals:
    void sigSearch(int type, const QString & search);
    void sigReply( const QString &userName, qulonglong statusId, bool dMsg );
    void sigDestroy( qulonglong statusId );
    void sigFavorite( qulonglong statusId, bool isFavorite );
    void sigReTweet( const QString &text );

protected slots:
    void setFavorite( bool isFavorite );
    void requestReply();
    void requestDestroy();
    void requestReTweet();
    void updateSign();
    void userAvatarFetched( const QString &avatarUrl, const QPixmap & pixmap );
    void fetchAvatarError( const QString &avatarUrl, const QString &errMsg );
    void missingStatusReceived( Status status );
    void setHeight();
    void checkAnchor(const QUrl & url);
    void baseStatusReceived( Status status );

    void twitpicImageFetched( const QString &imageUrl, const QPixmap & pixmap );
    void twitpicImageFailed( const QString &imageUrl, const QString &errMsg );

    void showUserInformation(const User& user);

    void slot301Redirected(KIO::Job *job, const KUrl &fromUrl, const KUrl &toUrl);

protected:
    void resizeEvent ( QResizeEvent * event );
    void enterEvent ( QEvent * event );
    void leaveEvent ( QEvent * event );

private:
    void setupUi();

    static KPushButton * getButton(const QString & objName, const QString & toolTip, const QString & icon);

    void setUserImage();
    QString prepareStatus( const QString &text );
    void checkForTwitPicImages(const QString &status);
    void setDirection();
    QString generateSign();
    void updateUi();

    static QString getColorString(const QColor & color);

    QTimer timer;
    Status mCurrentStatus;
    bool mIsRead;
    const Account *mCurrentAccount;

    QString mSign;
    QString mStatus;
    QString mImage;

    static const QString baseText;
    static const QString baseStyle;
    static QString style;

    static const QRegExp mUrlRegExp;
    static const QRegExp mUserRegExp;
    static const QRegExp mHashtagRegExp;
    static const QRegExp mGroupRegExp;

    KPushButton * btnReply,*btnFavorite,*btnRemove, *btnReTweet;
    bool isBaseStatusShowed;
    bool isMissingStatusRequested;
    QString twitpicImageUrl;
    QString twitpicPageUrl;
};

#endif
