/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "quickpost.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QPointer>
#include <QPushButton>

#include <KLocalizedString>
#include <KMessageBox>

#include "accountmanager.h"
#include "choqokbehaviorsettings.h"
#include "choqoktextedit.h"
#include "libchoqokdebug.h"
#include "microblog.h"
#include "notifymanager.h"
#include "shortenmanager.h"
#include "uploadmediadialog.h"

using namespace Choqok::UI;
using namespace Choqok;

class QuickPost::Private
{
public:
    Private()
        : submittedPost(nullptr), isPostSubmitted(false)
    {}
    QCheckBox *all;
    QComboBox *comboAccounts;
    TextEdit *txtPost;

    QHash< QString, Account * > accountsList;
    Post *submittedPost;
    QList<Account *> submittedAccounts;
    bool isPostSubmitted;
    QPushButton *attach;
//     QString replyToId;
};

QuickPost::QuickPost(QWidget *parent)
    : QDialog(parent), d(new Private)
{
    qCDebug(CHOQOK);
    setupUi();
    loadAccounts();
    connect(d->comboAccounts, (void (QComboBox::*)(int))&QComboBox::currentIndexChanged,
            this, &QuickPost::slotCurrentAccountChanged);
    connect(d->txtPost, &TextEdit::returnPressed, this, &QuickPost::submitPost);
    connect(d->all, &QCheckBox::toggled, this, &QuickPost::checkAll);
    connect(AccountManager::self(), &AccountManager::accountAdded, this, &QuickPost::addAccount);
    connect(AccountManager::self(), &AccountManager::accountRemoved, this, &QuickPost::removeAccount);
    connect(d->attach, &QPushButton::clicked, this, &QuickPost::slotAttachMedium);

    d->all->setChecked(Choqok::BehaviorSettings::all());
    slotCurrentAccountChanged(d->comboAccounts->currentIndex());
}

void QuickPost::setupUi()
{
    resize(Choqok::BehaviorSettings::quickPostDialogSize());
    d->all = new QCheckBox(i18nc("All accounts", "All"), this);
    d->comboAccounts = new QComboBox(this);
    d->attach = new QPushButton(QIcon::fromTheme(QLatin1String("mail-attachment")), QString(), this);
    d->attach->setMaximumWidth(d->attach->height());
    d->attach->setToolTip(i18n("Attach a file"));
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    QHBoxLayout *hLayout = new QHBoxLayout;
    hLayout->addWidget(d->all);
    hLayout->addWidget(d->comboAccounts);
    hLayout->addWidget(d->attach);
    mainLayout->addLayout(hLayout);
    d->txtPost = new TextEdit(0, this);
    d->txtPost->setTabChangesFocus(true);
    mainLayout->addWidget(d->txtPost);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    QPushButton *okButton = buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDefault(true);
    okButton->setShortcut(Qt::CTRL | Qt::Key_Return);
    okButton->setText(i18nc("Submit post", "Submit"));
    connect(buttonBox, &QDialogButtonBox::accepted, this, &QuickPost::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QuickPost::reject);
    mainLayout->addWidget(buttonBox);

    setLayout(mainLayout);

    d->txtPost->setFocus(Qt::OtherFocusReason);
    setWindowTitle(i18n("Quick Post"));
}

QuickPost::~QuickPost()
{
    BehaviorSettings::setAll(d->all->isChecked());
    BehaviorSettings::setQuickPostDialogSize(this->size());
    BehaviorSettings::self()->save();
    delete d;
    qCDebug(CHOQOK);
}

void QuickPost::show()
{
    d->txtPost->setFocus(Qt::OtherFocusReason);
    QDialog::show();
}

void QuickPost::slotSubmitPost(Account *a, Post *post)
{
    if (post == d->submittedPost && d->submittedAccounts.removeOne(a)) {
        Q_EMIT newPostSubmitted(Success, d->submittedPost);
    }
    if (d->isPostSubmitted && d->submittedAccounts.isEmpty()) {
        d->txtPost->setEnabled(true);
        d->txtPost->clear();
        delete d->submittedPost;
        d->submittedPost = nullptr;
        d->isPostSubmitted = false;
    }
}

void QuickPost::postError(Account *a, Choqok::Post *post,
                          Choqok::MicroBlog::ErrorType , const QString &)
{
    if (post == d->submittedPost && d->submittedAccounts.contains(a)) {
        d->txtPost->setEnabled(true);
        Q_EMIT newPostSubmitted(Fail, post); //Choqok crashes without post :(
        show();
    }
    if (d->submittedAccounts.isEmpty()) {
        d->txtPost->setEnabled(true);
        delete d->submittedPost;
        d->submittedPost = nullptr;
    }
}

void QuickPost::submitPost(const QString &txt)
{
    qCDebug(CHOQOK);
    if (txt.isEmpty()) {
        KMessageBox::error(choqokMainWindow, i18n("Cannot create a post without any text."));
        return;
    }
    Choqok::Account *currentAccount = d->accountsList.value(d->comboAccounts->currentText());
    if (!currentAccount) {
        KMessageBox::error(choqokMainWindow, i18n("Please configure at least one account to be included in \"Quick Post\".\nSettings -> Configure Choqok... -> Accounts"));
        return;
    }
    this->hide();
    d->submittedAccounts.clear();
    QString newPost = txt;
    if (currentAccount->postCharLimit() &&
            txt.size() > (int)currentAccount->postCharLimit()) {
        newPost = Choqok::ShortenManager::self()->parseText(txt);
    }
    delete d->submittedPost;
    if (d->all->isChecked()) {
        d->submittedPost = new Post;
        d->submittedPost->content = newPost;
        d->submittedPost->isPrivate = false;
        for (Account *acc: d->accountsList) {
            acc->microblog()->createPost(acc, d->submittedPost);
            d->submittedAccounts << acc;
        }
    } else {
        d->submittedPost = new Post;
        d->submittedPost->content = newPost;
        d->submittedPost->isPrivate = false;
        d->submittedAccounts << currentAccount;
        currentAccount->microblog()->createPost(d->accountsList.value(d->comboAccounts->currentText()),
                                                d->submittedPost);
    }
    d->isPostSubmitted = true;
}

void QuickPost::accept()
{
    qCDebug(CHOQOK);
    submitPost(d->txtPost->toPlainText());
}

void QuickPost::loadAccounts()
{
    qCDebug(CHOQOK);
    for (Choqok::Account *ac: AccountManager::self()->accounts()) {
        addAccount(ac);
    }
}

void QuickPost::addAccount(Choqok::Account *account)
{
    qCDebug(CHOQOK);
    connect(account, &Account::modified, this, &QuickPost::accountModified); //Added for later changes
    if (!account->isEnabled() || account->isReadOnly() || !account->showInQuickPost()) {
        return;
    }
    d->accountsList.insert(account->alias(), account);
    d->comboAccounts->addItem(QIcon::fromTheme(account->microblog()->pluginIcon()), account->alias());
    connect(account->microblog(), &MicroBlog::postCreated, this, &QuickPost::slotSubmitPost);
    connect(account->microblog(), &MicroBlog::errorPost, this, &QuickPost::postError);
}

void QuickPost::removeAccount(const QString &alias)
{
    qCDebug(CHOQOK);
    d->accountsList.remove(alias);
    d->comboAccounts->removeItem(d->comboAccounts->findText(alias));
}

void QuickPost::checkAll(bool isAll)
{
    d->comboAccounts->setEnabled(!isAll);
}

void QuickPost::setText(const QString &text)
{
    d->txtPost->setPlainText(text);
    this->show();
//     if(account)
//         d->comboAccounts->setCurrentItem(account->alias());
//     if(!replyToId.isEmpty())
//         d->replyToId = replyToId;
}

void QuickPost::appendText(const QString &text)
{
    d->txtPost->appendText(text);
    this->show();
}

void QuickPost::slotCurrentAccountChanged(int index)
{
    Q_UNUSED(index)
    if (!d->accountsList.isEmpty()) {
        d->txtPost->setCharLimit(d->accountsList.value(d->comboAccounts->currentText())->postCharLimit());
    }
}

void QuickPost::accountModified(Account *theAccount)
{
    qCDebug(CHOQOK);
    if (theAccount->isEnabled() && !theAccount->isReadOnly() && theAccount->showInQuickPost()) {
        if (!d->accountsList.contains(theAccount->alias())) {
            addAccount(theAccount);
        }
    } else if (d->accountsList.contains(theAccount->alias())) {
        removeAccount(theAccount->alias());
    }
}

void QuickPost::slotAttachMedium()
{
    KMessageBox::information(this, i18n("Link to uploaded medium will be added here after uploading process succeed."),
                             QString(), QLatin1String("quickPostAttachMedium"));
    QPointer<UploadMediaDialog> dlg = new UploadMediaDialog(this);
    dlg->show();
}

#include "moc_quickpost.cpp"
