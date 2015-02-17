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

#include <QCommandLineParser>

#include <KAboutData>
#include <KLocalizedString>

#include "choqokapplication.h"
#include "choqokdebug.h"

static const char description[] =
    I18N_NOOP( "KDE Micro-Blogging Client." );

static const char version[] = "1.5";

int main( int argc, char **argv )
{
    qCDebug(CHOQOK)<<"Choqok "<<version;
    KAboutData about( "choqok", "Choqok", version, i18n( description ),
                      KAboutLicense::GPL_V3, i18n( "(C) 2008-2010 Mehrdad Momeny\n(C) 2011-2015 Choqok Developers" ),
                      QString(), "http://choqok.gnufolks.org/");
    about.addAuthor( i18n( "Mehrdad Momeny" ), i18n( "Author, Developer and Maintainer" ),
                     "mehrdad.momeny@gmail.com", "http://momeny.wordpress.com" );
    about.addAuthor( i18n( "Andrey Esin" ), i18n( "Developer" ),
                     "gmlastik@gmail.com", "http://twitter.com/la_stik" );
    about.addAuthor( i18n( "Andrea Scarpino" ), i18n( "Developer" ),
                     "scarpino@kde.org", "http://www.andreascarpino.it" );

    about.addCredit( i18n( "Roozbeh Shafiee" ), i18n( "Artworks" ), "roozbeh@roozbehonline.com" );
    about.addCredit( i18n( "Shahrzad Shojaei" ), i18n( "Artworks" ), "shahrzadesign@gmail.com" );
    about.addCredit( i18n( "Daniel Schaal" ), i18n( "UI improvements" ), "daniel@foto-schaal.de");
    about.addCredit( i18n( "Stephen Henderson" ), i18n( "Search API implementation" ), "hendersonsk@gmail.com");
    about.addCredit( i18n( "Tejas Dinkar" ), i18n( "Developer" ),
                     "tejasdinkar@gmail.com", "http://twitter.com/tdinkar" );
    about.addCredit( i18n( "Emanuele Bigiarini"), i18n("D-Bus and Konqueror plugin"), "pulmro@gmail.com");
    about.addCredit( i18n( "Alex Infantes"), i18n("Improvements on Image preview plugin"),
                     "alexandro82@gmail.com" );
    about.addCredit( i18n( "Bardia Daneshvar" ), i18n("UI improvements"), "bardia.daneshvar@gmail.com");
    about.addCredit( i18n( "Atanas Gospodinov" ), i18n("Twitter photo upload"));
    about.addCredit( i18n( "Daniel Kreuter" ), i18n("Twitter microblog developer"), "daniel.kreuter85@gmail.com" );
    about.addCredit( i18n( "Lim Yuen Hoe" ), i18n("Bug fixes and improvements"), "yuenhoe86@gmail.com" );
    about.addCredit( i18n( "Ahmed I. Khalil" ), i18n("Various improvements"), "ahmedibrahimkhali@gmail.com" );

    //TODO before next release, Add new contributers to credits

    ChoqokApplication app(argc, argv);
    app.setApplicationVersion(version);

    QCommandLineParser parser;
    KAboutData::setApplicationData(about);
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    return app.exec();
}
