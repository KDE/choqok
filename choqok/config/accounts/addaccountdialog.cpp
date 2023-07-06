/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "addaccountdialog.h"

#include <QPushButton>
#include <QVBoxLayout>

#include <KLocalizedString>
#include <KMessageBox>

#include "accountmanager.h"
#include "accountsdebug.h"
#include "editaccountwidget.h"

AddAccountDialog::AddAccountDialog(ChoqokEditAccountWidget *addWidget, QWidget *parent, Qt::WindowFlags flags)
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
    connect(buttonBox, &QDialogButtonBox::accepted, this, &AddAccountDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &AddAccountDialog::reject);
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
      KMessageBox::error(
          this, i18n("Cannot validate your input information.\nPlease check "
                     "the fields' data.\nMaybe a required field is empty?"));
    }
}

#include "moc_addaccountdialog.cpp"
