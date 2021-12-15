/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOKAPPLICATION_H
#define CHOQOKAPPLICATION_H

#include <QPointer>

#include "application.h"

class MainWindow;

/**
 * @author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
 */
class ChoqokApplication : public Choqok::Application
{
    Q_OBJECT

public:
    ChoqokApplication(int &argc, char **argv);
    ~ChoqokApplication();

    void setupMainWindow();

public Q_SLOTS:
    /**
     * Quit Choqok, closing all the windows, which causes application shutdown
     * This method marks Choqok as 'shutting down'
     */
    void quitChoqok();
private:

    // The main window might get deleted behind our back (W_DestructiveClose),
    // so use a guarded pointer
    QPointer<MainWindow> m_mainWindow;
};

#endif

