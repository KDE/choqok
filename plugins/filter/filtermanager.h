/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2010-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef FILTERMANAGER_H
#define FILTERMANAGER_H

#include <QPointer>
#include <QQueue>

#include "plugin.h"

#include "filter.h"

class QAction;
namespace Choqok
{
namespace UI
{
class PostWidget;
}
}

/**
Filter Manager

@author Mehrdad Momeny \<mehrdad.momeny@gmail.com\>
*/
class FilterManager : public Choqok::Plugin
{
    Q_OBJECT
public:
    FilterManager(QObject *parent, const QList< QVariant > &args);
    ~FilterManager();

protected Q_SLOTS:
    void slotAddNewPostWidget(Choqok::UI::PostWidget *newWidget);
    void startParsing();
    void slotConfigureFilters();
    void slotHidePost();

private:
    enum ParserState { Stopped = 0, Running };
    ParserState state;

    Filter::FilterAction filterText(const QString &textToCheck, Filter *filter);
    void doFiltering(Choqok::UI::PostWidget *postToFilter, Filter::FilterAction action);

    void parse(Choqok::UI::PostWidget *postToParse);
    QQueue< QPointer<Choqok::UI::PostWidget> > postsQueue;

    bool parseSpecialRules(Choqok::UI::PostWidget *postToParse);

    QAction *hidePost;
};

#endif
