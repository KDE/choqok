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

#include "choqoktools.h"
#include "choqokuiglobal.h"

#include <QDesktopServices>

#include <KProcess>
#include <KMessageBox>

#include "choqokbehaviorsettings.h"

void Choqok::openUrl(const QUrl &url)
{
    bool urlOpeningFailed = false;
    QString failureMessage;
    if (Choqok::BehaviorSettings::useCustomBrowser()) {
        QStringList args = Choqok::BehaviorSettings::customBrowser().split(QLatin1Char(' '));
        args.append(url.toString());
        if (KProcess::startDetached(args) == 0) {
            urlOpeningFailed = true;
            failureMessage = i18n("Custom web browser \"%1\" is unable to open url \"%2\".\nPlease update the custom web browser in Configurations.",
                                 Choqok::BehaviorSettings::customBrowser(), url.toDisplayString());
        }
    } else if(!QDesktopServices::openUrl(url)) {
        urlOpeningFailed = true;
        failureMessage = i18n("Unable to open url \"%1\".\nPlease check Qt installation or set a custom web browser in Configurations.",
                              url.toDisplayString());
    }
    
    if (urlOpeningFailed)
        KMessageBox::error(Choqok::UI::Global::mainWindow(), failureMessage);
}

QString Choqok::getColorString(const QColor &color)
{
    return QLatin1String("rgb(") + QString::number(color.red()) + QLatin1Char(',') + QString::number(color.green()) + QLatin1Char(',') +
           QString::number(color.blue()) + QLatin1Char(')');
}
