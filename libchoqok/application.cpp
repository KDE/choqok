/*
This file is part of Choqok, the KDE micro-blogging client

Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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
#include "application.h"

using namespace Choqok;

class Application::Private
{
public:
    static bool isShuttingDown;
    static bool isStartingUp;
};

bool Application::Private::isShuttingDown = false;
bool Application::Private::isStartingUp = true;

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv), d(new Private)
{
}

Application::~Application()
{
    delete d;
}

bool Application::isShuttingDown()
{
    return Private::isShuttingDown;
}

void Application::setShuttingDown(bool isShuttingDown)
{
    Private::isShuttingDown = isShuttingDown;
}

bool Application::isStartingUp()
{
    return Private::isStartingUp;
}

void Application::setStartingUp(bool startingUp)
{
    Private::isStartingUp = startingUp;
}

