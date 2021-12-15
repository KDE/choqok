/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERPOSTWIDGET_H
#define TWITTERPOSTWIDGET_H

#include "twitterapipostwidget.h"

class TwitterPostWidget : public TwitterApiPostWidget
{
    Q_OBJECT
public:
    TwitterPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent = nullptr);
    ~TwitterPostWidget();

    virtual void initUi() override;

protected Q_SLOTS:
    void slotReplyToAll() override;
    void quotedAvatarFetched(const QUrl &remoteUrl, const QPixmap &pixmap);
    void quotedAvatarFetchError(const QUrl &remoteUrl, const QString &errMsg);

protected:
    QString prepareStatus(const QString &text) override;
    void checkAnchor(const QUrl &url) override;
    bool isRemoveAvailable() override;
    bool setupQuotedAvatar();

    static const QRegExp mTwitterUserRegExp;
    static const QRegExp mTwitterTagRegExp;
    static const QString mQuotedTextBase;
    static const QUrl    mQuotedAvatarResourceUrl;
private:
    QString getBackgroundColor();
};

#endif // TWITTERPOSTWIDGET_H
