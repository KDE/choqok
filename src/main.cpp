/*
    This file is part of choqoK, the KDE micro-blogging client

    Copyright (C) 2008-2009 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "mainwindow.h"
#include "settings.h"
#include "constants.h"
#include <kuniqueapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KDE/KLocale>

static const char description[] =
    I18N_NOOP( "KDE Micro-blogging client,\n\
Currently supports <a href='http://twitter.com'>Twitter.com</a> and \
<a href='http://identi.ca'>Identi.ca</a>" );

static const char version[] = "0.4";

int main( int argc, char **argv )
{
    KAboutData about( "choqok", 0, ki18n( "choqoK" ), version, ki18n( description ),
                      KAboutData::License_GPL_V3, ki18n( "(C) 2008-2009 Mehrdad Momeny" ),
                      KLocalizedString(), 0, "mehrdad.momeny@gmail.com" );
    about.addAuthor( ki18n( "Mehrdad Momeny" ), ki18n( "Author and Core Developer" ),
                     "mehrdad.momeny@gmail.com" );
    about.addCredit( ki18n( "Roozbeh Shafiee" ), ki18n( "Icon designer" ), "roozbeh@roozbehonline.com" );
    about.addCredit( ki18n( "Daniel Schaal" ), ki18n( "UI Improvements" ), "daniel@foto-schaal.de");
    about.addCredit( ki18n( "Stephen Henderson" ), ki18n( "Search API implementation" ), "hendersonsk@gmail.com");
    KCmdLineArgs::init( argc, argv, &about );

//     KCmdLineOptions options;
//     options.add("+[URL]", ki18n( "Document to open" ));
//     KCmdLineArgs::addCmdLineOptions(options);
    KUniqueApplication app;

    MainWindow *mainWin = new MainWindow;
    mainWin->setAttribute(Qt::WA_DeleteOnClose, false);
    if ( Settings::showMainWinOnStart() ) {
        mainWin->show();
    }
    return app.exec();
}
