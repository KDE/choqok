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
    ChoqokApplication();
    ~ChoqokApplication();

    virtual int newInstance();

public Q_SLOTS:
    /**
     * Quit Choqok, closing all the windows, which causes application shutdown
     * This method marks Choqok as 'shutting down'
     */
    void quitChoqok();

    virtual void commitData( QSessionManager &sm );
private:

    // The main window might get deleted behind our back (W_DestructiveClose),
    // so use a guarded pointer
    QPointer<MainWindow> m_mainWindow;
};

#endif

// vim: set noet ts=4 sts=4 sw=4:

