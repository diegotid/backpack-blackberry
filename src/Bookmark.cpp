
#include "Bookmark.hpp"
#include "Backpack.hpp"

#include <bb/cascades/WebPage>
#include <bb/cascades/WebSettings>

#define START_DOWNLOAD_TIMEOUT 5000

Bookmark::Bookmark(QUrl url, SqlDataAccess *data, QObject *parent) : QObject(parent) {

	this->url = url;
	this->data = data;

	QVariantMap id = data->execute("SELECT id, title, memo, keep FROM Bookmark WHERE url = ?", QVariantList() << url.toString()).toList().value(0).toMap();

	if (!id.value("id").isNull()) {
		this->id = id.value("id").toInt();
		this->title = id.value("title").toString();
		this->memo = id.value("memo").toString();
		this->kept = id.value("keep").toBool();
		this->existing = true;
	} else {
		id = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
		this->id = id.value("MAX(id)").isNull() ? 1 : id.value("MAX(id)").toInt() + 1;
		this->kept = false;
		this->existing = false;
		data->execute("INSERT INTO Bookmark (id, url, keep, date, time) VALUES (?, ?, ?, ?, ?)", QVariantList() << this->id << url.toString() << false << QDate::currentDate() << QDateTime::currentDateTime());
	}

	network = new QNetworkAccessManager(this);
	connect(network, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleSize(QNetworkReply*)));

	page = new WebPage();
	titleComplete = false;
	faviconComplete = false;
	WebSettings *settings = page->settings();
	settings->setImageDownloadingEnabled(true);
	settings->setBinaryFontDownloadingEnabled(false);
	settings->setCookiesEnabled(false);
	settings->setJavaScriptEnabled(false);
	connect(page, SIGNAL(titleChanged(QString)), this, SLOT(handleTitle(QString)));
	connect(page, SIGNAL(iconChanged(QUrl)), this, SLOT(handleIcon(QUrl)));
	connect(page, SIGNAL(loadingChanged(bb::cascades::WebLoadRequest*)), this, SLOT(handleDownloadStatusChange(bb::cascades::WebLoadRequest*)));
}

void Bookmark::fetchContent() {

	bookmarkRequest = QNetworkRequest();
	bookmarkRequest.setUrl(this->url);
	network->get(bookmarkRequest);

	page->setUrl(url);
	titleComplete = false;
	faviconComplete = false;
}

void Bookmark::handleDownloadStatusChange(bb::cascades::WebLoadRequest *request) {

	if (request->status() == WebLoadStatus::Failed) {
		this->page->stop();
		((Backpack*)this->parent())->handleDownloadFailed(request->url());
	}
}

bool Bookmark::loading() {

	return page->loading();
}

QString Bookmark::getTitle() {

	if (titleComplete && this->title.isNull()) {
		QVariantList bookmarks = data->execute("SELECT title FROM Bookmark WHERE id = ?", QVariantList() << this->id).toList();
		if (bookmarks.length() > 0)
			this->title = bookmarks.value(0).toMap().value("title").toString();
	}
	return this->title;
}

QString Bookmark::getMemo() {

	return this->memo;
}

bool Bookmark::isKept() {

	return this->kept;
}

void Bookmark::setKept(bool keep) {

	data->execute(QString("UPDATE Bookmark SET keep = ? WHERE id = ?"), QVariantList() << keep << this->id);
}

bool Bookmark::alreadyExisted() {

	return this->existing;
}

void Bookmark::handleTitle(QString title) {

	if (title.length() == 0)
		return;

	this->title = title;
	data->execute("UPDATE Bookmark SET title = ? WHERE id = ?", QVariantList() << title << this->id);

	titleComplete = true;
	if (faviconComplete)
		this->page->stop();

	emit titleChanged(title);
}

void Bookmark::handleIcon(QUrl icon) {

	QString url = icon.toString();
	if (url.length() == 0)
		return;

	QString domain = url.left(url.indexOf(icon.topLevelDomain()));
	domain = domain.right(domain.length() - domain.indexOf(".") - 1);
	domain.append(icon.topLevelDomain());

	iconRequest = QNetworkRequest();
	iconRequest.setUrl(QString("http://www.google.com/s2/favicons?domain=").append(domain));
	network->get(iconRequest);
}

void Bookmark::handleSize(QNetworkReply *reply) {

	if (reply->url() != bookmarkRequest.url()) {
		downloadFavicon(reply);
		return;
	}

	QVariantList sizeValues;
	data->execute("UPDATE Bookmark SET size = ? WHERE id = ?", sizeValues << reply->size() << this->id);

	emit sizeChanged();
}

void Bookmark::downloadFavicon(QNetworkReply *reply) {

	QFile *iconFile = new QFile(QString("data/icon-") % QString::number(this->id) % QString(".png"));
	iconFile->remove();
	iconFile->open(QIODevice::ReadWrite);
	iconFile->write(reply->readAll());
	iconFile->flush();
	iconFile->close();

	QFileInfo iconInfo(*iconFile);
	data->execute("UPDATE Bookmark SET favicon = ? WHERE id = ?", QVariantList() << QString("file://").append(iconInfo.absoluteFilePath()) << this->id);

	faviconComplete = true;
	if (titleComplete) {
		this->page->stop();
	}

	emit iconChanged();
}

void Bookmark::saveMemo(QString memo) {

	if (memo.length() > 0)
		data->execute(QString("UPDATE Bookmark SET memo = ? WHERE id = ?"), QVariantList() << memo << this->id);
}

void Bookmark::remove() {

	data->execute("DELETE FROM Bookmark WHERE id = ?", QVariantList() << this->id);

	QFile icon;
	icon.setFileName(QDir::home().absoluteFilePath(QString("icon-") % QString::number(id) % QString(".png")));
	icon.remove();
}

Bookmark::~Bookmark() {
}
