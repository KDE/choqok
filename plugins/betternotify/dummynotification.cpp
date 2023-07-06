/*
    This file is part of Choqok, the KDE micro-blogging client

    SPDX-FileCopyrightText: 2012 Mehrdad Momeny <mehrdad.momeny@gmail.com>

    SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only OR LicenseRef-KDE-Accepted-GPL
*/

#include "dummynotification.h"

#include <QIcon>
#include <QMouseEvent>

#include <KLocalizedString>

#include "choqokappearancesettings.h"
#include "choqoktools.h"
#include "notifysettings.h"
#include "postwidget.h"

DummyNotification::DummyNotification(const QFont &font, const QColor &color, const QColor &background, QWidget *parent)
    : QTextBrowser(parent), isMoving(false)
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setWindowOpacity(0.8);
    setWindowFlags(Qt::ToolTip);
    setFrameShape(QFrame::NoFrame);
    setOpenExternalLinks(false);
    setOpenLinks(false);
    setTextInteractionFlags(Qt::LinksAccessibleByMouse);
    document()->addResource(QTextDocument::ImageResource, QUrl(QLatin1String("img://profileImage")), QIcon::fromTheme(QLatin1String("choqok")).pixmap(48));
    document()->addResource(QTextDocument::ImageResource, QUrl(QLatin1String("icon://close")),
                            QIcon::fromTheme(QLatin1String("dialog-ok")).pixmap(16));
    setText(baseText.arg(i18n("Choqok")).arg(i18n("KDE Rocks! :)")).arg(i18n("OK")));
    connect(this, &DummyNotification::anchorClicked, this, &DummyNotification::slotProcessAnchor);

    QString fntStr = QLatin1String("font-family:\"") + font.family() + QLatin1String("\"; font-size:") + QString::number(font.pointSize()) + QLatin1String("pt;");
    fntStr += (font.bold() ? QLatin1String(" font-weight:bold;") : QString()) + (font.italic() ? QLatin1String(" font-style:italic;") : QString());
    QString style = Choqok::UI::PostWidget::getBaseStyle().arg(Choqok::getColorString(color), Choqok::getColorString(background), fntStr);

    setStyleSheet(style);
}

DummyNotification::~DummyNotification()
{
    disconnect(this, &DummyNotification::anchorClicked, this, &DummyNotification::slotProcessAnchor);
}

void DummyNotification::mouseMoveEvent(QMouseEvent *ev)
{
    QTextBrowser::mouseMoveEvent(ev);
    if (isMoving) {
        QPoint diff = ev->globalPos() - lastPressedPosition;
        lastPressedPosition = ev->globalPos();
        QPoint newPos = pos() + diff;
        move(newPos);
    }
}

void DummyNotification::mousePressEvent(QMouseEvent *ev)
{
    QTextBrowser::mousePressEvent(ev);
    isMoving = true;
    lastPressedPosition = ev->globalPos();
}

void DummyNotification::mouseReleaseEvent(QMouseEvent *ev)
{
    QTextBrowser::mouseReleaseEvent(ev);
    isMoving = false;
}

void DummyNotification::slotProcessAnchor(const QUrl &url)
{
    if (url.scheme() == QLatin1String("choqok")) {
        if (url.host() == QLatin1String("close")) {
            Q_EMIT positionSelected(pos());
        }
    }
}

#include "moc_dummynotification.cpp"
