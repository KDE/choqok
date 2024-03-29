/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "microblogwidget.h"

#include <QKeyEvent>
#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QPointer>
#include <QPushButton>
#include <QStatusBar>
#include <QTime>
#include <QVBoxLayout>

#include <KMessageBox>

#include "account.h"
#include "choqokappearancesettings.h"
#include "choqoktextedit.h"
#include "choqokuiglobal.h"
#include "composerwidget.h"
#include "libchoqokdebug.h"
#include "notifymanager.h"
#include "timelinewidget.h"

namespace Choqok
{
namespace UI
{

QIcon addNumToIcon(const QIcon &big , int number , const QSize &result_size , const QPalette &palette)
{
    QIcon result;

    QList<QIcon::Mode> mods;
    mods << QIcon::Active /*<< QIcon::Disabled << QIcon::Selected*/;

    for (const QIcon::Mode &m: mods) {
        QPixmap pixmap = big.pixmap(result_size);
        QPainter painter(&pixmap);
        QFont font;
        font.setWeight(result_size.height() / 2);
        font.setBold(true);
        font.setItalic(true);
        painter.setFont(font);

        QString numberStr = QString::number(number);
        int textWidth = painter.fontMetrics().horizontalAdvance(numberStr) + 6;

        if (textWidth < result_size.width() / 2) {
            textWidth = result_size.width() / 2;
        }

        QRect rct(result_size.width() - textWidth , result_size.width() / 2 ,
                  textWidth , result_size.height() / 2);
        QPointF center(rct.x() + rct.width() / 2 , rct.y() + rct.height() / 2);

        QPainterPath cyrcle_path;
        cyrcle_path.moveTo(center);
        cyrcle_path.arcTo(rct, 0, 360);

        painter.setRenderHint(QPainter::Antialiasing);
        painter.fillPath(cyrcle_path , palette.color(QPalette::Active , QPalette::Window));
        painter.setPen(palette.color(QPalette::Active , QPalette::Text));
        painter.drawText(rct , Qt::AlignHCenter | Qt::AlignVCenter , QString::number(number));

        result.addPixmap(pixmap , m);
    }

    return result;
}

class MicroBlogWidget::Private
{
public:
    Private(Account *acc)
        : account(acc), blog(acc->microblog()), composer(nullptr), btnMarkAllAsRead(nullptr)
    {
    }
    Account *account;
    MicroBlog *blog;
    QPointer<ComposerWidget> composer;
    QMap<QString, TimelineWidget *> timelines;
    Choqok::UI::ChoqokTabBar *timelinesTabWidget;
    QLabel *latestUpdate;
    QPushButton *btnMarkAllAsRead;
    QHBoxLayout *toolbar;
    QFrame *toolbar_widget;
};

MicroBlogWidget::MicroBlogWidget(Account *account, QWidget *parent)
    : QWidget(parent), d(new Private(account))
{
    qCDebug(CHOQOK);
    connect(d->blog, &MicroBlog::timelineDataReceived, this, &MicroBlogWidget::newTimelineDataRecieved);
    connect(d->blog, &MicroBlog::error, this, &MicroBlogWidget::error);
    connect(d->blog, &MicroBlog::errorPost, this, &MicroBlogWidget::errorPost);
}

Account *MicroBlogWidget::currentAccount() const
{
    return d->account;
}

void MicroBlogWidget::initUi()
{
    d->toolbar_widget = new QFrame();
    d->toolbar_widget->setFrameShape(QFrame::StyledPanel);
    d->toolbar_widget->setFrameShadow(QFrame::Sunken);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QVBoxLayout *toolbar_layout = new QVBoxLayout(d->toolbar_widget);
    toolbar_layout->addLayout(createToolbar());

    d->timelinesTabWidget = new Choqok::UI::ChoqokTabBar(this);
    d->timelinesTabWidget->setLinkedTabBar(true);
    d->timelinesTabWidget->setTabCloseActivatePrevious(true);
    d->timelinesTabWidget->setExtraWidget(d->toolbar_widget , Choqok::UI::ChoqokTabBar::Top);

    if (!d->account->isReadOnly()) {
        setComposerWidget(d->blog->createComposerWidget(currentAccount(), this));
    }

    layout->addWidget(d->timelinesTabWidget);
    this->layout()->setContentsMargins(0, 0, 0, 0);
    connect(currentAccount(), &Account::modified, this, &MicroBlogWidget::slotAccountModified);
    initTimelines();
}

void MicroBlogWidget::setComposerWidget(ComposerWidget *widget)
{
    if (d->composer) {
        d->composer->deleteLater();
    }
    if (!widget) {
        d->composer = nullptr;
        return;
    }
    d->composer = widget;
    d->composer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    qobject_cast<QVBoxLayout *>(d->toolbar_widget->layout())->insertWidget(1, d->composer);
    for (TimelineWidget *mbw: d->timelines) {
        connect(mbw, SIGNAL(forwardResendPost(QString)), d->composer, SLOT(setText(QString)));
        connect(mbw, &TimelineWidget::forwardReply, d->composer, &ComposerWidget::setText);
    }
}

MicroBlogWidget::~MicroBlogWidget()
{
    qCDebug(CHOQOK);
    delete d;
}

TimelineWidget *MicroBlogWidget::currentTimeline()
{
    return qobject_cast<TimelineWidget *>(d->timelinesTabWidget->currentWidget());
}

uint MicroBlogWidget::unreadCount() const
{
    uint sum = 0;
    for (TimelineWidget *wd: d->timelines) {
        sum += wd->unreadCount();
    }

    return sum;
}

void MicroBlogWidget::settingsChanged()
{
    for (TimelineWidget *wd: d->timelines) {
        wd->settingsChanged();
    }
}

void MicroBlogWidget::updateTimelines()
{
    qCDebug(CHOQOK) << d->account->alias();
    d->account->microblog()->updateTimelines(currentAccount());
}

void MicroBlogWidget::removeOldPosts()
{
    for (TimelineWidget *wd: d->timelines) {
        wd->removeOldPosts();
    }
}

void MicroBlogWidget::newTimelineDataRecieved(Choqok::Account *theAccount, const QString &type,
        QList< Choqok::Post * > data)
{
    if (theAccount != currentAccount()) {
        return;
    }

    qCDebug(CHOQOK) << d->account->alias() << ":" << type;
    d->latestUpdate->setText(QTime::currentTime().toString());
    if (d->timelines.contains(type)) {
        d->timelines.value(type)->addNewPosts(data);
    } else {
        if (TimelineWidget *wd = addTimelineWidgetToUi(type)) {
            wd->addNewPosts(data);
        }
    }
}

void MicroBlogWidget::initTimelines()
{
    qCDebug(CHOQOK);
    for (const QString &timeline: d->account->timelineNames()) {
        addTimelineWidgetToUi(timeline);
    }
//     qCDebug(CHOQOK)<<"========== Emiting loaded()";
    Q_EMIT loaded();
}

TimelineWidget *MicroBlogWidget::addTimelineWidgetToUi(const QString &name)
{
    TimelineWidget *mbw = d->blog->createTimelineWidget(d->account, name, this);
    if (mbw) {
        Choqok::TimelineInfo *info = currentAccount()->microblog()->timelineInfo(name);
        d->timelines.insert(name, mbw);
        d->timelinesTabWidget->addTab(mbw, info->name);
        d->timelinesTabWidget->setTabIcon(d->timelinesTabWidget->indexOf(mbw), QIcon::fromTheme(info->icon));
        connect(mbw, SIGNAL(updateUnreadCount(int)), this, SLOT(slotUpdateUnreadCount(int)));
        if (d->composer) {
            connect(mbw, SIGNAL(forwardResendPost(QString)), d->composer, SLOT(setText(QString)));
            connect(mbw, &TimelineWidget::forwardReply, d->composer, &ComposerWidget::setText);
        }
        slotUpdateUnreadCount(mbw->unreadCount(), mbw);
    } else {
        qCDebug(CHOQOK) << "Cannot Create a new TimelineWidget for timeline " << name;
        return nullptr;
    }
    if (d->timelinesTabWidget->count() == 1) {
        d->timelinesTabWidget->setTabBarHidden(true);
    } else {
        d->timelinesTabWidget->setTabBarHidden(false);
    }
    return mbw;
}

void MicroBlogWidget::slotUpdateUnreadCount(int change, Choqok::UI::TimelineWidget *widget)
{
    qCDebug(CHOQOK) << change;
    int sum = 0;
    for (TimelineWidget *mbw: d->timelines) {
        sum += mbw->unreadCount();
    }
    if (change != 0) {
        Q_EMIT updateUnreadCount(change, sum);
    }

    if (sum > 0) {
        if (!d->btnMarkAllAsRead) {
            d->btnMarkAllAsRead = new QPushButton(this);
            d->btnMarkAllAsRead->setIcon(QIcon::fromTheme(QLatin1String("mail-mark-read")));
            d->btnMarkAllAsRead->setIconSize(QSize(14, 14));
            d->btnMarkAllAsRead->setToolTip(i18n("Mark all timelines as read"));
            d->btnMarkAllAsRead->setMaximumWidth(d->btnMarkAllAsRead->height());
            connect(d->btnMarkAllAsRead, &QPushButton::clicked, this, &MicroBlogWidget::markAllAsRead);
            d->toolbar->insertWidget(1, d->btnMarkAllAsRead);
        }
    } else {
        d->btnMarkAllAsRead->deleteLater();
        d->btnMarkAllAsRead = nullptr;
    }
    TimelineWidget *wd = qobject_cast<TimelineWidget *>(sender());
    if (!wd) {
        wd = widget;
    }
    if (wd) {
        qCDebug(CHOQOK) << wd->unreadCount();
        int tabIndex = d->timelinesTabWidget->indexOf(wd);
        if (tabIndex == -1) {
            return;
        }
        if (wd->unreadCount() > 0) {
            d->timelinesTabWidget->setTabIcon(tabIndex , addNumToIcon(timelinesTabWidget()->tabIcon(tabIndex) , wd->unreadCount() , QSize(40, 40) , palette()));
            d->timelinesTabWidget->setTabText(tabIndex, wd->timelineInfoName() +
                                              QStringLiteral(" (%1)").arg(wd->unreadCount()));
        } else {
            if (!wd->timelineIconName().isEmpty()) {
                d->timelinesTabWidget->setTabIcon(tabIndex , QIcon::fromTheme(wd->timelineIconName()));
            } else {
                d->timelinesTabWidget->setTabIcon(tabIndex , wd->timelineIcon());
            }

            d->timelinesTabWidget->setTabText(tabIndex, wd->timelineInfoName());
        }
    }
}

void MicroBlogWidget::markAllAsRead()
{
    if (d->btnMarkAllAsRead) {
        d->btnMarkAllAsRead->deleteLater();
        d->btnMarkAllAsRead = nullptr;
    }
    for (TimelineWidget *wd: d->timelines) {
        wd->markAllAsRead();
        int tabIndex = d->timelinesTabWidget->indexOf(wd);
        if (tabIndex == -1) {
            continue;
        }
        d->timelinesTabWidget->setTabText(tabIndex, wd->timelineInfoName());
    }
}

ComposerWidget *MicroBlogWidget::composer()
{
    return d->composer;
}

QMap< QString, TimelineWidget * > &MicroBlogWidget::timelines()
{
    return d->timelines;
}

Choqok::UI::ChoqokTabBar *MicroBlogWidget::timelinesTabWidget()
{
    return d->timelinesTabWidget;
}

void MicroBlogWidget::error(Choqok::Account *theAccount, MicroBlog::ErrorType errorType,
                            const QString &errorMsg, MicroBlog::ErrorLevel level)
{
    if (theAccount == d->account) {
        switch (level) {
        case MicroBlog::Critical:
            KMessageBox::error(Choqok::UI::Global::mainWindow(), errorMsg, MicroBlog::errorString(errorType));
            break;
        case MicroBlog::Normal:
            NotifyManager::error(errorMsg, MicroBlog::errorString(errorType));
            break;
        default:
//             emit showStatusMessage(errorMsg);
            if (Choqok::UI::Global::mainWindow()->statusBar()) {
                Choqok::UI::Global::mainWindow()->statusBar()->showMessage(errorMsg);
            }
            break;
        };
    }
}
void MicroBlogWidget::errorPost(Choqok::Account *theAccount, Choqok::Post *, MicroBlog::ErrorType errorType,
                                const QString &errorMsg, MicroBlog::ErrorLevel level)
{
    if (theAccount == d->account) {
        switch (level) {
        case MicroBlog::Critical:
            KMessageBox::error(Choqok::UI::Global::mainWindow(), errorMsg, MicroBlog::errorString(errorType));
            break;
        case MicroBlog::Normal:
            NotifyManager::error(errorMsg, MicroBlog::errorString(errorType));
            break;
        default:
//             emit showStatusMessage(errorMsg);
            if (Choqok::UI::Global::mainWindow()->statusBar()) {
                Choqok::UI::Global::mainWindow()->statusBar()->showMessage(errorMsg);
            }
            break;
        };
    }
}

QLayout *MicroBlogWidget::createToolbar()
{
    d->toolbar = new QHBoxLayout;
    QPushButton *btnActions = new QPushButton(i18n("More"), this);

    QLabel *lblLatestUpdate = new QLabel(i18n("Latest update:"), this);
    lblLatestUpdate->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    d->latestUpdate = new QLabel(QTime::currentTime().toString(), this);
    QFont fnt = lblLatestUpdate->font();
    fnt.setPointSize(fnt.pointSize() - 1);
    lblLatestUpdate->setFont(fnt);
    fnt.setBold(true);
    d->latestUpdate->setFont(fnt);

    btnActions->setMenu(d->account->microblog()->createActionsMenu(d->account));
    d->toolbar->addWidget(btnActions);
    d->toolbar->addSpacerItem(new QSpacerItem(1, 10, QSizePolicy::Expanding));
    d->toolbar->addWidget(lblLatestUpdate);
    d->toolbar->addWidget(d->latestUpdate);
    return d->toolbar;
}

void MicroBlogWidget::slotAbortAllJobs()
{
    currentAccount()->microblog()->abortAllJobs(currentAccount());
    composer()->abort();
}

void MicroBlogWidget::keyPressEvent(QKeyEvent *e)
{
    if (e->key() == Qt::Key_Escape && composer()) {
        composer()->abort();
    }
    QWidget::keyPressEvent(e);
}

void MicroBlogWidget::setFocus()
{
    if (composer()) {
        composer()->editor()->setFocus(Qt::OtherFocusReason);
    } else {
        QWidget::setFocus();
    }
}

void MicroBlogWidget::slotAccountModified(Account *theAccount)
{
    if (theAccount == currentAccount()) {
        if (theAccount->isReadOnly()) {
            if (composer()) {
                setComposerWidget(nullptr);
            }
        } else if (!composer()) {
            setComposerWidget(theAccount->microblog()->createComposerWidget(theAccount, this));
        }
        int sum = 0;
        for (TimelineWidget *mbw: d->timelines) {
            sum += mbw->unreadCount();
        }
        Q_EMIT updateUnreadCount(0, sum);
    }
}

QLabel *MicroBlogWidget::latestUpdate()
{
    return d->latestUpdate;
}

}
}

#include "moc_microblogwidget.cpp"
