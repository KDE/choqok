//
// C++ Implementation: systrayicon
//
// Description: 
//
//
// Author:  Mehrdad Momeny <mehrdad.momeny@gmail.com>, (C) 2008
//
// Copyright: See COPYING file that comes with this distribution
//
//
#include "systrayicon.h"
#include "settings.h"
#include <kaction.h>
#include <kactioncollection.h>
#include <QMenu>
#include <QPainter>
#include <KColorScheme>
#include <QProcess>

SysTrayIcon::SysTrayIcon(QWidget* parent): KSystemTrayIcon(parent)
{
	kDebug();
	setToolTip(i18n("choqoK Twitter - Hit me to update your status"));
	
	mainWin = new MainWindow;
	
	m_defaultIcon = KIcon("choqok").pixmap(22);
	
	setIcon(m_defaultIcon);
	
	if(Settings::showMainWinOnStart()){
		mainWin->show();
	}
	
	quickWidget = new QuickTwit(mainWin);
	setupActions();
	
	connect(this, SIGNAL(activated( QSystemTrayIcon::ActivationReason )),
			 this, SLOT(sysTrayActivated(QSystemTrayIcon::ActivationReason)));
	
	connect(quickWidget, SIGNAL(sigStatusUpdated()), mainWin, SLOT(updateHomeTimeLine()));
	
	connect(mainWin, SIGNAL(sigSetUnread(int)), this, SLOT(slotSetUnread(int)));
	
	connect(mainWin, SIGNAL(sigNotify(const QString&, const QString&, const QString&)), 
			this, SLOT(systemNotify(const QString&, const QString&,const QString&)));
	
	connect(quickWidget, SIGNAL(sigNotify(const QString&,const  QString&,const  QString&)), 
			this, SLOT(systemNotify(const QString&, const QString&, const QString&)));
}

SysTrayIcon::~SysTrayIcon()
{
}

void SysTrayIcon::setupActions()
{

	QAction *newTwit = mainWin->actionCollection()->action("choqok_new_twit");
	connect(newTwit, SIGNAL(triggered( bool )), this, SLOT(postQuickTwit()));
	this->contextMenu()->addAction(newTwit);
	
	contextMenu()->addAction(mainWin->actionCollection()->action("update_timeline"));
	contextMenu()->addSeparator();
	
	KAction *showMain = new KAction(this);
	if(mainWin->isVisible())
		showMain->setText(i18n("Minimize"));
	else
		showMain->setText(i18n("Restore"));
	connect(showMain, SIGNAL(triggered( bool )), this, SLOT(toggleMainWindowVisibility()));
	actionCollection()->addAction("toggle-mainwin", showMain);
	contextMenu()->addAction(showMain);
}

void SysTrayIcon::quitApp()
{
	kDebug();
	qApp->quit();
}

void SysTrayIcon::postQuickTwit()
{
	if(quickWidget->isVisible()){
		quickWidget->hide();
	}
	else{
		quickWidget->show();
	}
}

void SysTrayIcon::toggleMainWindowVisibility()
{
	if(mainWin->isVisible()){
		mainWin->setUnreadStatusesToReadState();
		mainWin->hide();
		actionCollection()->action("toggle-mainwin")->setText(i18n("&Restore"));
	} else {
		mainWin->show();
		actionCollection()->action("toggle-mainwin")->setText(i18n("&Minimize"));
	}
}

void SysTrayIcon::sysTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
	if(reason == QSystemTrayIcon::Trigger){
		switch(Settings::systrayTriggerType()){
			case 0:
				toggleMainWindowVisibility();
				break;
			case 1:
				postQuickTwit();
				break;
			case 2:
				mainWin->updateTimeLines();
		};
	}
}

void SysTrayIcon::slotSetUnread(int unread)
{
	kDebug();
	if (unread == m_unread)
		return;

	m_unread=unread;

	this->setToolTip( i18np("choqoK - 1 unread status", "choqoK - %1 unread statuses", unread > 0 ? unread : 0));

	if (unread <= 0)
	{
		setIcon(m_defaultIcon);
	}
	else
	{
        // adapted from KMSystemTray::updateCount()
		int oldWidth = m_defaultIcon.size().width();

		if ( oldWidth == 0 )
			return;

		QString countStr = QString::number( unread );
		QFont f = KGlobalSettings::generalFont();
		f.setBold(true);

		float pointSize = f.pointSizeF();
		QFontMetrics fm(f);
		int w = fm.width(countStr);
		if( w > (oldWidth - 2) )
		{
			pointSize *= float(oldWidth - 2) / float(w);
			f.setPointSizeF(pointSize);
		}

        // overlay
		QImage overlayImg = m_defaultIcon.toImage().copy();
		QPainter p(&overlayImg);
		p.setFont(f);
		KColorScheme scheme(QPalette::Active, KColorScheme::View);

		fm = QFontMetrics(f);
		QRect boundingRect = fm.tightBoundingRect(countStr);
		boundingRect.adjust(0, 0, 0, 2);
		boundingRect.setHeight(qMin(boundingRect.height(), oldWidth));
		boundingRect.moveTo((oldWidth - boundingRect.width()) / 2,
							 ((oldWidth - boundingRect.height()) / 2) - 1);
		p.setOpacity(0.7);
		p.setBrush(scheme.background(KColorScheme::LinkBackground));
		p.setPen(scheme.background(KColorScheme::LinkBackground).color());
		p.drawRoundedRect(boundingRect, 2.0, 2.0);

		p.setBrush(Qt::NoBrush);
		p.setPen(scheme.foreground(KColorScheme::LinkText).color());
		p.setOpacity(1.0);
		p.drawText(overlayImg.rect(), Qt::AlignCenter, countStr);

		setIcon(QPixmap::fromImage(overlayImg));
	}
}

void SysTrayIcon::systemNotify(const QString &title, const QString &message, const QString &iconUrl)
{
	switch(Settings::notifyType()){
		case 0:
			break;
		case 1://KNotify
			break;
		case 2://Libnotify!
			QString libnotifyCmd = QString("notify-send -t ") + QString::number(Settings::notifyInterval()*1000) + QString(" -u low -i "+ iconUrl +" \"") + title + QString("\" \"") + message + QString("\"");
			QProcess::execute(libnotifyCmd);
			break;
	}
}

#include "systrayicon.moc"
