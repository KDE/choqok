/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOEDITACCOUNTWIDGET_H
#define PUMPIOEDITACCOUNTWIDGET_H

#include "editaccountwidget.h"

#include <QUrlQuery>

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
    void getPinCode();

private:
    void setAuthenticated(bool authenticated);
    void loadTimelinesTable();
    void registerClient();
    void saveTimelinesTable();

    PumpIOAccount *m_account;
    bool isAuthenticated;
};

#endif // PUMPIOEDITACCOUNTWIDGET_H
