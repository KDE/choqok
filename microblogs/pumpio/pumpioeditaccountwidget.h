/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013  Andrea Scarpino <scarpino@kde.org>

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

#ifndef PUMPIOEDITACCOUNTWIDGET_H
#define PUMPIOEDITACCOUNTWIDGET_H

#include "editaccountwidget.h"

#include <QUrlQuery>

#include <QtOAuth/QtOAuth>

#include "ui_pumpioeditaccountwidget.h"

class PumpIOAccount;
class PumpIOMicroBlog;

class PumpIOEditAccountWidget : public ChoqokEditAccountWidget, Ui::PumpIOEditAccountWidget
{
    Q_OBJECT
public:
    explicit PumpIOEditAccountWidget(PumpIOMicroBlog *microblog, PumpIOAccount *account,
                                     QWidget *parent);
    ~PumpIOEditAccountWidget();

    virtual Choqok::Account *apply() override;

    virtual bool validateData() override;

private Q_SLOTS:
    void authorizeUser();

private:
    bool isAuthenticated();
    void loadTimelinesTable();
    void registerClient();
    void saveTimelinesTable();

    PumpIOAccount *m_account;
    QOAuth::Interface *m_qoauth;
};

#endif // PUMPIOEDITACCOUNTWIDGET_H
