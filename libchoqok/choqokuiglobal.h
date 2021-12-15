/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKUIGLOBAL_H
#define CHOQOKUIGLOBAL_H

#include <QObject>

#include "choqokmainwindow.h"
#include "choqok_export.h"

#define choqokMainWindow Choqok::UI::Global::mainWindow()

namespace Choqok
{

class Account;

namespace UI
{
class PostWidget;
class QuickPost;

/**
* This namespace contains the Choqok user interface's global widgets
* UI Plugins can use these
*/
namespace Global
{
/**
* Set the main widget to widget
*/
CHOQOK_EXPORT void setMainWindow(Choqok::UI::MainWindow *window);
/**
* Returns the main widget - this is the widget that message boxes
* and KNotify stuff should use as a parent.
*/
CHOQOK_EXPORT Choqok::UI::MainWindow *mainWindow();

CHOQOK_EXPORT void setQuickPostWidget(QuickPost *quickPost);

CHOQOK_EXPORT QuickPost *quickPostWidget();

class CHOQOK_EXPORT SessionManager : public QObject
{
    Q_OBJECT
public:
    ~SessionManager();
    static SessionManager *self();
    void emitNewPostWidgetAdded(Choqok::UI::PostWidget *widget, Choqok::Account *theAccount,
                                const QString &timelineName = QString());

Q_SIGNALS:
    void newPostWidgetAdded(Choqok::UI::PostWidget *widget, Choqok::Account *theAccount,
                            const QString &timelineName);

public Q_SLOTS:
    void resetNotifyManager();

private:
    static SessionManager *m_self;
    SessionManager();
};
} //Global::UI

} //UI

}

#endif
