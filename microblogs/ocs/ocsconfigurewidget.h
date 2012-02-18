/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef OCSCONFIGUREWIDGET_H
#define OCSCONFIGUREWIDGET_H

#include "editaccountwidget.h"
#include "ui_ocsconfigurebase.h"

class OCSMicroblog;
class OCSAccount;
class OCSConfigureWidget : public ChoqokEditAccountWidget, Ui::OCSConfigureBase
{
Q_OBJECT
public:
    explicit OCSConfigureWidget(OCSMicroblog *microblog, OCSAccount* account, QWidget* parent);
    virtual ~OCSConfigureWidget();
    virtual bool validateData();
    virtual Choqok::Account* apply();

protected slots:
    void slotprovidersLoaded();

private:
    OCSAccount* mAccount;
    OCSMicroblog* mMicroblog;
    bool providersLoaded;
};

#endif // OCSCONFIGUREWIDGET_H
