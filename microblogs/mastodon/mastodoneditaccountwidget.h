/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2017  Andrea Scarpino <scarpino@kde.org>

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
