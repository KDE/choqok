/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
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
