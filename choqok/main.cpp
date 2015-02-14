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

#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>

#include "choqokapplication.h"
#include "choqokbehaviorsettings.h"
#include "mainwindow.h"

static const char description[] =
    I18N_NOOP( "KDE Micro-Blogging Client." );

static const char version[] = "1.5";

int main( int argc, char **argv )
{
    kDebug()<<"Choqok "<<version;
    KAboutData about( "choqok", 0, ki18n( "Choqok" ), version, ki18n( description ),
                      KAboutData::License_GPL_V3, ki18n( "(C) 2008-2010 Mehrdad Momeny\n(C) 2011-2015 Choqok Developers" ),
                      KLocalizedString(), 0  );
    about.addAuthor( ki18n( "Mehrdad Momeny" ), ki18n( "Author, Developer and Maintainer" ),
                     "mehrdad.momeny@gmail.com", "http://momeny.wordpress.com" );
    about.addAuthor( ki18n( "Andrey Esin" ), ki18n( "Developer" ),
                     "gmlastik@gmail.com", "http://twitter.com/la_stik" );
    about.addAuthor( ki18n( "Andrea Scarpino" ), ki18n( "Developer" ), "scarpino@kde.org" );
    
    
    about.addCredit( ki18n( "Roozbeh Shafiee" ), ki18n( "Artworks" ), "roozbeh@roozbehonline.com" );
    about.addCredit( ki18n( "Shahrzad Shojaei" ), ki18n( "Artworks" ), "shahrzadesign@gmail.com" );
    about.addCredit( ki18n( "Daniel Schaal" ), ki18n( "UI improvements" ), "daniel@foto-schaal.de");
    about.addCredit( ki18n( "Stephen Henderson" ), ki18n( "Search API implementation" ), "hendersonsk@gmail.com");
    about.addCredit( ki18n( "Tejas Dinkar" ), ki18n( "Developer" ),
                     "tejasdinkar@gmail.com", "http://twitter.com/tdinkar" );
    about.addCredit( ki18n( "Emanuele Bigiarini"), ki18n("D-Bus and Konqueror plugin"), "pulmro@gmail.com");
    about.addCredit( ki18n( "Alex Infantes"), ki18n("Improvements on Image preview plugin"),
                     "alexandro82@gmail.com" );
    about.addCredit( ki18n( "Bardia Daneshvar" ), ki18n("UI improvements"), "bardia.daneshvar@gmail.com");
    about.addCredit( ki18n( "Atanas Gospodinov" ), ki18n("Twitter photo upload"), QByteArray());
    about.addCredit( ki18n( "Daniel Kreuter" ), ki18n("Twitter microblog developer"), "daniel.kreuter85@gmail.com" );
    about.addCredit( ki18n( "Lim Yuen Hoe" ), ki18n("Bug fixes and improvements"), "yuenhoe86@gmail.com" );
    about.addCredit( ki18n( "Ahmed I. Khalil" ), ki18n("Various improvements"), "ahmedibrahimkhali@gmail.com" );

    //TODO before next release, Add new contributers to credits
    KCmdLineArgs::init( argc, argv, &about );

    ChoqokApplication app;
    return app.exec();
}
