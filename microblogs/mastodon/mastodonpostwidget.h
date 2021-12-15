/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MASTODONPOSTWIDGET_H
#define MASTODONPOSTWIDGET_H

#include "postwidget.h"

class MastodonPostWidget : public Choqok::UI::PostWidget
{
    Q_OBJECT
public:
    explicit MastodonPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent = nullptr);
    virtual ~MastodonPostWidget();

    virtual QString generateSign() override;

    virtual void initUi() override;

protected Q_SLOTS:
    virtual void slotResendPost() override;

    void slotToggleFavorite(Choqok::Account *, Choqok::Post *);
    void slotReply();
    void slotWriteTo();
    void slotReplyToAll();
    void toggleFavorite();

protected:
    virtual QString getUsernameHyperlink(const Choqok::User &user) const;

    static const QIcon unFavIcon;

private:
    void updateFavStat();

    class Private;
    Private *const d;
};

#endif // MASTODONPOSTWIDGET_H
