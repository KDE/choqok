/*
    This file is part of Choqok, the KDE micro-blogging client

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

#include "twitpic.h"
#include <KAction>
#include <KActionCollection>
#include <KAboutData>
#include <KGenericFactory>
#include <QDBusInterface>
#include <QDBusReply>
#include <choqokuiglobal.h>
#include <quickpost.h>
#include "twitpicsettings.h"
#include "twitpicuploadimage.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < Twitpic > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_twitpic" ) )

Twitpic::Twitpic(QObject* parent, const QList<QVariant>& )
    :Choqok::Plugin(MyPluginFactory::componentData(), parent)
{
    KAction *action = new KAction(KIcon("arrow-up"), i18n("Upload to TwitPic"), this);
    actionCollection()->addAction("uploadToTwitpic", action);
    connect( action, SIGNAL(triggered(bool)), SLOT(slotUploadImage()) );
    setXMLFile("twitpicui.rc");
}

Twitpic::~Twitpic()
{

}

void Twitpic::slotUploadImage()
{
    TwitpicUploadImage *upload = new TwitpicUploadImage(Choqok::UI::Global::mainWindow());
    upload->show();
}
