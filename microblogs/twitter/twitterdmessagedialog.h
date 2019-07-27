/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2015 Andrea Scarpino <scarpino@kde.org>

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

#ifndef TWITTERDMESSAGEDIALOG_H
#define TWITTERDMESSAGEDIALOG_H

#include "twitterapidmessagedialog.h"

class KJob;

class TwitterDMessageDialog : public TwitterApiDMessageDialog
{
    Q_OBJECT
public:
    explicit TwitterDMessageDialog(TwitterApiAccount *theAccount, QWidget *parent = nullptr, Qt::WindowFlags flags = nullptr);
    ~TwitterDMessageDialog();

private Q_SLOTS:
    void slotTextLimit(KJob *job);

private:
    void fetchTextLimit();

};

#endif // TWITTERDMESSAGEDIALOG_H
