/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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

