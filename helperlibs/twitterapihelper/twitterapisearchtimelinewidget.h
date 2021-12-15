/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef TWITTERAPISEARCHTIMELINEWIDGET_H
#define TWITTERAPISEARCHTIMELINEWIDGET_H

#include "timelinewidget.h"
#include "twitterapisearch.h"

class CHOQOK_HELPER_EXPORT TwitterApiSearchTimelineWidget : public Choqok::UI::TimelineWidget
{
    Q_OBJECT
public:
    TwitterApiSearchTimelineWidget(Choqok::Account *account, const QString &timelineName,
                                   const SearchInfo &info, QWidget *parent = nullptr);
    ~TwitterApiSearchTimelineWidget();
    virtual void addNewPosts(QList< Choqok::Post * > &postList) override;
    void removeAllPosts();
    SearchInfo searchInfo() const;

Q_SIGNALS:
    void closeMe();

protected Q_SLOTS:
    virtual void saveTimeline() override;
    virtual void loadTimeline() override;
    void slotUpdateSearchResults();

    void reloadList();
    void loadNextPage();
    void loadPreviousPage();
    void loadCustomPage();

private:
    void addFooter();
    void loadCustomPage(const QString &);

    class Private;
    Private *const d;
};

#endif // TWITTERAPISEARCHTIMELINEWIDGET_H
