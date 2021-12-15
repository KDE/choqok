/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "notifyconfig.h"

#include <QListWidget>
#include <QPushButton>
#include <QSpinBox>

#include <KAboutData>
#include <KColorButton>
#include <KFontRequester>
#include <KPluginFactory>

#include "account.h"
#include "accountmanager.h"

#include "dummynotification.h"
#include "notifysettings.h"

K_PLUGIN_FACTORY_WITH_JSON(NotifyConfigFactory, "choqok_notify_config.json",
                           registerPlugin < NotifyConfig > ();)

NotifyConfig::NotifyConfig(QWidget *parent, const QVariantList &args)
    : KCModule(parent, args)
{
    QVBoxLayout *layout = new QVBoxLayout(this);
    QWidget *wd = new QWidget(this);
    wd->setObjectName(QLatin1String("mNotifyCtl"));
    ui.setupUi(wd);
    layout->addWidget(wd);
    connect(ui.accountsList, &QListWidget::currentRowChanged,
            this, &NotifyConfig::updateTimelinesList);
    connect(ui.timelinesList, &QListWidget::itemSelectionChanged,
            this, &NotifyConfig::timelineSelectionChanged);
    connect(ui.interval, (void (QSpinBox::*)(int))&QSpinBox::valueChanged, this, &NotifyConfig::emitChanged);
    connect(ui.adjustPosition, &QPushButton::clicked, this, &NotifyConfig::slotAdjustNotificationPosition);
    connect(ui.backgroundColor, &KColorButton::changed, this, &NotifyConfig::emitChanged);
    connect(ui.foregroundColor, &KColorButton::changed, this, &NotifyConfig::emitChanged);
    connect(ui.font, &KFontRequester::fontSelected, this, &NotifyConfig::emitChanged);
    settings = new NotifySettings(this);
    ui.lblArrow->setPixmap(QIcon::fromTheme(QLatin1String("arrow-right")).pixmap(48));
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
    Choqok::Account *account = Choqok::AccountManager::self()->findAccount(acc);
    for (const QString &tm: account->timelineNames()) {
        ui.timelinesList->addItem(tm);
        if (accounts[acc].contains(tm)) {
            ui.timelinesList->item(ui.timelinesList->count() - 1)->setSelected(true);
        }
    }
    ui.timelinesList->blockSignals(false);
}

void NotifyConfig::timelineSelectionChanged()
{
    QStringList lst;
    for (QListWidgetItem *item: ui.timelinesList->selectedItems()) {
        lst.append(item->text());
    }
    accounts[ui.accountsList->currentItem()->text()] = lst;
    Q_EMIT changed();
}

void NotifyConfig::load()
{
    accounts = settings->accounts();

    ui.interval->setValue(settings->notifyInterval());

    for (const QString &acc: accounts.keys()) {
        ui.accountsList->addItem(acc);
    }
    if (ui.accountsList->count() > 0) {
        ui.accountsList->setCurrentRow(0);
        updateTimelinesList();
    }

    ui.backgroundColor->setColor(settings->backgroundColor());
    ui.foregroundColor->setColor(settings->foregroundColor());
    ui.font->setFont(settings->font());
}

void NotifyConfig::save()
{
    //qDebug()<< accounts.keys();
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
    if (!dummy) {
        dummy = new DummyNotification(ui.font->font(), ui.foregroundColor->color(),
                                      ui.backgroundColor->color(), this);
        dummy->setAttribute(Qt::WA_DeleteOnClose);
        dummy->resize(NOTIFICATION_WIDTH, NOTIFICATION_HEIGHT);
        connect(dummy, &DummyNotification::positionSelected, this, &NotifyConfig::slotNewPositionSelected);
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
