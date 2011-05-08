/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2011 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#ifndef CHOQOK_APPLICATION_H
#define CHOQOK_APPLICATION_H

#include <kuniqueapplication.h>
#include "choqok_export.h"

namespace Choqok {

class CHOQOK_EXPORT Application : public KUniqueApplication
{
Q_OBJECT
public:
    Application();
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
    Private * const d;
};

}

#endif // CHOQOK_APPLICATION_H
