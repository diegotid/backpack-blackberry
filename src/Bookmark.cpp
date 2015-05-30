
#include "Bookmark.hpp"
#include "Backpack.hpp"

#include <bb/cascades/WebPage>
#include <bb/cascades/WebSettings>

Bookmark::Bookmark(QUrl url, SqlDataAccess *data, QObject *parent) : QObject(parent) {

    this->data = data;
	this->url = url;
	this->hashUrl = cleanUrlHash(this->url);

	QVariantList currentList = data->execute("SELECT pocket_id, title, memo, keep, image, favicon FROM Bookmark WHERE hash_url = ?", QVariantList() << this->hashUrl).toList();

	if (currentList.size() > 0) {
	    QVariantMap id = currentList.value(0).toMap();
		this->pocketId = id.value("pocket_id").toLongLong();
		this->title = id.value("title").toString();
		this->memo = id.value("memo").toString();
		this->kept = id.value("keep").toBool();
		this->image = id.value("image").toString();
		this->favicon = id.value("favicon").toString();
		this->existing = true;
	} else {
		this->kept = false;
		this->existing = false;
		data->execute("INSERT INTO Bookmark (url, hash_url, keep, time) VALUES (?, ?, ?, ?)", QVariantList() << url.toString() << this->hashUrl << false << QDateTime::currentDateTime());
	}

	initiate();
}

Bookmark::Bookmark(QUrl url, qlonglong pocketId, QString title, bool favorited, SqlDataAccess *data, QObject *parent) : QObject(parent) {

	this->data = data;
    this->url = url;
	this->hashUrl = cleanUrlHash(this->url);
	this->pocketId = pocketId;
	this->title = title;
	this->kept = favorited;
	this->existing = true;

	initiate();
}

void Bookmark::initiate() {

    network = new QNetworkAccessManager(this);
    bool net_fin = connect(network, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleContent(QNetworkReply*)));
    Q_ASSERT(net_fin);
    Q_UNUSED(net_fin);

    page = new WebPage();
    bool pag_tit = connect(page, SIGNAL(titleChanged(QString)), this, SLOT(handleTitle(QString)));
    Q_ASSERT(pag_tit);
    Q_UNUSED(pag_tit);

    fetchingContent = false;
    fetchingImage = false;
    fetchingIcon = false;
}

void Bookmark::pocketSync(qlonglong id) {

	data->execute("UPDATE Bookmark SET pocket_id = ? WHERE hash_url = ?", QVariantList() << id << this->hashUrl);
	this->pocketId = id;
}

void Bookmark::pocketSync(qlonglong id, QDateTime added) {

	data->execute("UPDATE Bookmark SET pocket_id = ?, time = ? WHERE hash_url = ?", QVariantList() << id << added << this->hashUrl);
	this->pocketId = id;
}

qlonglong Bookmark::getPocketId(SqlDataAccess *data, QUrl url) {

    QVariantList bookmarks = data->execute("SELECT pocket_id FROM Bookmark WHERE hash_url = ?", QVariantList() << cleanUrlHash(url)).toList();
    if (bookmarks.length() > 0)
        return bookmarks.value(0).toMap().value("pocket_id").toLongLong();
    else
        return NULL;
}

void Bookmark::fetchContent() {

    bookmarkRequest = QNetworkRequest();
    bookmarkRequest.setUrl(this->url);

    network->get(bookmarkRequest);

    fetchingContent = true;

    QString iconUrl = this->url.toString();
    QString domain = iconUrl.left(iconUrl.indexOf(this->url.topLevelDomain()));
    domain = domain.right(domain.length() - domain.indexOf(".") - 1);
    domain.append(this->url.topLevelDomain());

    iconRequest = QNetworkRequest();
    iconRequest.setUrl(QUrl(QString("http://www.google.com/s2/favicons?domain=").append(domain)));
    network->get(iconRequest);

    fetchingIcon = true;
}

void Bookmark::handleDownloadStatusChange(bb::cascades::WebLoadRequest *request) {

	if (request->status() == WebLoadStatus::Failed) {
		((Backpack*)this->parent())->handleDownloadFailed(request->url());
	}
}

bool Bookmark::loading() {

    return fetchingContent || fetchingImage || fetchingIcon;
}

QString Bookmark::getTitle() {

    if (this->title.isNull()) {
        QVariantList bookmarks = data->execute("SELECT title FROM Bookmark WHERE hash_url = ?", QVariantList() << this->hashUrl).toList();
        if (bookmarks.length() > 0)
            this->title = bookmarks.value(0).toMap().value("title").toString();
    }
    return this->title;
}

QString Bookmark::getImagePath() {

    if (this->image.isNull()) {
        QVariantList bookmarks = data->execute("SELECT image FROM Bookmark WHERE hash_url = ?", QVariantList() << this->hashUrl).toList();
        if (bookmarks.length() > 0)
            this->image = bookmarks.value(0).toMap().value("image").toString();
    }
    return this->image;
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

void Bookmark::setKept(SqlDataAccess *data, QUrl url, bool keep) {

	data->execute("UPDATE Bookmark SET keep = ? WHERE hash_url = ?", QVariantList() << keep << cleanUrlHash(url));
}

bool Bookmark::alreadyExisted() {

	return this->existing;
}

void Bookmark::handleTitle(QString title) {

    page->deleteLater();

	if (title.length() == 0) return;

	this->title = title;
	data->execute("UPDATE Bookmark SET title = ? WHERE hash_url = ?", QVariantList() << title << this->hashUrl);

	((Backpack*)this->parent())->updateTitle(this->url, title);

	fetchingContent = false;

    if (!fetchingImage && !fetchingIcon) {
        network->deleteLater();
        emit downloadComplete(this->hashUrl);
    }
}


void Bookmark::handleContent(QNetworkReply *reply) {

    if (reply->url().toString().length() == 0) {
        return;
    }

    QUrl redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirect.isValid()) {
        if (redirect.isRelative()) {
            redirect.setScheme(reply->url().scheme());
            redirect.setHost(reply->url().host());
        }
        if (reply->url() == iconRequest.url()) {
            iconRequest.setUrl(redirect);
            network->get(iconRequest);
        } else if (reply->url() == imageRequest.url()) {
            imageRequest.setUrl(redirect);
            network->get(imageRequest);
        } else {
            bookmarkRequest.setUrl(redirect);
            network->get(bookmarkRequest);
        }
        return;
    }

    if (reply->url() == iconRequest.url()) {
		downloadFavicon(reply);
		return;
	} else if (reply->url() == imageRequest.url()) {
		downloadImage(reply);
		return;
	}

	QByteArray htmlSource = reply->readAll();

	if (htmlSource.toLower().indexOf("og:image") > 0) {
		QByteArray ogpImage = htmlSource.right(htmlSource.length() - htmlSource.toLower().indexOf("og:image"));
		ogpImage = ogpImage.right(ogpImage.length() - ogpImage.toLower().indexOf("http"));
		ogpImage = ogpImage.left(ogpImage.indexOf('"'));

		if (ogpImage.length() > 0) {
			imageRequest = QNetworkRequest();
			imageRequest.setUrl(QUrl(ogpImage));
            network->get(imageRequest);
            fetchingImage = true;
		} else {
		    data->execute("UPDATE Bookmark SET image = '.' WHERE hash_url = ?", QVariantList() << this->hashUrl);
		}
    } else {
        data->execute("UPDATE Bookmark SET image = '.' WHERE hash_url = ?", QVariantList() << this->hashUrl);
	}

	int type = Backpack::ALL;

	if (htmlSource.toLower().indexOf("og:type") > 0) {
		QByteArray ogpType = htmlSource.right(htmlSource.length() - htmlSource.toLower().indexOf("og:type"));
		ogpType = ogpType.right(ogpType.length() - ogpType.toLower().indexOf("content"));
		ogpType = ogpType.right(ogpType.length() - ogpType.indexOf('"') - 1);
		ogpType = ogpType.left(ogpType.indexOf('"'));

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

	page->loadData(htmlSource, "text/html", reply->url());

	if (htmlSource.size() > 0) {
        if (type == Backpack::ALL)
            data->execute("UPDATE Bookmark SET size = ? WHERE hash_url = ?", QVariantList() << htmlSource.size() << this->hashUrl);
        else
            data->execute("UPDATE Bookmark SET size = ?, type = ? WHERE hash_url = ?", QVariantList() << htmlSource.size() << type << this->hashUrl);
	}

    ((Backpack*)this->parent())->handleBookmarkComplete(this->url, htmlSource.size());

    reply->deleteLater();
}

void Bookmark::downloadFavicon(QNetworkReply *reply) {

	QFile *iconFile = new QFile(QDir::home().absoluteFilePath(QString("icon.") % this->url.host() % QString(".png")));
	iconFile->remove();
	iconFile->open(QIODevice::ReadWrite);
	iconFile->write(reply->readAll());
	iconFile->flush();
	iconFile->close();
	iconFile->deleteLater();

	QFileInfo iconInfo(*iconFile);
	this->favicon = iconInfo.absoluteFilePath();
	data->execute("UPDATE Bookmark SET favicon = ? WHERE hash_url = ?", QVariantList() << this->favicon << this->hashUrl);

	((Backpack*)this->parent())->updateFavicon(this->url, QUrl(this->favicon));

	fetchingIcon = false;

    if (!fetchingContent && !fetchingImage) {
        network->deleteLater();
        emit downloadComplete(this->hashUrl);
    }
    reply->deleteLater();
}

void Bookmark::downloadImage(QNetworkReply *reply) {

    QString extension = reply->url().toString();
	extension = extension.right(extension.length() - extension.lastIndexOf("."));
	if (extension.indexOf('?') > 0)
		extension = extension.left(extension.indexOf('?'));
	QFile *imageFile = new QFile(QDir::home().absoluteFilePath(QString("img.") % QString::number(this->hashUrl) % extension));
	imageFile->remove();
	imageFile->open(QIODevice::ReadWrite);
	imageFile->write(reply->readAll());
	imageFile->flush();
	imageFile->close();
	imageFile->deleteLater();

	QFileInfo imageInfo(*imageFile);
	this->image = imageInfo.absoluteFilePath();
	data->execute("UPDATE Bookmark SET image = ? WHERE hash_url = ?", QVariantList() << this->image << this->hashUrl);

	((Backpack*)this->parent())->updateImage(this->url, QUrl(this->image));

    fetchingImage = false;

    if (!fetchingContent && !fetchingIcon) {
        network->deleteLater();
        emit downloadComplete(this->hashUrl);
    }
    reply->deleteLater();
}

void Bookmark::saveMemo(SqlDataAccess *data, QUrl url, QString memo) {

	if (memo.length() > 0) {
        data->execute(QString("UPDATE Bookmark SET memo = ? WHERE hash_url = ?"), QVariantList() << memo << cleanUrlHash(url));
	} else {
        data->execute(QString("UPDATE Bookmark SET memo = NULL WHERE hash_url = ?"), QVariantList() << cleanUrlHash(url));
	}
}

void Bookmark::remove() {

	data->execute("DELETE FROM Bookmark WHERE hash_url = ?", QVariantList() << this->hashUrl);

	if (this->favicon != NULL) {
	    QFile *iconFile = new QFile(this->favicon);
	    if (iconFile->exists()) iconFile->remove();
	    iconFile->deleteLater();
	}

    if (this->image != NULL) {
        QFile *imageFile = new QFile(this->image);
        if (imageFile->exists()) imageFile->remove();
        imageFile->deleteLater();
    }

    emit downloadComplete(this->hashUrl);
}

void Bookmark::remove(SqlDataAccess *data, QUrl url) {

    QVariantList bookmarks = data->execute("SELECT * FROM Bookmark WHERE hash_url = ?", QVariantList() << cleanUrlHash(url)).toList();

    if (bookmarks.size() == 0) {
        return;
    }

    QVariantMap bookmark = bookmarks.value(0).toMap();

    if (!bookmark.value("favicon").isNull()) {
        QFile *iconFile = new QFile(bookmark.value("favicon").toString());
        if (iconFile->exists()) iconFile->remove();
        iconFile->deleteLater();
    }

    if (!bookmark.value("image").isNull()) {
        QFile *imageFile = new QFile(bookmark.value("image").toString());
        if (imageFile->exists()) imageFile->remove();
        imageFile->deleteLater();
    }

    data->execute("DELETE FROM Bookmark WHERE hash_url = ?", QVariantList() << cleanUrlHash(url));
}

QString Bookmark::cleanUrl(QUrl url) {

	QString clean = url.toString();
	if (clean.indexOf("://") > 0)
		clean = clean.right(clean.length() - clean.indexOf("://") - 3);
	if (clean.indexOf("www") == 0)
		clean = clean.right(clean.length() - 4);
	if (clean.length() > 0 && clean.at(clean.length() - 1) == '/')
		clean = clean.left(clean.length() - 1);

	return "http://" + QUrl(clean).toString();
}

uint Bookmark::cleanUrlHash(QUrl url) {

	return qHash(cleanUrl(url));
}

Bookmark::~Bookmark() {
}
