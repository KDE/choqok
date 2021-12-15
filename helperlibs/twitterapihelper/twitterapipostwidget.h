/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIPOSTWIDGET_H
#define TWITTERAPIPOSTWIDGET_H

#include "postwidget.h"

/**
@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_HELPER_EXPORT TwitterApiPostWidget : public Choqok::UI::PostWidget
{
    Q_OBJECT
public:
    TwitterApiPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent = nullptr);
    ~TwitterApiPostWidget();

    virtual void initUi() override;

protected Q_SLOTS:
    virtual void checkAnchor(const QUrl &url) override;
    virtual void setFavorite();
    virtual void slotSetFavorite(Choqok::Account *theAccount, const QString &postId);
    virtual void slotReply();
    virtual void repeatPost();
    virtual void slotWriteTo();
    virtual void slotReplyToAll();

    void slotBasePostFetched(Choqok::Account *theAccount, Choqok::Post *post);

protected:
    virtual QString generateSign() override;
    virtual QString getUsernameHyperlink(const Choqok::User &user) const;

    void updateFavStat();

    static const QIcon unFavIcon;
private:
    class Private;
    Private *const d;

};

#endif // TWITTERPOSTWIDGET_H
