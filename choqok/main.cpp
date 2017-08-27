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
#include <Kdelibs4ConfigMigrator>

#include "choqokapplication.h"
#include "choqokdebug.h"

static const char description[] =
    I18N_NOOP("KDE Micro-Blogging Client.");

static const char version[] = "1.6.0";

int main(int argc, char **argv)
{
    qCDebug(CHOQOK) << "Choqok " << version;

    ChoqokApplication app(argc, argv);

    KLocalizedString::setApplicationDomain("choqok");
    KAboutData about(QLatin1String("choqok"), QLatin1String("Choqok"), QLatin1String(version), i18n(description),
                     KAboutLicense::GPL_V3, i18n("(C) 2008-2010 Mehrdad Momeny\n(C) 2011-2015 Choqok Developers"),
                     QString(), QLatin1String("http://choqok.gnufolks.org/"));

    about.setOrganizationDomain("kde.org");
    about.setDesktopFileName(QStringLiteral("org.kde.choqok"));

    about.addAuthor(i18n("Mehrdad Momeny"), i18n("Author, Developer and Maintainer"),
                    QLatin1String("mehrdad.momeny@gmail.com"), QLatin1String("http://momeny.wordpress.com"));
    about.addAuthor(i18n("Andrey Esin"), i18n("Developer"),
                    QLatin1String("gmlastik@gmail.com"), QLatin1String("https://twitter.com/la_stik"));
    about.addAuthor(i18n("Andrea Scarpino"), i18n("Developer"),
                    QLatin1String("scarpino@kde.org"), QLatin1String("http://www.andreascarpino.it"));

    about.addCredit(i18n("Roozbeh Shafiee"), i18n("Artworks"), QLatin1String("roozbeh@roozbehonline.com"));
    about.addCredit(i18n("Shahrzad Shojaei"), i18n("Artworks"), QLatin1String("shahrzadesign@gmail.com"));
    about.addCredit(i18n("Daniel Schaal"), i18n("UI improvements"), QLatin1String("daniel@foto-schaal.de"));
    about.addCredit(i18n("Stephen Henderson"), i18n("Search API implementation"), QLatin1String("hendersonsk@gmail.com"));
    about.addCredit(i18n("Tejas Dinkar"), i18n("Developer"),
                    QLatin1String("tejasdinkar@gmail.com"), QLatin1String("https://twitter.com/tdinkar"));
    about.addCredit(i18n("Emanuele Bigiarini"), i18n("D-Bus and Konqueror plugin"), QLatin1String("pulmro@gmail.com"));
    about.addCredit(i18n("Alex Infantes"), i18n("Improvements on Image preview plugin"),
                    QLatin1String("alexandro82@gmail.com"));
    about.addCredit(i18n("Bardia Daneshvar"), i18n("UI improvements"), QLatin1String("bardia.daneshvar@gmail.com"));
    about.addCredit(i18n("Atanas Gospodinov"), i18n("Twitter photo upload"));
    about.addCredit(i18n("Daniel Kreuter"), i18n("Twitter microblog developer"), QLatin1String("daniel.kreuter85@gmail.com"));
    about.addCredit(i18n("Lim Yuen Hoe"), i18n("Bug fixes and improvements"), QLatin1String("yuenhoe86@gmail.com"));
    about.addCredit(i18n("Ahmed I. Khalil"), i18n("Various improvements"), QLatin1String("ahmedibrahimkhali@gmail.com"));

    //TODO before next release, Add new contributers to credits

    // Migrate configurations from KDE4
    QStringList configFiles;
    QStringList rcFiles;
    configFiles << QLatin1String("choqokrc");
    rcFiles << QLatin1String("choqokui.rc");

    KAboutData::setApplicationData(about);

    app.setApplicationName(about.componentName());
    app.setApplicationDisplayName(about.displayName());
    app.setOrganizationDomain(about.organizationDomain());
    app.setApplicationVersion(about.version());

    QCommandLineParser parser;
    parser.addVersionOption();
    parser.addHelpOption();
    about.setupCommandLine(&parser);
    parser.process(app);
    about.processCommandLine(&parser);

    Kdelibs4ConfigMigrator migrator(about.componentName());
    migrator.setConfigFiles(configFiles);
    migrator.setUiFiles(rcFiles);
    migrator.migrate();

    app.setupMainWindow();

    return app.exec();
}
