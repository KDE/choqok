/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef CHOQOK_APPLICATION_H
#define CHOQOK_APPLICATION_H

#include <QApplication>

#include "choqok_export.h"

namespace Choqok
{

class CHOQOK_EXPORT Application : public QApplication
{
    Q_OBJECT
public:
    Application(int &argc, char **argv);
    virtual ~Application();

    /**
     * Method to return whether or not we're shutting down
     * or not at this point.
     */
    static bool isShuttingDown();

    static bool isStartingUp();
    static void setStartingUp(bool startingUp);

protected:
    static void setShuttingDown(bool isShuttingDown = true);

private:
    class Private;
    Private *const d;
};

}

#endif // CHOQOK_APPLICATION_H
