/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2010 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include "notifyconfig.h"
#include <KPluginFactory>
#include <klocale.h>
#include <qlayout.h>
#include <QVBoxLayout>
#include <kstandarddirs.h>
#include <QFile>

#include "notifysettings.h"
#include <account.h>
#include <accountmanager.h>
#include <KDebug>
K_PLUGIN_FACTORY( NotifyConfigFactory, registerPlugin < NotifyConfig > (); )
K_EXPORT_PLUGIN( NotifyConfigFactory( "kcm_choqok_notify" ) )

NotifyConfig::NotifyConfig(QWidget* parent, const QVariantList& args):
        KCModule( NotifyConfigFactory::componentData(), parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName("mNotifyCtl");
    ui.setupUi(wd);
    layout->addWidget(wd);
    connect(ui.accountsList, SIGNAL(currentRowChanged(int)), SLOT(updateTimelinesList()));
    connect(ui.timelinesList, SIGNAL(itemSelectionChanged()), SLOT(timelineSelectionChanged()));
}

NotifyConfig::~NotifyConfig()
{

}

void NotifyConfig::updateTimelinesList()
{
    ui.timelinesList->blockSignals(true);
    ui.timelinesList->clear();
    QString acc = ui.accountsList->currentItem()->text();
    Choqok::Account* account = Choqok::AccountManager::self()->findAccount(acc);
    foreach(const QString& tm, account->timelineNames()){
        ui.timelinesList->addItem(tm);
        if(accounts[acc].contains(tm))
            ui.timelinesList->item(ui.timelinesList->count()-1)->setSelected(true);
    }
    ui.timelinesList->blockSignals(false);
}

void NotifyConfig::timelineSelectionChanged()
{
    QStringList lst;
    foreach(QListWidgetItem* item, ui.timelinesList->selectedItems()){
        lst.append(item->text());
    }
    accounts[ui.accountsList->currentItem()->text()] = lst;
    emit changed();
}

void NotifyConfig::load()
{
    accounts = NotifySettings().accounts();

    foreach(const QString& acc, accounts.keys()){
        ui.accountsList->addItem(acc);
    }
    if(ui.accountsList->count()>0) {
        ui.accountsList->setCurrentRow(0);
        updateTimelinesList();
    }
}

void NotifyConfig::save()
{
    kDebug()<< accounts.keys();
    NotifySettings s;
    s.setAccounts(accounts);
    s.save();
}

#include "notifyconfig.moc"
