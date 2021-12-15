/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2017 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef MASTODONEDITACCOUNTWIDGET_H
#define MASTODONEDITACCOUNTWIDGET_H

#include "editaccountwidget.h"

#include <QUrlQuery>

#include "ui_mastodoneditaccountwidget.h"

class MastodonAccount;
class MastodonMicroBlog;

class MastodonEditAccountWidget : public ChoqokEditAccountWidget, Ui::MastodonEditAccountWidget
{
    Q_OBJECT
public:
    explicit MastodonEditAccountWidget(MastodonMicroBlog *microblog, MastodonAccount *account,
                                       QWidget *parent);
    ~MastodonEditAccountWidget();

    virtual Choqok::Account *apply() override;

    virtual bool validateData() override;

private Q_SLOTS:
    void authorizeUser();
    void gotToken();

private:
    void setAuthenticated(bool authenticated);
    void loadTimelinesTable();
    void registerClient();
    void saveTimelinesTable();

    MastodonAccount *m_account;
    bool isAuthenticated;
};

#endif // MASTODONEDITACCOUNTWIDGET_H
