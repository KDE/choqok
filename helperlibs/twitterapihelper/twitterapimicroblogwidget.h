/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPIMICROBLOGWIDGET_H
#define TWITTERAPIMICROBLOGWIDGET_H

#include <QMap>
#include <QPoint>

#include "twitterapihelper_export.h"

#include "microblogwidget.h"
#include "timelinewidget.h"
#include "twitterapisearch.h"

class TwitterApiSearchTimelineWidget;

class TWITTERAPIHELPER_EXPORT TwitterApiMicroBlogWidget : public Choqok::UI::MicroBlogWidget
{
    Q_OBJECT
public:
    explicit TwitterApiMicroBlogWidget(Choqok::Account *account, QWidget *parent = nullptr);
    ~TwitterApiMicroBlogWidget();
    virtual void initUi() override;

public Q_SLOTS:
    void slotContextMenu(QWidget *w, const QPoint &pt);

protected Q_SLOTS:
    void closeAllSearches();
    void slotAccountModified(Choqok::Account *account);
    void slotCloseCurrentSearch();
    virtual void saveSearchTimelinesState();
    virtual void loadSearchTimelinesState();
    virtual void slotSearchResultsReceived(const SearchInfo &info,
                                           QList<Choqok::Post *> &postsList);

protected:
    void closeSearch(Choqok::UI::TimelineWidget *searchWidget);
    QMap<QString, TwitterApiSearchTimelineWidget *> mSearchTimelines;
    TwitterApiSearchTimelineWidget *addSearchTimelineWidgetToUi(const QString &name,
            const SearchInfo &info);
private:
    class Private;
    Private *const d;
};

#endif // TWITTERAPIMICROBLOGWIDGET_H
