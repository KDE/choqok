/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "addeditfilter.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QPushButton>

#include <KLocalizedString>

#include "filtersettings.h"

AddEditFilter::AddEditFilter(QWidget *parent, Filter *filter)
    : QDialog(parent), currentFilter(filter)
{
    ui.setupUi(this);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddEditFilter::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddEditFilter::reject);
    ui.formLayout->addWidget(buttonBox);

    connect(ui.filterAction, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &AddEditFilter::slotFilterActionChanged);

    setupFilterFields();
    setupFilterTypes();
    setupFilterActions();

    if (filter) {
        //qDebug() << filter->filterField();
        //Editing
        ui.filterField->setCurrentIndex(ui.filterField->findData(filter->filterField()));
        ui.filterType->setCurrentIndex(ui.filterType->findData(filter->filterType()));
        ui.filterAction->setCurrentIndex(ui.filterAction->findData(filter->filterAction()));
        ui.filterText->setText(filter->filterText());
        ui.dontHideReplies->setChecked(filter->dontHideReplies());
        setWindowTitle(i18n("Modify filter rules"));
    }
    ui.filterText->setFocus();

}

AddEditFilter::~AddEditFilter()
{
}

void AddEditFilter::accept()
{
    Filter::FilterField field =
        (Filter::FilterField) ui.filterField->itemData(ui.filterField->currentIndex()).toInt();
    Filter::FilterType type =
        (Filter::FilterType) ui.filterType->itemData(ui.filterType->currentIndex()).toInt();
    Filter::FilterAction action =
        (Filter::FilterAction) ui.filterAction->itemData(ui.filterAction->currentIndex()).toInt();
    QString fText = ui.filterText->text();
    bool dontHideReplies = ui.dontHideReplies->isChecked();
    if (currentFilter) {
        currentFilter->setFilterField(field);
        currentFilter->setFilterText(fText);
        currentFilter->setFilterType(type);
        currentFilter->setFilterAction(action);
        currentFilter->setDontHideReplies(dontHideReplies);
        Q_EMIT filterUpdated(currentFilter);
    } else {
        currentFilter = new Filter(fText, field, type, action, dontHideReplies);
        Q_EMIT newFilterRegistered(currentFilter);
    }
    QDialog::accept();
}

void AddEditFilter::slotFilterActionChanged(int index)
{
    ui.dontHideReplies->setVisible((Filter::FilterAction)ui.filterAction->itemData(index).toInt() == Filter::Remove);
}

void AddEditFilter::setupFilterFields()
{
    const QMap<Filter::FilterField, QString> fields = FilterSettings::filterFieldsMap();
    for (const Filter::FilterField &field: fields.keys()) {
        ui.filterField->addItem(fields.value(field), field);
    }
}

void AddEditFilter::setupFilterTypes()
{
    const QMap<Filter::FilterType, QString> types = FilterSettings::filterTypesMap();
    for (const Filter::FilterType &type: types.keys()) {
        ui.filterType->addItem(types.value(type), type);
    }
}

void AddEditFilter::setupFilterActions()
{
    const QMap<Filter::FilterAction, QString> actions = FilterSettings::filterActionsMap();
    for (const Filter::FilterAction &action: actions.keys()) {
        ui.filterAction->addItem(actions.value(action), action);
    }
}

#include "moc_addeditfilter.cpp"
