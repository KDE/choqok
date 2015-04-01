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

#include "addaccountdialog.h"

#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

#include "accountmanager.h"
#include "accountsdebug.h"
#include "editaccountwidget.h"

AddAccountDialog::AddAccountDialog(ChoqokEditAccountWidget *addWidget, QWidget *parent, Qt::WFlags flags)
    : QDialog(parent, flags), widget(addWidget)
{
    if (!widget) {
        this->deleteLater();
        return;
    }

    setWindowTitle(i18n("Add New Account"));

    QVBoxLayout *mainLayout = new QVBoxLayout;
    setLayout(mainLayout);
    mainLayout->addWidget(widget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));
    mainLayout->addWidget(buttonBox);
}

AddAccountDialog::~AddAccountDialog()
{
}

void AddAccountDialog::accept()
{
    qCDebug(CHOQOK);
    if (widget->validateData()) {
        if (Choqok::Account *acc = widget->apply()) {
            if (!Choqok::AccountManager::self()->registerAccount(acc)) {
                KMessageBox::detailedError(this, i18n("The Account registration failed."),
                                            Choqok::AccountManager::self()->lastError());
            } else {
                QDialog::accept();
            }
        }
    } else {
        KMessageBox::sorry(this, i18n("Cannot validate your input information.\nPlease check the fields' data.\nMaybe a required field is empty?"));
    }
}

void AddAccountDialog::reject()
{
    Choqok::AccountManager::self()->removeAccount(widget->account()->alias());
    QDialog::reject();
}
