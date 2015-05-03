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

ActiveFrame::ActiveFrame(QObject *parent) : SceneCover(parent) {

	qmlCover = QmlDocument::create("asset:///ActiveFrame.qml").parent(this);
	coverContainer = qmlCover->createRootObject<Container>();
	setContent(coverContainer);

	QDir homeDir = QDir::home();
	dbFile.setFileName(homeDir.absoluteFilePath("backpack.db"));
	dbFile.open(QIODevice::ReadOnly);
	sql = new SqlDataAccess(dbFile.fileName(), "ActiveFrame", this);

	bookmarks = QVariantList();
	previous = QVariantMap();
}

ActiveFrame::~ActiveFrame() {

	sql->connection().close();
	dbFile.close();
}

QUrl ActiveFrame::update(bool force) {

	if (force || bookmarks.isEmpty()) {
		bookmarks.clear();
		bookmarks << sql->execute("SELECT * FROM Bookmark").toList();
	}

	if (bookmarks.size() == 0) {
		coverContainer->findChild<Label*>("bookmarkURL")->setText("");
		coverContainer->findChild<Label*>("bookmarkTitle")->setText("Nothing in your backpack");
		coverContainer->findChild<QObject*>("bookmarkPic")->setProperty("imageSource", "");
		coverContainer->findChild<QObject*>("bookmarkIcon")->setProperty("imageSource", "");
		return QUrl();
	}

    srand((unsigned)time(0));
    int i = rand() % bookmarks.size();
    QVariantMap item = bookmarks.value(i).toMap();

    if (previous.isEmpty()) {
        previous = item;
        return update(force);
    }

    if (previous.value("image").toString().length() == 0 || previous.value("favicon").toString().length() == 0) {
        previous = sql->execute("SELECT * FROM Bookmark WHERE url = ?", QVariantList() << previous.value("url").toString()).toList().value(0).toMap();
    }

	coverContainer->findChild<Label*>("bookmarkURL")->setText(previous.value("url").toString());
	coverContainer->findChild<Label*>("bookmarkTitle")->setText(previous.value("title").toString());
	coverContainer->findChild<ImageView*>("bookmarkPic")->setImageSource(previous.value("image").toString());
	coverContainer->findChild<ImageView*>("bookmarkIcon")->setImageSource(previous.value("favicon").toString());

	previous = item;

	if (item.value("image").toString().length() == 0 || item.value("favicon").toString().length() == 0) {
	    return QUrl(item.value("url").toString());
	} else {
        return QUrl();
	}
}
