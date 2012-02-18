/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#ifndef TWITTERAPISEARCHDIALOG_H
#define TWITTERAPISEARCHDIALOG_H

#include <kdialog.h>
#include "choqok_export.h"

class TwitterApiAccount;
class CHOQOK_HELPER_EXPORT TwitterApiSearchDialog : public KDialog
{
    Q_OBJECT
public:
    explicit TwitterApiSearchDialog(TwitterApiAccount* theAccount, QWidget* parent = 0);
    ~TwitterApiSearchDialog();

protected:
    virtual void createUi();
    virtual void fillSearchTypes();
    virtual void slotButtonClicked(int button);

protected Q_SLOTS:
    void slotSearchTypeChanged(int);

private:
    class Private;
    Private * const d;
};

#endif // TWITTERAPISEARCHDIALOG_H
