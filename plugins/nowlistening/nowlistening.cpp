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

#include "nowlistening.h"
#include <KAction>
#include <KActionCollection>
#include <KAboutData>
#include <KGenericFactory>
#include <QDBusInterface>
#include <QDBusReply>
#include <choqokuiglobal.h>
#include <quickpost.h>
#include "nowlisteningsettings.h"

K_PLUGIN_FACTORY( MyPluginFactory, registerPlugin < NowListening > (); )
K_EXPORT_PLUGIN( MyPluginFactory( "choqok_nowlistening" ) )

// typedef KGenericFactory<NowListening> MyPluginFactory;
// static const KAboutData aboutdata("choqok_nowlistening", 0, ki18n("Now Listening") , "0.1",
//                                   ki18n("Tells your friends what you're listening to"),
//                                   KAboutData::License_GPL_V3);
// K_EXPORT_COMPONENT_FACTORY( choqok_nowlistening, MyPluginFactory( &aboutdata )  )

NowListening::NowListening(QObject* parent, const QList<QVariant>& )
    :Choqok::Plugin(MyPluginFactory::componentData(), parent)
{
    KAction *action = new KAction(KIcon("media-playback-start"), i18n("Now Listening"), this);
    actionCollection()->addAction("nowListening", action);
    connect(action, SIGNAL(triggered(bool)), SLOT(slotPrepareNowListening()));
    setXMLFile("nowlisteningui.rc");
}

NowListening::~NowListening()
{

}

void NowListening::slotPrepareNowListening()
{
    QDBusInterface remoteApp( "org.kde.amarok", "/Player", "org.freedesktop.MediaPlayer" );
    QDBusReply< QMap<QString, QVariant> > reply = remoteApp.call( "GetMetadata" );
    QVariantMap trackInfo = reply.value();
    QString text = NowListeningSettings::templateString();
    text.replace("%track%", trackInfo["tracknumber"].toString());
    text.replace("%title%", trackInfo["title"].toString());
    text.replace("%album%", trackInfo["album"].toString());
    text.replace("%artist%", trackInfo["artist"].toString());
    text.replace("%year%", trackInfo["year"].toString());
    text.replace("%genre%", trackInfo["genre"].toString());
    if( Choqok::UI::Global::quickPostWidget() )
        Choqok::UI::Global::quickPostWidget()->setText(text);
}

// #include "nowlistening.moc"
