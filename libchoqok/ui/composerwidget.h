/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2008-2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#ifndef COMPOSERWIDGET_H
#define COMPOSERWIDGET_H

#include <QWidget>
#include "choqok_export.h"
#include <choqoktypes.h>
#include <KPushButton>
#include <QPointer>

class QHBoxLayout;
namespace Choqok {
class Account;

namespace UI {
class TextEdit;
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT ComposerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComposerWidget(Account *account, QWidget* parent = 0);
    virtual ~ComposerWidget();
    TextEdit *editor();

public Q_SLOTS:
    virtual void setText(const QString &text, const QString &replyToId = QString());
    virtual void abort();

protected Q_SLOTS:
    virtual void submitPost( const QString &text );
    virtual void slotPostSubmited(Choqok::Account *theAccount, Choqok::Post* post);
    virtual void slotErrorPost(Choqok::Account* theAccount,Choqok::Post* post);
    virtual void editorTextChanged();
    virtual void editorCleared();

protected:
    Account *currentAccount();
    QWidget *editorContainer();
    Choqok::Post *postToSubmit();
    void setPostToSubmit( Choqok::Post *post );


    QString replyToId;
    QPointer<KPushButton> btnAbort;

private:
    class Private;
    Private * const d;
};
}
}
#endif // COMPOSERWIDGET_H
