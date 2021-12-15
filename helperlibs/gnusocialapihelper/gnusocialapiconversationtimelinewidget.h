/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2011-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#ifndef GNUSOCIALAPICONVERSATIONTIMELINEWIDGET_H
#define GNUSOCIALAPICONVERSATIONTIMELINEWIDGET_H

#include "twitterapitimelinewidget.h"

class CHOQOK_HELPER_EXPORT GNUSocialApiConversationTimelineWidget : public TwitterApiTimelineWidget
{
    Q_OBJECT
public:
    GNUSocialApiConversationTimelineWidget(Choqok::Account *currentAccount, const QString &conversationId, QWidget *parent = nullptr);
    ~GNUSocialApiConversationTimelineWidget();

protected:
    virtual void saveTimeline() override;
    virtual void loadTimeline() override;

    QString conversationId;

protected Q_SLOTS:
    void slotConversationFetched(Choqok::Account *theAccount, const QString &conversationId,
                                 QList<Choqok::Post *> posts);
    void updateHeight();
};

#endif // GNUSOCIALAPICONVERSATIONTIMELINEWIDGET_H
