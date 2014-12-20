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

#ifndef TWITTERAPITEXTEDIT_H
#define TWITTERAPITEXTEDIT_H

#include "account.h"
#include "choqoktextedit.h"
#include "choqok_export.h"

class QCompleter;
class KJob;

class CHOQOK_HELPER_EXPORT TwitterApiTextEdit : public Choqok::UI::TextEdit
{
Q_OBJECT

public:
    explicit TwitterApiTextEdit(Choqok::Account *theAccount, QWidget* parent = 0);
    ~TwitterApiTextEdit();

    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

protected Q_SLOTS:
    virtual void updateRemainingCharsCount();

protected:
    void keyPressEvent(QKeyEvent *e);
    void focusInEvent(QFocusEvent *e);

private Q_SLOTS:
    void insertCompletion(const QString &completion);
    void slotTCoMaximumLength(KJob *job);

private:
//     QString textUnderCursor() const;
    void fetchTCoMaximumLength();

    class Private;
    Private * const d;
};

#endif // TWITTERAPITEXTEDIT_H
