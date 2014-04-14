
#include "Bookmark.hpp"
#include "Backpack.hpp"

#include <bb/cascades/WebPage>
#include <bb/cascades/WebSettings>

#define START_DOWNLOAD_TIMEOUT 5000

Bookmark::Bookmark(QUrl url, SqlDataAccess *data, QObject *parent) : QObject(parent) {

	this->url = url;
	this->data = data;

	QVariantMap id = data->execute("SELECT id, pocket_id, title, memo, keep, image, favicon FROM Bookmark WHERE url = ?", QVariantList() << url.toString()).toList().value(0).toMap();

	if (!id.value("id").isNull()) {
		this->id = id.value("id").toInt();
		this->pocketId = id.value("pocket_id").toLongLong();
		this->title = id.value("title").toString();
		this->memo = id.value("memo").toString();
		this->kept = id.value("keep").toBool();
		this->image = id.value("image").toString();
		this->favicon = id.value("favicon").toString();
		this->existing = true;
	} else {
		id = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
		this->id = id.value("MAX(id)").isNull() ? 1 : id.value("MAX(id)").toInt() + 1;
		this->kept = false;
		this->existing = false;
		data->execute("INSERT INTO Bookmark (id, url, keep, time) VALUES (?, ?, ?, ?)", QVariantList() << this->id << url.toString() << false << QDateTime::currentDateTime());
	}

	network = new QNetworkAccessManager(this);
	connect(network, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleContent(QNetworkReply*)));

	page = new WebPage();

	WebSettings *settings = page->settings();
	settings->setImageDownloadingEnabled(true);
	settings->setBinaryFontDownloadingEnabled(false);
	settings->setCookiesEnabled(false);
	settings->setJavaScriptEnabled(false);
	connect(page, SIGNAL(titleChanged(QString)), this, SLOT(handleTitle(QString)));
	connect(page, SIGNAL(iconChanged(QUrl)), this, SLOT(handleIcon(QUrl)));
	connect(page, SIGNAL(loadingChanged(bb::cascades::WebLoadRequest*)), this, SLOT(handleDownloadStatusChange(bb::cascades::WebLoadRequest*)));
}

void Bookmark::pocketSync(qlonglong id) {

	data->execute("UPDATE Bookmark SET pocket_id = ? WHERE url = ?", QVariantList() << id << this->url.toString());
	this->pocketId = id;
}

void Bookmark::pocketSync(qlonglong id, QDateTime added) {

	data->execute("UPDATE Bookmark SET pocket_id = ?, time = ? WHERE url = ?", QVariantList() << id << added << this->url.toString());
	this->pocketId = id;
}

qlonglong Bookmark::getPocketId() {

	return this->pocketId;
}

void Bookmark::fetchContent() {

	bookmarkRequest = QNetworkRequest();
	bookmarkRequest.setUrl(this->url);
	network->get(bookmarkRequest);

	page->setUrl(this->url);
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

	if (this->title.isNull()) {
		QVariantList bookmarks = data->execute("SELECT title FROM Bookmark WHERE id = ?", QVariantList() << this->id).toList();
		if (bookmarks.length() > 0)
			this->title = bookmarks.value(0).toMap().value("title").toString();
	}
	return this->title;
}

QString Bookmark::getMemo() {

	return this->memo;
}

QUrl Bookmark::getUrl() {

	return this->url;
}

bool Bookmark::isKept() {

	return this->kept;
}

void Bookmark::setKept(bool keep) {

	data->execute(QString("UPDATE Bookmark SET keep = ? WHERE id = ?"), QVariantList() << keep << this->id);
	this->kept = keep;
}

bool Bookmark::alreadyExisted() {

	return this->existing;
}

void Bookmark::handleTitle(QString title) {

	if (title.length() == 0)
		return;

	this->title = title;
	data->execute("UPDATE Bookmark SET title = ? WHERE id = ?", QVariantList() << title << this->id);

	emit titleChanged(title);
}

void Bookmark::handleIcon(QUrl icon) {

	QString iconUrl = icon.toString();
	if (iconUrl.length() == 0)
		return;

	QString domain = iconUrl.left(iconUrl.indexOf(icon.topLevelDomain()));
	domain = domain.right(domain.length() - domain.indexOf(".") - 1);
	domain.append(icon.topLevelDomain());

	iconRequest = QNetworkRequest();
	iconRequest.setUrl(QString("http://www.google.com/s2/favicons?domain=").append(domain));
	network->get(iconRequest);
}

void Bookmark::handleContent(QNetworkReply *reply) {

	if (reply->url() == iconRequest.url()) {
		downloadFavicon(reply);
		return;
	} else if (reply->url() == imageRequest.url()) {
		downloadImage(reply);
		return;
	}

	QByteArray htmlSource = reply->readAll();

	if (htmlSource.indexOf("og:image") > 0) {
		QByteArray ogpImage = htmlSource.right(htmlSource.length() - htmlSource.toLower().indexOf("og:image"));
		ogpImage = ogpImage.right(ogpImage.length() - ogpImage.toLower().indexOf("http"));
		ogpImage = ogpImage.left(ogpImage.indexOf('"'));

		if (ogpImage.length() > 0) {
			imageRequest = QNetworkRequest();
			imageRequest.setUrl(QUrl(ogpImage));
			network->get(imageRequest);
		}
	}

	int type = Backpack::ALL;

	if (htmlSource.indexOf("og:image") > 0) {
		QByteArray ogpType = htmlSource.right(htmlSource.length() - htmlSource.toLower().indexOf("og:type"));
		ogpType = ogpType.right(ogpType.length() - ogpType.toLower().indexOf("content"));
		ogpType = ogpType.right(ogpType.length() - ogpType.indexOf('"') - 1);
		ogpType = ogpType.left(ogpType.indexOf('"')).toLower();

		if (ogpType.length() > 0) {
			if (ogpType.indexOf("article") >= 0)
				type = Backpack::ARTICLES;
			else if (ogpType.indexOf("video") >= 0)
				type = Backpack::VIDEOS;
			else if (ogpType.indexOf("image") >= 0
					|| ogpType.indexOf("photo") >= 0
					|| ogpType.indexOf("picture") >= 0)
				type = Backpack::IMAGES;
		}
	}

	if (type == Backpack::ALL)
		data->execute("UPDATE Bookmark SET size = ? WHERE id = ?", QVariantList() << htmlSource.size() << this->id);
	else
		data->execute("UPDATE Bookmark SET size = ?, type = ? WHERE id = ?", QVariantList() << htmlSource.size() << type << this->id);

	emit sizeChanged();
}

void Bookmark::downloadFavicon(QNetworkReply *reply) {

	QFile *iconFile = new QFile(QDir::home().absoluteFilePath(QString("icon.") % this->url.host() % QString(".png")));
	iconFile->remove();
	iconFile->open(QIODevice::ReadWrite);
	iconFile->write(reply->readAll());
	iconFile->flush();
	iconFile->close();

	QFileInfo iconInfo(*iconFile);
	this->favicon = iconInfo.absoluteFilePath();
	data->execute("UPDATE Bookmark SET favicon = ? WHERE id = ?", QVariantList() << this->favicon << this->id);

	emit faviconChanged(QUrl(this->favicon));
}

void Bookmark::downloadImage(QNetworkReply *reply) {

	QString extension = reply->url().toString();
	extension = extension.right(extension.length() - extension.lastIndexOf("."));
	if (extension.indexOf('?') > 0)
		extension = extension.left(extension.indexOf('?'));
	QFile *imageFile = new QFile(QDir::home().absoluteFilePath(QString("img.") % QString::number(this->id) % extension));
	imageFile->remove();
	imageFile->open(QIODevice::ReadWrite);
	imageFile->write(reply->readAll());
	imageFile->flush();
	imageFile->close();

	QFileInfo imageInfo(*imageFile);
	this->image = imageInfo.absoluteFilePath();
	data->execute("UPDATE Bookmark SET image = ? WHERE id = ?", QVariantList() << this->image << this->id);

	emit imageChanged(QUrl(this->image));
}

void Bookmark::saveMemo(QString memo) {

	if (memo.length() > 0)
		data->execute(QString("UPDATE Bookmark SET memo = ? WHERE id = ?"), QVariantList() << memo << this->id);

	this->memo = memo;
}

void Bookmark::remove() {

	data->execute("DELETE FROM Bookmark WHERE id = ?", QVariantList() << this->id);

	QFile *iconFile = new QFile(this->favicon);
	iconFile->remove();

	QFile *imageFile = new QFile(this->image);
	imageFile->remove();
}

Bookmark::~Bookmark() {
}
