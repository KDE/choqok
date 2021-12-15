/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2013-2014 Andrea Scarpino <scarpino@kde.org>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef PUMPIOMESSAGEDIALOG_H
#define PUMPIOMESSAGEDIALOG_H

#include <QDialog>

#include "ui_pumpiomessagedialog.h"

namespace Choqok
{
class Account;
}

class PumpIOMessageDialog : public QDialog, Ui::PumpIOMessageDialog
{
    Q_OBJECT

public:
    explicit PumpIOMessageDialog(Choqok::Account *theAccount, QWidget *parent = nullptr,
                                 Qt::WindowFlags flags = {});
    ~PumpIOMessageDialog();

protected Q_SLOTS:
    virtual void accept() override;
    void attachMedia();
    void cancelAttach();
    void fetchFollowing();
    void slotFetchFollowing(Choqok::Account *);

private:
    class Private;
    Private *const d;

};

#endif // PUMPIOMESSAGEDIALOG_H
