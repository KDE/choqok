/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "konqchoqok.h"

#include <QDBusInterface>
#include <QDBusConnection>
#include <QDBusConnectionInterface>
#include <QMenu>

#include <KActionCollection>
#include <KActionMenu>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>
#include <KToggleAction>
#include <KToolInvocation>
#include <KWebPage>

K_PLUGIN_CLASS_WITH_JSON(KonqPluginChoqok, "konqchoqok.json")

KonqPluginChoqok::KonqPluginChoqok(QObject *parent, const QVariantList &)
    : Plugin(parent) , m_interface(nullptr)
{
    KActionMenu *menu = new KActionMenu(QIcon::fromTheme(QLatin1String("choqok")) , i18n("Choqok"),
                                        actionCollection());
    actionCollection()->addAction(QLatin1String("action menu"), menu);
    menu->setDelayed(false);

    QAction *postaction = actionCollection()->addAction(QLatin1String("post_choqok"));
    postaction->setText(i18n("Post Text with Choqok"));
    connect(postaction, SIGNAL(triggered(bool)), SLOT(slotpostSelectedText()));
    menu->addAction(postaction);

    QAction *shortening = actionCollection()->add<KToggleAction>(QLatin1String("shortening_choqok"));
    shortening->setText(i18n("Shorten URL on Paste"));
    connect(shortening, SIGNAL(toggled(bool)), SLOT(toggleShortening(bool)));
    menu->addAction(shortening);

    connect(menu->menu(), SIGNAL(aboutToShow()), SLOT(updateActions()));
}

void KonqPluginChoqok::updateActions()
{

    // Is Choqok running?
    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.choqok"))) {
        ((KToggleAction *) actionCollection()->action(QLatin1String("shortening_choqok")))->setEnabled(false);
        return;
    }
    // Choqok is running, so I can connect to it, if I haven't done yet.
    if (!m_interface) {
        m_interface = new  QDBusInterface(QLatin1String("org.kde.choqok"),
                                          QLatin1String("/"),
                                          QLatin1String("org.kde.choqok"),
                                          QDBusConnection::sessionBus());

    }
    QDBusReply<bool> reply = m_interface->call(QLatin1String("getShortening"));
    if (reply.isValid()) {
        ((KToggleAction *) actionCollection()->action(QLatin1String("shortening_choqok")))->setEnabled(true);
        ((KToggleAction *) actionCollection()->action(QLatin1String("shortening_choqok")))->setChecked(reply.value());
    }
}

KonqPluginChoqok::~KonqPluginChoqok()
{

}

void KonqPluginChoqok::slotpostSelectedText()
{
    QWidget *m_parentWidget;
    QString text;

    if (parent()->inherits("KWebPage")) {
        m_parentWidget = qobject_cast< KWebPage * >(parent())->view();
        text = QString(qobject_cast< KWebPage * >(parent())->selectedText());
    } else {
        return;
    }

    if (text.isEmpty()) {
        KMessageBox::information(m_parentWidget,
                                 i18n("You need to select text to post."),
                                 i18n("Post Text with Choqok"));
        return;
    }

    if (!QDBusConnection::sessionBus().interface()->isServiceRegistered(QLatin1String("org.kde.choqok"))) {
        //qDebug() << "Choqok is not running, starting it!..." << endl;
        KToolInvocation::startServiceByDesktopName(QLatin1String("choqok"),
                QStringList());
    }
    if (!m_interface) {
        m_interface = new  QDBusInterface(QLatin1String("org.kde.choqok"),
                                          QLatin1String("/"),
                                          QLatin1String("org.kde.choqok"),
                                          QDBusConnection::sessionBus());
    }

    m_interface->call(QLatin1String("postText"), text);
}

void KonqPluginChoqok::toggleShortening(bool value)
{
    m_interface->call(QLatin1String("setShortening"), value);
    ((KToggleAction *) actionCollection()->action(QLatin1String("shortening_choqok")))->setChecked(value);
}

#include "konqchoqok.moc"
