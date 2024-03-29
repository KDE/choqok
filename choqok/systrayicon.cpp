/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2008-2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "systrayicon.h"

#include <QFontDatabase>
#include <QPainter>
#include <QTimer>

#include <KColorScheme>
#include <KLocalizedString>

#include "choqokdebug.h"

SysTrayIcon::SysTrayIcon(Choqok::UI::MainWindow *parent)
    : KStatusNotifierItem(parent), _mainwin(parent), isOffline(false)
{
    qCDebug(CHOQOK);
    unread = 0;
    setAssociatedWidget(parent);
    setCategory(ApplicationStatus);
    setStandardActionsEnabled(false);
//     setStatus(Active);
    setIconByName(currentIconName());
}

SysTrayIcon::~SysTrayIcon()
{
    qCDebug(CHOQOK);
}

void SysTrayIcon::resetUnreadCount()
{
    updateUnreadCount(-unread);
}

QString SysTrayIcon::currentIconName()
{
    if (isOffline) {
        return QLatin1String("choqok_offline");
    } else {
        return QLatin1String("choqok");
    }
}

void SysTrayIcon::updateUnreadCount(int changeOfUnreadPosts)
{
    qCDebug(CHOQOK);
    unread += changeOfUnreadPosts;

    if (unread <= 0) {
        setIconByName(currentIconName());
        unread = 0;
        setStatus(Passive);
    } else {
        setStatus(Active);
        int oldWidth = 22;

        QString countStr = QString::number(unread);
        QFont f = QFontDatabase::systemFont(QFontDatabase::GeneralFont);
        f.setBold(true);

        auto pointSize = f.pointSizeF();
        QFontMetrics fm(f);
        int w = fm.horizontalAdvance(countStr);
        if (w > (oldWidth - 2)) {
            pointSize *= float(oldWidth - 2) / float(w);
            f.setPointSizeF(pointSize);
        }

        // overlay
        QPixmap overlayImg = QIcon::fromTheme(currentIconName()).pixmap(22, 22);
        QPainter p(&overlayImg);
        p.setFont(f);

        fm = QFontMetrics(f);
        QRect boundingRect = fm.tightBoundingRect(countStr);
        boundingRect.adjust(0, 0, 0, 2);
        boundingRect.setHeight(qMin(boundingRect.height(), oldWidth));
        boundingRect.moveTo((oldWidth - boundingRect.width()) / 2,
                            ((oldWidth - boundingRect.height()) / 2) - 1);
        p.setOpacity(0.7);
        QBrush br(QColor(255, 255, 255), Qt::SolidPattern);
        p.setBrush(br);
        p.setPen(QColor(255, 255, 255));
        p.drawRoundedRect(boundingRect, 2.0, 2.0);

        p.setBrush(Qt::NoBrush);
        p.setPen(QColor(0, 0, 0));
        p.setOpacity(1.0);
        p.drawText(overlayImg.rect(), Qt::AlignCenter, countStr);
        setIconByPixmap(overlayImg);
    }
    this->setToolTip(QLatin1String("choqok"), i18n("Choqok"), i18np("1 unread post", "%1 unread posts", unread));
}

void SysTrayIcon::setTimeLineUpdatesEnabled(bool isEnabled)
{
    if (isEnabled) {
        setToolTip(QLatin1String("choqok"), i18n("Choqok"), QString());
        setIconByName(QLatin1String("choqok"));
    } else {
        setToolTip(QLatin1String("choqok"), i18n("Choqok - Disabled"), QString());
        setIconByName(QLatin1String("choqok_offline"));
    }
    isOffline = !isEnabled;
    updateUnreadCount(0);
}

void SysTrayIcon::slotJobDone(Choqok::JobResult result)
{
    qCDebug(CHOQOK);
    if (result == Choqok::Success) {
        setOverlayIconByName(QLatin1String("task-complete"));
    } else {
        setOverlayIconByName(QLatin1String("task-reject"));
    }
    QTimer::singleShot(5000, this, &SysTrayIcon::slotRestoreIcon);
}

void SysTrayIcon::slotRestoreIcon()
{
    setOverlayIconByName(QString());
    updateUnreadCount(0);
}

int SysTrayIcon::unreadCount() const
{
    return unread;
}

#include "moc_systrayicon.cpp"
