/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOPOSTWIDGET_H
#define PUMPIOPOSTWIDGET_H

#include "postwidget.h"

class PumpIOPostWidget : public Choqok::UI::PostWidget
{
    Q_OBJECT
public:
    explicit PumpIOPostWidget(Choqok::Account *account, Choqok::Post *post, QWidget *parent = nullptr);
    virtual ~PumpIOPostWidget();

    virtual void checkAnchor(const QUrl &url) override;

    virtual QString generateSign() override;

    virtual void initUi() override;

protected Q_SLOTS:
    virtual void slotPostError(Choqok::Account *theAccount, Choqok::Post *post,
                               Choqok::MicroBlog::ErrorType error, const QString &errorMessage) override;

    virtual void slotResendPost() override;

    void slotReplyTo();

    void slotToggleFavorite(Choqok::Account *, Choqok::Post *);

    void toggleFavorite();

protected:
    virtual bool isResendAvailable() override;
    virtual QString getUsernameHyperlink(const Choqok::User &user) const;

    static const QIcon unFavIcon;

private:
    void updateFavStat();
    bool isReplyAvailable();

    class Private;
    Private *const d;

};

#endif // PUMPIOPOSTWIDGET_H
