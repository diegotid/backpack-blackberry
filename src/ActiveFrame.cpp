/*
 * ActiveFrame.cpp
 *
 *  Created on: 25/03/2013
 *      Author: Diego
 */

#include "ActiveFrame.hpp"
#include "Backpack.hpp"

#include <bb/cascades/Label>
#include <bb/cascades/ImageView>

using namespace std;
using namespace bb::cascades;

#define INTERVAL 15

ActiveFrame::ActiveFrame(QObject *parent) : SceneCover(parent) {

	qmlCover = QmlDocument::create("asset:///ActiveFrame.qml").parent(this);
	coverContainer = qmlCover->createRootObject<Container>();
	setContent(coverContainer);

	QDir homeDir = QDir::home();
	dbFile.setFileName(homeDir.absoluteFilePath("backpack.db"));
	dbFile.open(QIODevice::ReadOnly);
	sql = new SqlDataAccess(dbFile.fileName(), "ActiveFrame", this);

	bookmarks = QVariantList();

	updateTimer = new QTimer();

	updateTimer->setSingleShot(true);
	updateTimer->setInterval(INTERVAL * 1000);
	QObject::connect(updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}

ActiveFrame::~ActiveFrame() {

	sql->connection().close();
	dbFile.close();
}

void ActiveFrame::takeFigures(QObject *parent) {

	if (bookmarks.size() == 0) {
		coverContainer->findChild<Label*>("oldest")->setText("Nothing in");
		coverContainer->findChild<Label*>("quickest")->setText("your backpack");
		return;
	}

	QDate now = QDate::currentDate();
	QDate oldest = ((Backpack*)parent)->getOldestDate();
	int ago = oldest.daysTo(now);
	QString agoText;
	if (ago == 0)
		agoText = "Today";
	else if (ago == 1)
		agoText = "Yesterday";
	else if (ago < 7)
		agoText = QString::number(ago).append(" days ago");
	else if (ago < 30) {
		int weeks = (ago - ago % 7) / 7;
		agoText = QString::number(weeks).append(" week").append(weeks > 1 ? "s " : " ").append("ago");
	} else {
		int months = (ago - ago % 30) / 30;
		agoText = QString::number(months).append(" month").append(months > 1 ? "s " : " ").append("ago");
	}
	coverContainer->findChild<Label*>("oldest")->setText("Oldest: " + agoText);

	int size = ((Backpack*)parent)->getQuickestSize();
	int mins = (size - size % 10000) / 10000;
	QString sizeText = mins < 1 ? "< 1 min" : QString::number(mins).append(" min.");
	coverContainer->findChild<Label*>("quickest")->setText("Quickest: " + sizeText);
}

void ActiveFrame::update() {

	update(false);
}

void ActiveFrame::update(bool force) {

	updateTimer->start();

	if (force || bookmarks.isEmpty()) {
		bookmarks.clear();
		bookmarks << sql->execute("SELECT * FROM Bookmark").toList();
	}

	if (bookmarks.size() == 0) {
		coverContainer->findChild<Label*>("memo")->setText("");
		coverContainer->findChild<Label*>("bookmarkURL")->setText("");
		coverContainer->findChild<Label*>("bookmarkTitle")->setText("");
		coverContainer->findChild<QObject*>("bookmarkPic")->setProperty("imageSource", "");
		coverContainer->findChild<QObject*>("bookmarkIcon")->setProperty("imageSource", "");
		return;
	}

	srand((unsigned)time(0));
	int i = rand() % bookmarks.size();

	QVariantMap item = bookmarks.value(i).toMap();

	coverContainer->findChild<Label*>("memo")->setText(item.value("memo").toString());
	coverContainer->findChild<Label*>("bookmarkURL")->setText(item.value("url").toString());
	coverContainer->findChild<Label*>("bookmarkTitle")->setText(item.value("title").toString());
	coverContainer->findChild<ImageView*>("bookmarkPic")->setImageSource(item.value("image").toString());
	coverContainer->findChild<ImageView*>("bookmarkIcon")->setImageSource(item.value("favicon").toString());
}
