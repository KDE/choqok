/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef COMPOSERWIDGET_H
#define COMPOSERWIDGET_H

#include <QPointer>
#include <QWidget>

#include "account.h"
#include "choqoktypes.h"
#include "choqok_export.h"

class QLabel;
class QPushButton;

namespace Choqok
{
namespace UI
{
class TextEdit;
/**

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class CHOQOK_EXPORT ComposerWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ComposerWidget(Account *account, QWidget *parent = nullptr);
    virtual ~ComposerWidget();
    TextEdit *editor();

public Q_SLOTS:
    virtual void setText(const QString &text, const QString &replyToId = QString(),
                         const QString &replyToUsername = QString());
    virtual void abort();

protected Q_SLOTS:
    virtual void submitPost(const QString &text);
    virtual void slotPostSubmited(Choqok::Account *theAccount, Choqok::Post *post);
    virtual void slotErrorPost(Choqok::Account *theAccount, Choqok::Post *post);
    virtual void editorTextChanged();
    virtual void editorCleared();

protected:
    /**
    Sub classes can use another editor! (Should be a subclass of Choqok::Editor)
    */
    virtual void setEditor(TextEdit *editor);
    QPointer<QPushButton> btnCancelReply();
    Account *currentAccount();
    QWidget *editorContainer();
    Choqok::Post *postToSubmit();
    QPointer<QLabel> replyToUsernameLabel();
    void setPostToSubmit(Choqok::Post *post);

    QString replyToId;
    QString replyToUsername;
    QPointer<QPushButton> btnAbort;

private:
    class Private;
    Private *const d;
};
}
}
#endif // COMPOSERWIDGET_H
