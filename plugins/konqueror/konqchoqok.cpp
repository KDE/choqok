/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "konqchoqok.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMenu>

#include <KAboutData>
#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KToggleAction>
#include <KToolInvocation>
#include <KWebPage>

//static const KAboutData aboutdata("konqchoqokplugin", i18n("Konqueror Choqok Plugin") , "1.0" );
K_PLUGIN_FACTORY_WITH_JSON( KonqPluginChoqokFactory, "konqchoqok.json", registerPlugin<KonqPluginChoqok>(); )

KonqPluginChoqok::KonqPluginChoqok(QObject* parent, const QVariantList& )
    : Plugin( parent ) , m_interface(0)
{ 
    KActionMenu *menu = new KActionMenu(QIcon::fromTheme("choqok") , i18n("Choqok"),
    actionCollection() );
    actionCollection()->addAction( "action menu", menu);
    menu->setDelayed( false );
    
    QAction *postaction = actionCollection()->addAction( "post_choqok" );
    postaction->setText( i18n("Post Text with Choqok") );
    connect( postaction, SIGNAL( triggered(bool) ), SLOT( slotpostSelectedText() ) );
    menu->addAction( postaction );
    
    QAction *shortening = actionCollection()->add<KToggleAction>( "shortening_choqok");
    shortening->setText( i18n("Shorten URL on Paste") );
    connect( shortening, SIGNAL( toggled(bool) ), SLOT( toggleShortening(bool) ) );
    menu->addAction( shortening );
    
    connect( menu->menu(), SIGNAL( aboutToShow() ), SLOT( updateActions() ) );
}

void KonqPluginChoqok::updateActions()
{
    
    // Is Choqok running?
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.choqok")) {
    ((KToggleAction*) actionCollection()->action("shortening_choqok"))->setEnabled(false);
    return;
    }
    // Choqok is running, so I can connect to it, if I haven't done yet.
    if (!m_interface) {
    m_interface = new  QDBusInterface("org.kde.choqok",
                        "/",
                        "org.kde.choqok",
                        QDBusConnection::sessionBus());
    
    }
    QDBusReply<bool> reply = m_interface->call("getShortening");
    if ( reply.isValid() ) {
      ((KToggleAction*) actionCollection()->action("shortening_choqok"))->setEnabled(true);
      ((KToggleAction*) actionCollection()->action("shortening_choqok"))->setChecked(reply.value());
    }
}


KonqPluginChoqok::~KonqPluginChoqok()
{

}

void KonqPluginChoqok::slotpostSelectedText()
{
    QWidget *m_parentWidget;
    QString text;

    if ( parent()->inherits("KWebPage") ) {
        m_parentWidget = qobject_cast< KWebPage* >(parent())->view();
        text = QString(qobject_cast< KWebPage* >(parent())->selectedText());
    } else {
        return;
    }

    if (text.isEmpty()) {
        KMessageBox::information( m_parentWidget,
                  i18n("You need to select text to post."),
                  i18n("Post Text with Choqok"));
        return;
    }
    
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered("org.kde.choqok"))
    {
    //qDebug() << "Choqok is not running, starting it!..." << endl;
    KToolInvocation::startServiceByDesktopName(QString("choqok"),
                           QStringList());
    }
    if (!m_interface) {
    m_interface = new  QDBusInterface("org.kde.choqok",
                        "/",
                        "org.kde.choqok",
                        QDBusConnection::sessionBus());
    }

    m_interface->call("postText",text);
}

void KonqPluginChoqok::toggleShortening(bool value)
{
    m_interface->call("setShortening", value );
    ((KToggleAction*) actionCollection()->action("shortening_choqok"))->setChecked(value);
}

#include "konqchoqok.moc"
