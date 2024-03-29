/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "filtermanager.h"

#include <QAction>
#include <QTimer>

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KPluginFactory>

#include "choqokuiglobal.h"
#include "postwidget.h"
#include "quickpost.h"
#include "timelinewidget.h"

#include "twitterapiaccount.h"

#include "configurefilters.h"
#include "filter.h"
#include "filtersettings.h"

K_PLUGIN_CLASS_WITH_JSON(FilterManager, "choqok_filter.json")

FilterManager::FilterManager(QObject *parent, const QList<QVariant> &)
    : Choqok::Plugin(QLatin1String("choqok_filter"), parent), state(Stopped)
{
    QAction *action = new QAction(i18n("Configure Filters..."), this);
    actionCollection()->addAction(QLatin1String("configureFilters"), action);
    connect(action, &QAction::triggered, this, &FilterManager::slotConfigureFilters);
    setXMLFile(QLatin1String("filterui.rc"));
    connect(Choqok::UI::Global::SessionManager::self(), &Choqok::UI::Global::SessionManager::newPostWidgetAdded,
            this, &FilterManager::slotAddNewPostWidget);

    hidePost = new QAction(i18n("Hide Post"), this);
    Choqok::UI::PostWidget::addAction(hidePost);
    connect(hidePost, &QAction::triggered, this, &FilterManager::slotHidePost);
}

FilterManager::~FilterManager()
{

}

void FilterManager::slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget)
{
    postsQueue.enqueue(newWidget);
    if (state == Stopped) {
        state = Running;
        QTimer::singleShot(1000, this, SLOT(startParsing()));
    }
}

void FilterManager::startParsing()
{
    int i = 8;
    while (!postsQueue.isEmpty() && i > 0) {
        parse(postsQueue.dequeue());
        --i;
    }

    if (postsQueue.isEmpty()) {
        state = Stopped;
    } else {
        QTimer::singleShot(500, this, SLOT(startParsing()));
    }
}

void FilterManager::parse(Choqok::UI::PostWidget *postToParse)
{
    if (!postToParse ||
            postToParse->currentPost()->author.userName == postToParse->currentAccount()->username() ||
            postToParse->isRead()) {
        return;
    }

    if (parseSpecialRules(postToParse)) {
        return;
    }

    if (!postToParse) {
        return;
    }
    //qDebug() << "Processing: "<<postToParse->content();
    for (Filter *filter: FilterSettings::self()->filters()) {
        if (filter->filterText().isEmpty()) {
            return;
        }
        if (filter->filterAction() == Filter::Remove && filter->dontHideReplies() &&
                (postToParse->currentPost()->replyToUser.userName.compare(postToParse->currentAccount()->username(),
                        Qt::CaseInsensitive) == 0 ||
                 postToParse->currentPost()->content.contains(QStringLiteral("@%1").arg(postToParse->currentAccount()->username())))
           ) {
            continue;
        }
        switch (filter->filterField()) {
        case Filter::Content:
            doFiltering(postToParse, filterText(postToParse->currentPost()->content, filter));
            break;
        case Filter::AuthorUsername:
            doFiltering(postToParse, filterText(postToParse->currentPost()->author.userName, filter));
            break;
        case Filter::ReplyToUsername:
            doFiltering(postToParse, filterText(postToParse->currentPost()->replyToUser.userName, filter));
            break;
        case Filter::Source:
            doFiltering(postToParse, filterText(postToParse->currentPost()->source, filter));
            break;
        default:
            break;
        };
    }
}

Filter::FilterAction FilterManager::filterText(const QString &textToCheck, Filter *filter)
{
    bool filtered = false;
    switch (filter->filterType()) {
    case Filter::ExactMatch:
        if (textToCheck.compare(filter->filterText(), Qt::CaseInsensitive) == 0) {
            //qDebug() << "ExactMatch:" << filter->filterText();
            filtered = true;
        }
        break;
    case Filter::RegExp:
        if (textToCheck.contains(QRegExp(filter->filterText()))) {
            //qDebug() << "RegExp:" << filter->filterText();
            filtered = true;
        }
        break;
    case Filter::Contain:
        if (textToCheck.contains(filter->filterText(), Qt::CaseInsensitive)) {
            //qDebug() << "Contain:" << filter->filterText();
            filtered = true;
        }
        break;
    case Filter::DoesNotContain:
        if (!textToCheck.contains(filter->filterText(), Qt::CaseInsensitive)) {
            //qDebug() << "DoesNotContain:" << filter->filterText();
            filtered = true;
        }
        break;
    default:
        break;
    }
    if (filtered) {
        return filter->filterAction();
    } else {
        return Filter::None;
    }
}

void FilterManager::doFiltering(Choqok::UI::PostWidget *postToFilter, Filter::FilterAction action)
{
    QString css;
    switch (action) {
    case Filter::Remove:
        //qDebug() << "Post removed:" << postToFilter->currentPost()->content;
        postToFilter->close();
        break;
    case Filter::Highlight:
        css = postToFilter->styleSheet();
        css.replace(QLatin1String("border: 1px solid rgb(150,150,150)"), QLatin1String("border: 2px solid rgb(255,0,0)"));
        postToFilter->setStyleSheet(css);
        break;
    case Filter::None:
    default:
        //Do nothing
        break;
    }
}

void FilterManager::slotConfigureFilters()
{
    QPointer<ConfigureFilters> dlg = new ConfigureFilters(Choqok::UI::Global::mainWindow());
    dlg->show();
}

bool FilterManager::parseSpecialRules(Choqok::UI::PostWidget *postToParse)
{
    if (FilterSettings::hideRepliesNotRelatedToMe()) {
        if (!postToParse->currentPost()->replyToUser.userName.isEmpty() &&
                postToParse->currentPost()->replyToUser.userName != postToParse->currentAccount()->username()) {
            if (!postToParse->currentPost()->content.contains(postToParse->currentAccount()->username())) {
                postToParse->close();
//                qDebug() << "NOT RELATE TO ME FILTERING......";
                return true;
            }
        }
    }

    if (FilterSettings::hideNoneFriendsReplies()) {
        TwitterApiAccount *acc = qobject_cast<TwitterApiAccount *>(postToParse->currentAccount());
        if (!acc) {
            return false;
        }
        if (!postToParse->currentPost()->replyToUser.userName.isEmpty() &&
                !acc->friendsList().contains(postToParse->currentPost()->replyToUser.userName)) {
            if (!postToParse->currentPost()->content.contains(postToParse->currentAccount()->username())) {
                postToParse->close();
//                qDebug() << "NONE FRIEND FILTERING......";
                return true;
            }
        }
    }

    return false;
}

void FilterManager::slotHidePost()
{
    Choqok::UI::PostWidget *wd;
    wd = dynamic_cast<Choqok::UI::PostWidgetUserData *>(hidePost->userData(32))->postWidget();
    QString username = wd->currentPost()->author.userName;
    int res = KMessageBox::questionYesNoCancel(choqokMainWindow, i18n("Hide all posts from <b>@%1</b>?",
              username));
    if (res == KMessageBox::Cancel) {
        return;
    } else if (res == KMessageBox::Yes) {
        Filter *fil = new Filter(username, Filter::AuthorUsername, Filter::ExactMatch);
        fil->writeConfig();
        QList<Filter *> filterList = FilterSettings::self()->filters();
        filterList.append(fil);
        FilterSettings::self()->setFilters(filterList);
        Choqok::UI::TimelineWidget *tm = wd->timelineWidget();
        if (tm) {
//            qDebug() << "Closing all posts";
            for (Choqok::UI::PostWidget *pw: tm->postWidgets()) {
                if (pw->currentPost()->author.userName == username) {
                    pw->close();
                }
            }
        } else {
            wd->close();
        }
    } else {
        wd->close();
    }
}

#include "filtermanager.moc"
#include "moc_filtermanager.cpp"
