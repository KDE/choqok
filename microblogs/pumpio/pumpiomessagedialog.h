/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2013-2014 Andrea Scarpino <scarpino@kde.org>

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

#ifndef PUMPIOMESSAGEDIALOG_H
#define PUMPIOMESSAGEDIALOG_H

#include <KDialog>

#include "ui_pumpiomessagedialog.h"

namespace Choqok {
    class Account;
}

class PumpIOMessageDialog : public KDialog, Ui::PumpIOMessageDialog
{
    Q_OBJECT

public:
    explicit PumpIOMessageDialog(Choqok::Account *theAccount, QWidget* parent = 0,
                                 Qt::WindowFlags flags = 0);
    virtual ~PumpIOMessageDialog();

protected Q_SLOTS:
    void attachMedia();
    void cancelAttach();
    void fetchFollowing();
    void sendPost();
    void slotFetchFollowing(Choqok::Account*);

private:
    class Private;
    Private * const d;

};

#endif // PUMPIOMESSAGEDIALOG_H