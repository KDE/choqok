/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPITEXTEDIT_H
#define TWITTERAPITEXTEDIT_H

#include <QCompleter>

#include "account.h"
#include "choqoktextedit.h"
#include "twitterapihelper_export.h"

class KJob;

class TWITTERAPIHELPER_EXPORT TwitterApiTextEdit : public Choqok::UI::TextEdit
{
    Q_OBJECT

public:
    explicit TwitterApiTextEdit(Choqok::Account *theAccount, QWidget *parent = nullptr);
    ~TwitterApiTextEdit();

    void setCompleter(QCompleter *c);
    QCompleter *completer() const;

protected:
    void keyPressEvent(QKeyEvent *e) override;
    void focusInEvent(QFocusEvent *e) override;

private Q_SLOTS:
    void insertCompletion(const QString &completion);

private:
//     QString textUnderCursor() const;

    class Private;
    Private *const d;
};

#endif // TWITTERAPITEXTEDIT_H
