/*
    This file is part of Choqok, the KDE micro-blogging client

    Copyright (C) 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

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

#include <QFile>
#include <QVBoxLayout>

#include "choqokdebug.h"
#include <KLocale>
#include <KPluginFactory>
#include <KStandardDirs>

#include "account.h"
#include "accountmanager.h"
#include "notifysettings.h"

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
    connect(ui.interval, SIGNAL(valueChanged(int)), this, SLOT(emitChanged()));
    connect(ui.adjustPosition, SIGNAL(clicked()), this, SLOT(slotAdjustNotificationPosition()) );
    connect(ui.backgroundColor, SIGNAL(changed(QColor)), this, SLOT(emitChanged()));
    connect(ui.foregroundColor, SIGNAL(changed(QColor)), this, SLOT(emitChanged()));
    connect(ui.font, SIGNAL(fontSelected(QFont)), this, SLOT(emitChanged()));
    settings = new NotifySettings(this);
    ui.lblArrow->setPixmap(QIcon::fromTheme("arrow-right").pixmap(48));
}

NotifyConfig::~NotifyConfig()
{
}

void NotifyConfig::emitChanged()
{
    Q_EMIT changed(true);
}


void NotifyConfig::updateTimelinesList()
{
    ui.timelinesList->blockSignals(true);
    ui.timelinesList->clear();
    QString acc = ui.accountsList->currentItem()->text();
    Choqok::Account* account = Choqok::AccountManager::self()->findAccount(acc);
    Q_FOREACH (const QString& tm, account->timelineNames()) {
        ui.timelinesList->addItem(tm);
        if(accounts[acc].contains(tm))
            ui.timelinesList->item(ui.timelinesList->count()-1)->setSelected(true);
    }
    ui.timelinesList->blockSignals(false);
}

void NotifyConfig::timelineSelectionChanged()
{
    QStringList lst;
    Q_FOREACH (QListWidgetItem* item, ui.timelinesList->selectedItems()) {
        lst.append(item->text());
    }
    accounts[ui.accountsList->currentItem()->text()] = lst;
    Q_EMIT changed();
}

void NotifyConfig::load()
{
    accounts = settings->accounts();

    ui.interval->setValue(settings->notifyInterval());

    Q_FOREACH (const QString& acc, accounts.keys()) {
        ui.accountsList->addItem(acc);
    }
    if(ui.accountsList->count()>0) {
        ui.accountsList->setCurrentRow(0);
        updateTimelinesList();
    }

    ui.backgroundColor->setColor(settings->backgroundColor());
    ui.foregroundColor->setColor(settings->foregroundColor());
    ui.font->setFont(settings->font());
}

void NotifyConfig::save()
{
    qCDebug(CHOQOK)<< accounts.keys();
    settings->setAccounts(accounts);
    settings->setNotifyInterval(ui.interval->value());
    settings->setBackgroundColor(ui.backgroundColor->color());
    settings->setForegroundColor(ui.foregroundColor->color());
    settings->setFont(ui.font->font());
    settings->save();
}

void NotifyConfig::slotAdjustNotificationPosition()
{
    ui.adjustPosition->setDisabled(true);
    if(!dummy){
        dummy = new DummyNotification(ui.font->font(), ui.foregroundColor->color(),
                                      ui.backgroundColor->color(), this);
        dummy->setAttribute(Qt::WA_DeleteOnClose);
        dummy->resize(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);
        connect(dummy, SIGNAL(positionSelected(QPoint)), this, SLOT(slotNewPositionSelected(QPoint)));
    }
    dummy->move(settings->position());
    dummy->show();
}

void NotifyConfig::slotNewPositionSelected(QPoint pos)
{
    settings->setPosition(pos);
    dummy->close();
    ui.adjustPosition->setEnabled(true);
    emitChanged();
}

#include "notifyconfig.moc"
