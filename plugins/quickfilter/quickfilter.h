/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>
    SPDX-FileCopyrightText: 2011 Farhad Hedayati-Fard <hf.farhad@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef QUICKFILTER_H
#define QUICKFILTER_H

#include <QList>

#include "plugin.h"

class QAction;
namespace Choqok
{
class Account;
namespace UI
{
class PostWidget;
}
}

class QLineEdit;
class QToolBar;

class QuickFilter : public Choqok::Plugin
{
    Q_OBJECT
public:
    QuickFilter(QObject *parent, const QList< QVariant > &args);
    ~QuickFilter();

public Q_SLOTS:
    void createUiInterface();
    void showAuthorFilterUiInterface(bool);
    void showContentFilterUiInterface(bool);
    void showAllPosts();

protected Q_SLOTS:
    void filterByAuthor();
    void filterByContent();
    void filterNewPost(Choqok::UI::PostWidget *, Choqok::Account *, QString);

private Q_SLOTS:
    void updateUser(QString user);
    void updateContent(QString text);

private:
    QString m_filterUser;
    QString m_filterText;
    QLineEdit *m_aledit;
    QLineEdit *m_tledit;
    QToolBar *m_authorToolbar;
    QToolBar *m_textToolbar;
    QAction *m_authorAction;
    QAction *m_textAction;
};

#endif // QUICKFILTER_H
