/*
    This file is part of choqoK, the KDE Twitter client

    Copyright (C) 2008 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.

*/

#include "systrayicon.h"
#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <KDE/KLocale>

static const char description[] =
    I18N_NOOP("A KDE 4 Twitter Client");

static const char version[] = "0.1";

int main(int argc, char **argv)
{
	KAboutData about("choqok", 0, ki18n("choqoK"), version, ki18n(description),
                     KAboutData::License_GPL_V3, ki18n("(C) 2008 Mehrdad Momeny"), KLocalizedString(), 0, "mehrdad.momeny@gmail.com");
    about.addAuthor( ki18n("Mehrdad Momeny"), ki18n("Author and Core Developer"),
					 "mehrdad.momeny@gmail.com");
	about.addCredit(ki18n("Roozbeh Shafiee"), ki18n("Icon designer"), "roozbeh@roozbehonline.com");
    KCmdLineArgs::init(argc, argv, &about);

//     KCmdLineOptions options;
//     options.add("+[URL]", ki18n( "Document to open" ));
//     KCmdLineArgs::addCmdLineOptions(options);
    KApplication app;

	if(QSystemTrayIcon::isSystemTrayAvailable()){
		SysTrayIcon *sysIcon = new SysTrayIcon;
		sysIcon->show();
	} else {
		MainWindow *mainWin = new MainWindow;
		mainWin->show();
	}


    return app.exec();
}
