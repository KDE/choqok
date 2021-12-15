/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GNUSOCIALAPIPOSTWIDGET_H
#define GNUSOCIALAPIPOSTWIDGET_H

#include "twitterapipostwidget.h"

class CHOQOK_HELPER_EXPORT GNUSocialApiPostWidget : public TwitterApiPostWidget
{
public:
    GNUSocialApiPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent = nullptr);
    ~GNUSocialApiPostWidget();
    virtual void initUi() override;

protected:
    static const QRegExp mGroupRegExp;
    static const QRegExp mGNUSocialApiUserRegExp;
    static const QRegExp mGNUSocialApiHashRegExp;
    static const QRegExp mStatusNetUserRegExp;
    virtual QString prepareStatus(const QString &text) override;
    virtual void checkAnchor(const QUrl &url) override;
    virtual QString generateSign() override;
    virtual void slotReplyToAll() override;

protected Q_SLOTS:
    virtual void slotResendPost() override;

private:
    class Private;
    Private *d;
};

#endif // GNUSOCIALAPIPOSTWIDGET_H
