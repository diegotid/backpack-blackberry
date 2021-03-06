
#include "Backpack.hpp"

#include <Flurry.h>

#include <bb/ApplicationInfo>
#include <bb/cascades/Application>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/NavigationPane>
#include <bb/cascades/ActivityIndicator>
#include <bb/cascades/ProgressIndicator>
#include <bb/cascades/ActionItem>
#include <bb/cascades/ImageView>
#include <bb/cascades/ListView>
#include <bb/cascades/ListHeaderMode>
#include <bb/cascades/TextArea>
#include <bb/cascades/Sheet>
#include <bb/cascades/Page>
#include <bb/cascades/Label>
#include <bb/cascades/Button>
#include <bb/cascades/TitleBar>
#include <bb/cascades/ToggleButton>
#include <bb/cascades/RadioGroup>
#include <bb/system/SystemDialog>
#include <bb/data/XmlDataAccess>
#include <bb/data/JsonDataAccess>
#include <bb/PpsObject>

// #include <bb/cascades/core/developmentsupport.h>

using namespace bb::cascades;
using namespace bb::system;
using namespace bb::data;
using namespace bb;
using namespace std;

#define HOST "getpocket.com"
#define TEXT_HOST "text.getpocket.com"
#define APIKEY "22109-9f0af838570499419e7b5886"
#define ACKURL "http://bbornot2b.com/backpack/auth"
#define FRAMEINTERVAL 10

#define WORDS_MINUTE 180    // Average 180 words per minute on a monitor; Assumed 10000 bytes per minute
#define BYTES_WORD 10       // Average 10 bytes per word
#define IMAGES_SIZE 50      // Average web image size 50KB
#define MAX_IMAGES 150      // Maximum number of images to calculate offline download

#define ARTICLES_DIR "shared/downloads"
#define BACKUP_DIR "shared/documents"
#define APP_SUBDIR "Backpack"

#define HTML_STYLE "<style type='text/css'>p { float: left; clear: both; width: 100%; font-size: 2em; } img { float: left; clear: both; width: 100%; padding: 5px 0px; } h1 { font-size: 3em; } body { background-color: #f3e1c4; padding: 30px; }</style>"

const int Backpack::ALL = 0;
const int Backpack::FAVORITES = 1;
const int Backpack::ARTICLES = 2;
const int Backpack::VIDEOS = 3;
const int Backpack::IMAGES = 4;

Backpack::Backpack(bb::cascades::Application *app) : QObject(app) {

    dbFile.setFileName(QDir::home().absoluteFilePath("backpack.db"));
	dbFile.open(QIODevice::ReadWrite);
	data = new SqlDataAccess(dbFile.fileName(), "Backpack", this);

	createSettings();

	iManager = new InvokeManager(this);
	bool res_invoke = connect(iManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));
    Q_ASSERT(res_invoke);
    Q_UNUSED(res_invoke);

	network = new QNetworkAccessManager(this);
	bool get_fin = connect(network, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleGet(QNetworkReply*)));
	Q_ASSERT(get_fin);
	Q_UNUSED(get_fin);

    pocketUpdateTimer = new QTimer();
    pocketUpdateTimer->setSingleShot(false);
    bool updaterConnected = connect(pocketUpdateTimer, SIGNAL(timeout()), this, SLOT(pocketRetrieve()));
    Q_ASSERT(updaterConnected);
    Q_UNUSED(updaterConnected);

    downloadPending = false;

    downloadPendingTimer = new QTimer();
    downloadPendingTimer->setSingleShot(false);
    bool resumerConnected = connect(downloadPendingTimer, SIGNAL(timeout()), this, SLOT(resumeOfflineDownload()));
    Q_ASSERT(resumerConnected);
    Q_UNUSED(resumerConnected);

    currentDir = ARTICLES_DIR;

    downloadingSize = 0;
    downloadingProgress = 0;
    offlineCompleting = false;

    imgReply = NULL;
    betaReply = NULL;

	if (iManager->startupMode() == ApplicationStartupMode::InvokeApplication) {

		QmlDocument *invokedQml = QmlDocument::create("asset:///InvokedForm.qml").parent(this);
	    invokedQml->setContextProperty("app", this);
		invokedForm = invokedQml->createRootObject<Page>();

		app->setScene(invokedForm);

	} else {

		QmlDocument *homeQml = QmlDocument::create("asset:///main.qml").parent(this);
	    homeQml->setContextProperty("app", this);
	    mainPage = homeQml->createRootObject<TabbedPane>();

	    app->setScene(mainPage);
		activeFrame = (ActiveFrame*)app->cover();

	    frameUpdateTimer = new QTimer();
	    frameUpdateTimer->setSingleShot(false);
	    frameUpdateTimer->setInterval(FRAMEINTERVAL * 1000);
	    frameUpdateTimer->start();
	    bool updaterConnected = connect(frameUpdateTimer, SIGNAL(timeout()), this, SLOT(updateActiveFrame()));
	    Q_ASSERT(updaterConnected);
	    Q_UNUSED(updaterConnected);

		QSettings settings;
		QVariant username = settings.value("pocketUser");
		if (!username.isNull()) {
			mainPage->setProperty("username", username.toString());
			mainPage->findChild<Page*>("pocketPage")->setProperty("username", username.toString());
		}
		invokedForm = mainPage->findChild<Page*>("invokedForm");
		bookmarks = mainPage->at(0)->content()->findChild<ListView*>("bookmarks");

	    bookmarksByDate = new GroupDataModel(QStringList() << "date" << "time");
	    bookmarksByDate->setGrouping(ItemGrouping::ByFullValue);
	    bookmarksByDate->setSortedAscending(false);
	    bookmarks->setDataModel(bookmarksByDate);

	    bookmarksByURL = new GroupDataModel(QStringList() << "hash_url");
	    bookmarksByURL->setGrouping(ItemGrouping::ByFullValue);

		if (!username.isNull()
				&& !settings.value("pocketSync").isNull()
				&& settings.value("pocketSync").toBool()) {
			pocketRetrieve();
		} else { // pocketRetrieve already does refreshBookmarks when finished
			refreshBookmarks();
		}

		if (!settings.value("pocketUser").isNull()) {
            pocketUpdateTimer->setInterval(settings.value("pocketInterval", 1).toInt() * 1000 * 60 * 60);
			pocketUpdateTimer->start();
		} else {
			pocketUpdateTimer->stop();
		}
	}

//	mRoot = bb::cascades::Application::instance()->scene();
//	mRoot->setParent(this);
//	DevelopmentSupport* devSupport = new DevelopmentSupport(this);
//	bool res_changed = connect(devSupport, SIGNAL(assetsChanged(QUrl)), this, SLOT(reloadQML(QUrl)));
//	Q_ASSERT(res_changed);
//	Q_UNUSED(res_changed);
//
//	bool res_cleanup = connect(bb::cascades::Application::instance(), SIGNAL(aboutToQuit()), this, SLOT(cleanup()));
//	Q_ASSERT(res_cleanup);
//	Q_UNUSED(res_cleanup);
}

//void Backpack::cleanup() {
//	bb::cascades::Application::instance()->setScene(0);
//    mRoot->setParent(0);
//    delete mRoot;
//}
//
//void Backpack::reloadQML(QUrl mainFile) {
//
//    QDeclarativeContext* context = QDeclarativeEngine::contextForObject(mRoot);
//    QDeclarativeEngine* appEngine = context->engine();
//    appEngine->clearComponentCache();
//
//    QmlDocument* qml = QmlDocument::create(mainFile);
//    AbstractPane *root = qml->createRootObject<AbstractPane>(context);
//    qml->setParent(root);
//    bb::cascades::Application::instance()->setScene(root);
//}

void Backpack::updateActiveFrame() {

    updateActiveFrame(false);
}

void Backpack::updateActiveFrame(bool force) {

    QUrl toFetch = activeFrame->update(force);
    if (toFetch.toString().trimmed().length() > 0) {
        fetchContent(toFetch.toString().trimmed());
    }
}

QString Backpack::getAppVersion() {

	ApplicationInfo thisApp;
	return thisApp.version();
}

QString Backpack::getOfflineDir() {

    if (currentDir == ARTICLES_DIR) {
        currentDir = ".";
        return ARTICLES_DIR;
    } else {
        return currentDir;
    }
}

uint Backpack::getOfflineDirSize() {

    uint size = 0;

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    if (downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
        QFileInfoList downloadedFiles = downloadDir.entryInfoList(QDir::Files);
        for (QFileInfoList::iterator df = downloadedFiles.begin(); df != downloadedFiles.end(); df++) {
            size += df->size();
        }
    }

    return size;
}

QString Backpack::getReadStyle() {

    return HTML_STYLE;
}

void Backpack::logEvent(QString mode) {

    Flurry::Analytics::LogEvent(mode);
}

void Backpack::createSettings() {

    bool legacy = false;

	data->execute("SELECT COUNT(url) FROM Bookmark");
	if (data->hasError()) {
	    data->execute("CREATE TABLE Bookmark ("
	            "url VARCHAR(255), "
	            "title VARCHAR(255), "
	            "favicon VARCHAR(255), "
	            "memo VARCHAR(255), "
	            "time DATETIME, "
	            "size INTEGER, "
	            "keep BOOL)");
	} else {
	    legacy = true;
	}

    data->execute("SELECT COUNT(pocket_id) FROM Bookmark"); // Added on 2.0 for Pocket integration
    if (data->hasError()) {
        legacy = false;
        data->execute("ALTER TABLE Bookmark ADD image VARCHAR(255)");
        data->execute("ALTER TABLE Bookmark ADD pocket_id INTEGER");
        data->execute("ALTER TABLE Bookmark ADD type INTEGER");
    }

    data->execute("SELECT COUNT(hash_url) FROM Bookmark"); // Added on 2.0.1 for Pocket integration
    if (data->hasError()) {
        data->execute("ALTER TABLE Bookmark ADD hash_url VARCHAR(255)");
    }

    data->execute("SELECT COUNT(first_img) FROM Bookmark"); // Added on 3.1 for backing up images
    if (data->hasError()) {
        data->execute("ALTER TABLE Bookmark ADD first_img VARCHAR(255)");
    } else {
        legacy = false;
    }

    QSettings settings;
    if (settings.value("premium").isNull())
        settings.setValue("premium", legacy);
}

bool Backpack::isPremium() {

    QSettings settings;
    if (settings.value("premium").isNull())
        settings.setValue("premium", false);

    return settings.value("premium").toBool();
}

void Backpack::setPremium() {

    QSettings settings;
    settings.setValue("premium", true);
}

bool Backpack::getOfflineMode() {

    QSettings settings;
    if (settings.value("offline").isNull())
        settings.setValue("offline", true);

    return settings.value("offline").toBool();
}

void Backpack::setOfflineMode(bool active) {

    QSettings settings;
    settings.setValue("offline", active);

    Page *pocketSheet = mainPage->findChild<Page*>("pocketPage");
    if (pocketSheet) {
        pocketSheet->setProperty("offline", active);
    }

    if (active) {
        refreshBookmarks();
    } else {
        downloading.empty();
        downloadingImages.empty();
        if (imgReply != NULL && imgReply->isRunning()) imgReply->abort();
        if (betaReply != NULL && betaReply->isRunning()) betaReply->abort();
        downloadingSize = 0;
        downloadingProgress = 0;
        mainPage->setProperty("progress", 0);

        QDir downloadDir;
        downloadDir.setCurrent(QDir::homePath());
        downloadDir.setCurrent("..");
        if (downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
            QFileInfoList downloadedFiles = downloadDir.entryInfoList(QDir::Files);
            for (QFileInfoList::iterator df = downloadedFiles.begin(); df != downloadedFiles.end(); df++) {
                downloadDir.remove(df->fileName());
            }
        }
    }
}

bool Backpack::getOfflineImages() {

    QSettings settings;
    if (settings.value("offimages").isNull())
        settings.setValue("offimages", true);

    return settings.value("offimages").toBool();
}

void Backpack::setOfflineImages(bool available) {

    QSettings settings;
    settings.setValue("offimages", available);

    Page *pocketSheet = mainPage->findChild<Page*>("pocketPage");
    if (pocketSheet) {
        pocketSheet->setProperty("offimages", available);
    }

    if (!available) {
        int rest = downloadingImages.count();
        downloadingImages.empty();
        if (imgReply != NULL && imgReply->isRunning()) imgReply->abort();
        downloadingSize = downloadingSize - rest;
        downloadingProgress = downloadingProgress - rest;
        mainPage->setProperty("progress", (float)downloadingProgress/(float)downloadingSize);

        QDir downloadDir;
        downloadDir.setCurrent(QDir::homePath());
        downloadDir.setCurrent("..");
        if (downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
            QFileInfoList downloadedFiles = downloadDir.entryInfoList(QDir::Files);
            for (QFileInfoList::iterator df = downloadedFiles.begin(); df != downloadedFiles.end(); df++) {
                QString ext = df->fileName();
                ext = ext.right(ext.length() - ext.lastIndexOf('.'));
                if (ext.toLower() != ".html" && ext.toLower() != ".json") {
                    downloadDir.remove(df->fileName());
                }
            }
        }
    }
}

bool Backpack::getOfflineWiFi() {

    QSettings settings;
    if (settings.value("offwifi").isNull())
        settings.setValue("offwifi", false);

    return settings.value("offwifi").toBool();
}

void Backpack::setOfflineWiFi(bool onlyWiFi) {

    QSettings settings;
    settings.setValue("offwifi", onlyWiFi);

    Page *pocketSheet = mainPage->findChild<Page*>("pocketPage");
    if (pocketSheet) {
        pocketSheet->setProperty("offwifi", onlyWiFi);
    }
}

bool Backpack::getSettingsUnderstood() {

    QSettings settings;
    if (settings.value("settingsUnderstook").isNull())
        settings.setValue("settingsUnderstook", false);

    return settings.value("settingsUnderstook").toBool();
}

void Backpack::setSettingsUnderstood() {

    QSettings settings;
    settings.setValue("settingsUnderstook", true);
}

void Backpack::setKeepAfterRead(bool keep) {

	QSettings settings;
	settings.setValue("keepMode", keep);
}

bool Backpack::getKeepAfterRead() {

	QSettings settings;
	if (settings.value("keepMode").isNull())
		settings.setValue("keepMode", false);

	return settings.value("keepMode").toBool();
}

void Backpack::setPocketDeleteMode(bool pocketDelete) {

	QSettings settings;
	settings.setValue("pocketDelMode", pocketDelete);
}

int Backpack::getPocketDeleteMode() {

	QSettings settings;
	if (settings.value("pocketDelMode").isNull())
		settings.setValue("pocketDelMode", 0);

	return settings.value("pocketDelMode").toInt();
}

void Backpack::setIgnoreKeptShuffle(bool ignore) {

	QSettings settings;
	settings.setValue("ignoreKeptShuffle", ignore);
}

bool Backpack::getIgnoreKeptShuffle() {

	QSettings settings;
	if (settings.value("ignoreKeptShuffle").isNull())
		settings.setValue("ignoreKeptShuffle", true);

	return settings.value("ignoreKeptShuffle").toBool();
}

void Backpack::setIgnoreKeptQuickest(bool ignore) {

	QSettings settings;
	settings.setValue("ignoreKeptQuickest", ignore);
}

bool Backpack::getIgnoreKeptQuickest() {

	QSettings settings;
	if (settings.value("ignoreKeptQuickest").isNull())
		settings.setValue("ignoreKeptQuickest", true);

	return settings.value("ignoreKeptQuickest").toBool();
}

void Backpack::setIgnoreKeptLounge(bool ignore) {

	QSettings settings;
	settings.setValue("ignoreKeptLounge", ignore);
}

bool Backpack::getIgnoreKeptLounge() {

	QSettings settings;
	if (settings.value("ignoreKeptLounge").isNull())
		settings.setValue("ignoreKeptLounge", true);

	return settings.value("ignoreKeptLounge").toBool();
}

void Backpack::handleDownloadFailed(QUrl url) {

	if (iManager->startupMode() == ApplicationStartupMode::InvokeApplication
			|| mainPage->findChild<Sheet*>("bookmarkSheet")->isOpened()) {
		SystemToast *noConn = new SystemToast(this);
		SystemUiButton *closeButton = noConn->button();
		noConn->setBody("Unable to connect to the Internet to retrieve content");
		closeButton->setLabel("Ok");
		noConn->show();
		if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
			mainPage->findChild<Sheet*>("bookmarkSheet")->close();
		else {
			invokedForm->findChild<QObject*>("status")->setProperty("text", url.toString());
			invokedForm->findChild<Container*>("activity")->setVisible(false);
		}
	}
}

void Backpack::startPendingTimer() {

    downloadPending = true;
    downloadPendingTimer->setInterval(1000 * 60);
    downloadPendingTimer->start();
}

void Backpack::resumeOfflineDownload() {

    if (downloadPending && onWiFi()) {
        refreshBookmarks();
        downloadPending = false;
        downloadPendingTimer->stop();
    }
}

void Backpack::refreshBookmarks() {

    refreshBookmarks(NULL);
}

void Backpack::refreshBookmarks(QString query) {

	int type = Backpack::ALL;
	RadioGroup *filtro = mainPage->findChild<RadioGroup*>("filterType");
	if (filtro != NULL)
		type = filtro->selectedValue().toInt();

	QVariantList list;
	if (query.isNull()) {
		if (type == Backpack::ALL)
			list = data->execute("SELECT * FROM Bookmark").toList();
		else if (type == Backpack::FAVORITES)
			list = data->execute("SELECT * FROM Bookmark WHERE keep = ?", QVariantList() << true).toList();
		else
			list = data->execute("SELECT * FROM Bookmark WHERE type = ?", QVariantList() << type).toList();
	} else {
		QString queryString("%");
		queryString.append(query);
		queryString.append("%");
		if (type == Backpack::ALL)
			list = data->execute("SELECT * FROM Bookmark WHERE title LIKE ? OR memo LIKE ?", QVariantList() << queryString << queryString).toList();
		else if (type == Backpack::FAVORITES)
			list = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND (title LIKE ? OR memo LIKE ?)", QVariantList() << true << queryString << queryString).toList();
		else
			list = data->execute("SELECT * FROM Bookmark WHERE type = ? AND (title LIKE ? OR memo LIKE ?)", QVariantList() << type << queryString << queryString).toList();
	}

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    bool dirExists = downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR));

	QVariantList alreadyAdded;
	QVariantList staticList;
	for (int i = 0; i < list.size(); i++) {
		QVariantMap bmMap = list.at(i).toMap();
		if (bmMap["url"].isNull()
		|| alreadyAdded.contains(bmMap["url"])) {
		    continue;
		}
		uint urlHash = Bookmark::cleanUrlHash(QUrl(bmMap["url"].toString()));
        if (bmMap["title"].toString().length() == 0) {
            bmMap["title"] = QString("...");
            loading[urlHash] = new Bookmark(QUrl(bmMap["url"].toString()), data, this);
            loading[urlHash]->fetchContent();
            bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
            Q_ASSERT(res_loading_end);
            Q_UNUSED(res_loading_end);
        }
        if (getOfflineMode() && (!getOfflineWiFi() || onWiFi())) {
            QFile *jsonFile = new QFile(QString("%1.json").arg(urlHash));
            if (getOfflineImages() && jsonFile->open(QIODevice::ReadOnly)) {
                JsonDataAccess json;
                QVariantMap content = json.loadFromBuffer(jsonFile->readAll()).toMap();
                checkOfflineImages(content);
                jsonFile->close();
            } else {
                pocketProgressiveDownload(bmMap["url"].toString());
            }
            jsonFile->deleteLater();
        } else if (getOfflineMode() && !onWiFi()) {
            startPendingTimer();
        }
        if (bmMap["size"].toInt() == 0) bmMap["size"] = QString("");
		bmMap["date"] = bmMap["time"].toDate().toString("yyyy-MM-dd");
		alreadyAdded << bmMap["url"];
		staticList << bmMap;
	}
	list.clear();
	list << staticList;

    bookmarksByURL->clear();
    bookmarksByDate->clear();
    bookmarksByURL->insertList(list);
    bookmarksByDate->insertList(list);
    mainPage->findChild<Page*>("browseListPage")->setProperty("listSize", list.size());
    mainPage->findChild<Page*>("settingsSheet")->setProperty("backpackCount", list.size());

    updateActiveFrame(true);

	if (type == 0 && query.isNull() && list.size() == 0) {
        mainPage->findChild<Label*>("emptyHint")->setVisible(true);
        if (mainPage->findChild<NavigationPane*>("articlesPane")->count() == 1) {
            mainPage->setActiveTab(mainPage->findChild<Tab*>("putinTab"));
        }
	} else {
	    mainPage->findChild<Label*>("emptyHint")->setVisible(false);
	}
}

void Backpack::checkOfflineImages(QVariantMap content) {

    uint urlHash = content["hash"].toUInt();

    QFile *bodyFile = new QFile(QString("%1.html").arg(urlHash));
    bodyFile->open(QIODevice::ReadOnly);
    QTextStream source(bodyFile);
    QString body = source.readAll();

    bool checked = false;
    bool changed = false;

    if (content.contains("images")) {
        QVariantMap images = content.value("images").toMap();
        for (QVariantMap::iterator item = images.begin(); item != images.end(); item++) {
            int ord = item.key().toInt();
            QVariantMap image = item.value().toMap();
            QString imageFileName = image.value("src").toString();
            if (imageFileName.lastIndexOf('.') > imageFileName.lastIndexOf('/')) {
                QString ext(imageFileName.right(imageFileName.length() - imageFileName.lastIndexOf('.') - 1));
                if (ext.indexOf('?') > 0) {
                    ext = ext.left(ext.indexOf('?'));
                }
                imageFileName = QString("%1-%2.%3").arg(urlHash).arg(ord).arg(ext);
            } else {
                imageFileName = QString("%1-%2").arg(urlHash).arg(ord);
            }
            QFile imageFile(imageFileName);
            if (!imageFile.exists()) {
                image["hash"] = urlHash;
                if (downloadingSize == 0) {
                    initializeDownloadProgress();
                }
                downloadingSize++;
                mainPage->setProperty("progress", (float)downloadingProgress/(float)downloadingSize);
                if (getOfflineImages() && downloadingImages.isEmpty()) {
                    QNetworkRequest offlineImage(QUrl(image.value("src").toString()));
                    imgReply = network->get(offlineImage);
                }
                downloadingImages.append(image);
            }
            if (!checked) {
                if (getOfflineImages() && body.contains(image.value("src").toString())) {
                    body.replace(image.value("src").toString(), imageFileName);
                    changed = true;
                } else if (!getOfflineImages() && body.contains(imageFileName)) {
                    body.replace(imageFileName, image.value("src").toString());
                    changed = true;
                } else {
                    checked = true;
                }
            }
        }
        if (changed) {
            bodyFile->close();
            bodyFile->open(QIODevice::WriteOnly | QIODevice::Truncate);
            bodyFile->write(body.toAscii());
            bodyFile->flush();
        }
        bodyFile->close();
        bodyFile->deleteLater();
    }
}

void Backpack::saveBackup() {

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
	if (!appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR))) {
		appDir.setCurrent(BACKUP_DIR);
		appDir.mkdir(APP_SUBDIR);
		appDir.setCurrent(APP_SUBDIR);
	}
	QFile file(QDateTime::currentDateTime().toString("yyyyMMddhhmmsszzz") % ".xml");

	QVariantMap bookmarksList;
	bookmarksList["bookmark"] = data->execute("SELECT url, memo, time, keep FROM Bookmark WHERE url IS NOT NULL").toList();
	QVariantMap bookmarksRoot;
	bookmarksRoot["bookmarks"] = bookmarksList;

	if (file.open(QIODevice::ReadWrite)) {
		XmlDataAccess backup;
		backup.save(QVariant(bookmarksRoot), &file);
		file.close();
		showBackups();
	} else {
		SystemToast *noFiles = new SystemToast(this);
		noFiles->setBody("Unable to write file to system. Please check app permissions");
		noFiles->show();
	}
}

void Backpack::showBackups() {

	Page *backupSheet = mainPage->findChild<Page*>("backupPage");
	ListView *backupsList = backupSheet->findChild<ListView*>("backupsList");

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
    if (!appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR))) {
		return;
	}

	QVariantList list;
	QStringList filesList = appDir.entryList();
	for (int i = 0; i < filesList.size(); i++) {
		QString fileName = filesList.value(i);
		if (fileName.indexOf(".xml") < 0 || fileName.indexOf(".tmp") > 0)
			continue;
		QString date = QDate(fileName.left(4).toInt(), fileName.mid(4, 2).toInt(), fileName.mid(6, 2).toInt()).toString();
		date = date.left(1).toUpper() % date.right(date.length() - 1);
		QVariantMap entry;
		entry["file"] = fileName;
		entry["date"] = date;
		list << entry;
	}
	backupSheet->findChild<Container*>("lastBackupStatus")->setVisible(list.size() == 0);

	GroupDataModel *model = new GroupDataModel(QStringList() << "file");
	model->insertList(list);
	model->setGrouping(ItemGrouping::None);
	model->setSortedAscending(false);
	backupsList->setDataModel(model);
}

void Backpack::shareBackup(QString backupFilename) {

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
	appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR));

	InvokeManager invokeSender;
	InvokeRequest sending;
	sending.setAction("bb.action.COMPOSE");
	sending.setMimeType("message/rfc822");
	QFileInfo backupFile(backupFilename);
	QString date = QDate(backupFilename.left(4).toInt(), backupFilename.mid(4, 2).toInt(), backupFilename.mid(6, 2).toInt()).toString();
	date = date.left(1).toUpper() % date.right(date.length() - 1);
	QVariantMap meta;
	meta["subject"] = "Backpack backup file";
	meta["attachment"] = (QVariantList() << QString(QUrl(backupFile.absoluteFilePath()).toEncoded()));
	meta["body"] = QString("Find attached Backpack backup file dated ").append(date);
	QVariantMap emailData;
	emailData["data"] = meta;
	bool ok;
	sending.setData(bb::PpsObject::encode(emailData, &ok));
	invokeSender.invoke(sending);
}

void Backpack::restoreBackup(QString backupFilename) {

	QSettings settings;

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
	appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR));

	XmlDataAccess backup;
	QVariantList bookmarksList;
	QVariant backupContent = backup.load(backupFilename, "/bookmarks/bookmark");
	if (backupContent.canConvert(QVariant::List))
		bookmarksList << backupContent.toList();
	else
		bookmarksList << backupContent.toMap();

	for (int i = 0; i < bookmarksList.size(); i++) {
		QVariantMap backupData = bookmarksList.at(i).toMap();
		if (backupData["url"].isNull())
			continue;
		QUrl url(backupData["url"].toString());
		uint urlHash = Bookmark::cleanUrlHash(url);
        if (urlHash == 0)
            continue;
        if (1 > data->execute("SELECT COUNT(*) number FROM Bookmark WHERE hash_url = ?", QVariantList() << urlHash).toList().value(0).toMap().value("number").toInt()) {
			data->execute("INSERT INTO Bookmark (url, hash_url, time, keep, memo) VALUES (?, ?, ?, ?, ?)", QVariantList() << url.toString() << urlHash << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString());
		} else {
			data->execute("UPDATE Bookmark SET time = ?, keep = ?, memo = ? WHERE hash_url = ?", QVariantList() << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString() << urlHash);
		}
	}

	refreshBookmarks();

	backupToast = new SystemToast();
	QString date = QDate(backupFilename.left(4).toInt(), backupFilename.mid(4, 2).toInt(), backupFilename.mid(6, 2).toInt()).toString();
	date = date.left(1).toUpper() % date.right(date.length() - 1);
	backupToast->setBody(QString::number(bookmarksList.size()).append(" bookmarks restored"));
	SystemUiButton *viewButton = backupToast->button();
	viewButton->setLabel("View");
	timeout = new QTimer();
	timeout->setSingleShot(true);
	timeout->start(7500);
	bool res_toast = connect(timeout, SIGNAL(timeout()), backupToast, SLOT(cancel()));
    Q_ASSERT(res_toast);
    Q_UNUSED(res_toast);
	bool res_toast_act = connect(backupToast, SIGNAL(finished(bb::system::SystemUiResult::Type)), this, SLOT(restoreFinishedFeedback(bb::system::SystemUiResult::Type)));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);
	backupToast->show();

	QList<Button*> buttons = mainPage->findChildren<Button*>("restoreButton");
	for (int i = 0; i < buttons.size(); i++) {
		buttons.at(i)->setEnabled(true);
	}
}

void Backpack::importBackupFile(QString backupFilePath) {

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
	if (!appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR))) {
		appDir.setCurrent(BACKUP_DIR);
		appDir.mkdir(APP_SUBDIR);
		appDir.setCurrent(APP_SUBDIR);
	}
	QFileInfo backupFile(backupFilePath);
	QFile::copy(backupFilePath, backupFile.fileName());
	showBackups();
}

void Backpack::deleteBackup(QString backupFilename) {

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
	appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR));
	appDir.rename(backupFilename, backupFilename % ".tmp");
	showBackups();

	backupToast = new SystemToast();
	SystemUiButton *undoButton = backupToast->button();
	QString date = QDate(backupFilename.left(4).toInt(), backupFilename.mid(4, 2).toInt(), backupFilename.mid(6, 2).toInt()).toString();
	date = date.left(1).toUpper() % date.right(date.length() - 1);
	backupToast->setBody(date % "\nBackup file has been deleted");
	undoButton->setLabel("Undo");

	timeout = new QTimer();
	timeout->setSingleShot(true);
	timeout->start(3000);
	bool res_toast = connect(timeout, SIGNAL(timeout()), this, SLOT(deleteBackupConfirmation()));
    Q_ASSERT(res_toast);
    Q_UNUSED(res_toast);
    bool res_toast_act = connect(backupToast, SIGNAL(finished(bb::system::SystemUiResult::Type)), this, SLOT(deleteBackupFeedback(bb::system::SystemUiResult::Type)));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);

	backupToast->show();
}

void Backpack::restoreFinishedFeedback(bb::system::SystemUiResult::Type value) {

	if (value == SystemUiResult::ButtonSelection) {
        mainPage->findChild<Sheet*>("backupSheet")->close();
	}
	backupToast->deleteLater();
}

void Backpack::deleteBackupFeedback(bb::system::SystemUiResult::Type value) {

	if (value == SystemUiResult::ButtonSelection) {

        QDir appDir;
        appDir.setCurrent(QDir::homePath());
        appDir.setCurrent("..");
        appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR));
        QStringList filesList = appDir.entryList();

        for (int i = 0; i < filesList.size(); i++) {
            QString fileName = filesList.value(i);
            if (fileName.indexOf(".tmp") > 0)
                appDir.rename(fileName, fileName.left(fileName.indexOf(".tmp")));
        }
        showBackups();
	}
	backupToast->deleteLater();
}

void Backpack::deleteBackupConfirmation() {

    timeout->deleteLater();

	QDir appDir;
    appDir.setCurrent(QDir::homePath());
    appDir.setCurrent("..");
	appDir.setCurrent(QString("%1/%2").arg(BACKUP_DIR).arg(APP_SUBDIR));
	QStringList filesList = appDir.entryList();

	for (int i = 0; i < filesList.size(); i++) {
		QString fileName = filesList.value(i);
		if (fileName.indexOf(".tmp") > 0)
			appDir.remove(fileName);
	}
	backupToast->cancel();
}

void Backpack::freeLoadingPage(uint urlHash) {

    loading[urlHash]->deleteLater();
    loading.remove(urlHash);
}

void Backpack::handleGet(QNetworkReply *thisReply) {

    if (thisReply->url().toString().indexOf(HOST) > 0 || downloadingImages.isEmpty()) {
        return;
    }

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    if (!downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
        downloadDir.setCurrent(ARTICLES_DIR);
        downloadDir.mkdir(APP_SUBDIR);
        downloadDir.setCurrent(APP_SUBDIR);
    }

    QVariantMap image = downloadingImages.first().toMap();

    QString imageFileName = thisReply->url().toString();
    if (imageFileName.lastIndexOf('.') > imageFileName.lastIndexOf('/')) {
        QString ext(imageFileName.right(imageFileName.length() - imageFileName.lastIndexOf('.') - 1));
        if (ext.indexOf('?') > 0)
            ext = ext.left(ext.indexOf('?'));
        if (ext.indexOf('#') > 0)
            ext = ext.left(ext.indexOf('#'));
        imageFileName = QString("%1-%2.%3").arg(image["hash"].toString()).arg(image["image_id"].toString()).arg(ext);
    } else {
        imageFileName = QString("%1-%2").arg(image["hash"].toString()).arg(image["image_id"].toString());
    }
    QFile *imageFile = new QFile(imageFileName);
    imageFile->remove();

    if (imageFile->open(QIODevice::ReadWrite)) {

        imageFile->write(thisReply->readAll());
        imageFile->flush();

        imageFileName = QFileInfo(*imageFile).absoluteFilePath();

        if (image["image_id"].toInt() == 1) {
            data->execute("UPDATE Bookmark SET first_img = ? WHERE hash_url = ?", QVariantList() << QFileInfo(*imageFile).absoluteFilePath() << image["hash"].toString());
            updateImage(image["hash"].toUInt(), imageFileName);
        }

        Page *readPage = mainPage->findChild<Page*>("readPage");
        if (readPage && readPage->property("hash").toUInt() == image["hash"].toUInt()) {
            WebView *article = readPage->findChild<WebView*>("articleBody");
            if (article && article->url().isValid()) { // Avoids 404
                article->reload();
            }
        }

        QImage originalImage(imageFileName);
        if (originalImage.width() > 2048) {
            QImage smallerImage = originalImage.scaledToWidth(2048, Qt::FastTransformation);
            smallerImage.save(imageFileName);
        }
    }
    imageFile->close();
    imageFile->deleteLater();

    downloadingImages.removeFirst();
    downloadingProgress++;
    mainPage->setProperty("progress", (float)downloadingProgress/(float)downloadingSize);

    if (getOfflineImages() && !downloadingImages.empty()) {
        QNetworkRequest offlineImage(QUrl(downloadingImages.first().toMap().value("src").toString()));
        imgReply = network->get(offlineImage);
    } else if (downloading.isEmpty()) {
        downloadingSize = 0;
        downloadingProgress = 0;
        mainPage->setProperty("progress", 1);
        mainPage->findChild<Page*>("aboutSheet")->setProperty("taken", getOfflineDirSize());
    }
}

void Backpack::handleInvoke(const bb::system::InvokeRequest& request) {

    if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        mainPage->findChild<Sheet*>("bookmarkSheet")->open();
    }
    invokedForm->setProperty("item", NULL);
    invokedForm->findChild<QObject*>("invokedURL")->setProperty("text", request.uri().toString());

    TextArea *memoArea = invokedForm->findChild<TextArea*>("memo");

    uint urlHash = Bookmark::cleanUrlHash(request.uri());
    QVariantList bookmarks = data->execute("SELECT * FROM Bookmark WHERE hash_url = ?", QVariantList() << urlHash).toList();

    if (bookmarks.size() > 0) {
        QVariantMap bookmarkContent = bookmarks.value(0).toMap();
        invokedForm->titleBar()->setTitle("Edit item");
        invokedForm->findChild<QObject*>("status")->setProperty("text", "Article is already in your Backpack");
        invokedForm->findChild<QObject*>("title")->setProperty("text", bookmarkContent["title"]);
        invokedForm->findChild<ImageView*>("invokedImage")->setImageSource(QString("file://").append(bookmarkContent["image"].toString()));
        invokedForm->findChild<ToggleButton*>("keepCheck")->setProperty("invokeChecked", bookmarkContent["keep"]);
        invokedForm->findChild<Container*>("activity")->setVisible(false);
        invokedForm->findChild<Container*>("toggleFav")->setVisible(false);
        memoArea->setEnabled(false);
        memoArea->setText(bookmarkContent["memo"].toString());
        memoArea->setVisible(bookmarkContent["memo"].toString().trimmed().length() > 0);
        if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
            mainPage->findChild<Sheet*>("bookmarkSheet")->open();
        }
        return;
    }

    if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QDir downloadDir;
        downloadDir.setCurrent(QDir::homePath());
        downloadDir.setCurrent("..");
        bool dirExists = downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR));

        if (getOfflineMode() && (!dirExists || !QFile::exists(QString("%1.json").arg(urlHash)))) {
            if (!getOfflineWiFi() || onWiFi()) {
                pocketProgressiveDownload(request.uri().toString());
            } else {
                startPendingTimer();
            }
        }
    }

    invokedForm->findChild<ImageView*>("invokedImage")->setImageSource(QString("asset:///images/backpack.png"));
    invokedForm->findChild<QObject*>("activity")->setProperty("visible", true);
    invokedForm->findChild<QObject*>("status")->setProperty("text", "Fetching page content...");
    invokedForm->findChild<QObject*>("title")->setProperty("text", "");
    invokedForm->findChild<Container*>("toggleFav")->setVisible(true);
    invokedForm->findChild<ToggleButton*>("keepCheck")->setChecked(false);
    memoArea->setEnabled(true);
    memoArea->setVisible(true);
    memoArea->setText("");

    Backpack::add(request.uri());
}

void Backpack::add(QString url) {

    Backpack::add(QUrl(url));
}

void Backpack::add(QUrl url) {

    uint urlHash = Bookmark::cleanUrlHash(url);

    if (1 <= data->execute("SELECT * FROM Bookmark WHERE hash_url = ?", QVariantList() << urlHash).toList().size()) {
        return;
    }
    logEvent("Add");

    loading[urlHash] = new Bookmark(url, data, this);
    loading[urlHash]->fetchContent();
    bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
    Q_ASSERT(res_loading_end);
    Q_UNUSED(res_loading_end);

    QSettings settings;
    if (!settings.value("pocketUser").isNull()) {
        pocketPost(url);
    }

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
	    QVariantMap newMap;
        newMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(url));
        newMap["url"] = url.toString();
        newMap["time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        newMap["date"] = newMap["time"].toDate().toString("yyyy-MM-dd");
        newMap["keep"] = 0;
        bookmarksByURL->insert(newMap);
        bookmarksByDate->insert(newMap);
        mainPage->findChild<Page*>("browseListPage")->setProperty("listSize", bookmarksByURL->size());
        mainPage->findChild<Page*>("settingsSheet")->setProperty("backpackCount", bookmarksByURL->size());

        if (bookmarksByURL->size() > 0) {
            updateActiveFrame(true);
            mainPage->findChild<Label*>("emptyHint")->setVisible(false);
            mainPage->setActiveTab(mainPage->findChild<Tab*>("exploreTab"));
        }

        if (!toBeRemovedFiles.empty()) {
            QFileInfoList::iterator df = toBeRemovedFiles.begin();
            while (df != toBeRemovedFiles.end()) {
                if (df->fileName().indexOf(QString::number(urlHash)) >= 0) {
                    df = toBeRemovedFiles.erase(df);
                } else {
                    df++;
                }
            }
        }
	}
}

void Backpack::fetchContent(QString url) {

    uint urlHash = Bookmark::cleanUrlHash(url);

    if (!loading.contains(urlHash)) {
        loading[urlHash] = new Bookmark(url, data, this);
        bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
        Q_ASSERT(res_loading_end);
        Q_UNUSED(res_loading_end);
        loading[urlHash]->fetchContent();
    }

    QVariantMap queryMap;
    queryMap["hash_url"] = QString::number(urlHash);
    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
    QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
    bookmarkContent["fetched"] = true;
    bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
    bookmarksByDate->updateItem(indexPath, bookmarkContent);
}

void Backpack::memoBookmark(QString url, QString memo) {

	memoBookmark(QUrl(url), memo);
}

void Backpack::memoBookmark(QUrl url, QString memo) {

	Bookmark::saveMemo(data, url, memo);

	if (memo.length() == 0)
		invokedForm->findChild<TextArea*>("memo")->resetText();

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
	    QVariantMap queryMap;
	    queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(url));
	    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
	    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
	    QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
	    bookmarkContent["memo"] = memo;
	    bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
	    bookmarksByDate->updateItem(indexPath, bookmarkContent);
	} else {
		invokedForm->findChild<ActionItem*>("acceptButton")->setEnabled(false);
        uint urlHash = Bookmark::cleanUrlHash(url);
        if (urlHash == 0)
            return;
		if (memo.length() == 0 || !loading.contains(urlHash))
			invokedForm->findChild<ActionItem*>("dismissButton")->setTitle("Close");
		if (memo.length() > 0 && !loading.contains(urlHash))
			invokedForm->findChild<QObject*>("status")->setProperty("text", "Updated!");
	}
}

void Backpack::browseBookmark(QString uri) {

    InvokeManager invokeSender;
    InvokeRequest request;
    request.setTarget("sys.browser");
    request.setAction("bb.action.OPEN");
    request.setUri(uri);
    invokeSender.invoke(request);
}

void Backpack::readBookmark(QString uri) {

	QUrl url(uri);
    QVariantMap queryMap;
    queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(url));
    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    if (!downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
        pocketDownload(uri);
    } else {
        QFile *jsonFile = new QFile(QString("%1.json").arg(queryMap["hash_url"].toString()));
        if (jsonFile->open(QIODevice::ReadOnly)) {
            JsonDataAccess json;
            loadOffline(json.loadFromBuffer(jsonFile->readAll()).toMap());
            jsonFile->close();
            jsonFile->deleteLater();
        } else {
            pocketDownload(uri);
        }
    }

	if (!getKeepAfterRead() && !bookmarkContent["keep"].toBool()) {
		removeBookmark(url);
	}
}

void Backpack::shuffleBookmark() {

    QSettings settings;
    QVariantList ids;

    if (settings.value("ignoreKeptShuffle").isNull())
        settings.setValue("ignoreKeptShuffle", true);

    if (settings.value("ignoreKeptShuffle").toBool())
        ids = data->execute("SELECT url FROM Bookmark WHERE keep = ?", QVariantList() << false).toList();

    if (ids.size() == 0)
        ids = data->execute("SELECT url FROM Bookmark").toList();

    srand((unsigned)time(0));
    int randomNumber = rand() % ids.size();
    readBookmark(ids.value(randomNumber).toMap().value("url").toString());
}

QVariant Backpack::quickestBookmark() {

    return quickestBookmark(0);
}

QVariant Backpack::quickestBookmark(int offset) {

	QSettings settings;
	QVariantMap values;
	QVariantList ids;

	if (settings.value("ignoreKeptQuickest").isNull())
		settings.setValue("ignoreKeptQuickest", true);

	if (settings.value("ignoreKeptQuickest").toBool()) {
		ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND size IS NOT NULL AND size > 0 AND type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << false << Backpack::VIDEOS << Backpack::IMAGES << offset).toList();
		if (ids.size() == 0)
			ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << false << Backpack::VIDEOS << Backpack::IMAGES << offset).toList();
	}
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE size IS NOT NULL AND size > 0 AND type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES << offset).toList();
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES << offset).toList();

	if (settings.value("ignoreKeptQuickest").toBool()) {
		if (ids.size() == 0)
			ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND size IS NOT NULL AND size > 0 ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << false << offset).toList();
		if (ids.size() == 0)
			ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << false << offset).toList();
	}
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE size IS NOT NULL AND size > 0 ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << offset).toList();
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark ORDER BY size LIMIT 1 OFFSET ?", QVariantList() << offset).toList();

	values = ids.value(0).toMap();
	if (values.value("size").isNull())
		values["label"] = 0;
	else
		values["label"] = values.value("size").toInt();

	QUrl url = values.value("url").toUrl();
	uint urlHash = Bookmark::cleanUrlHash(url);
    if (urlHash == 0)
        return NULL;

    if (!loading.contains(urlHash)
    && (values.value("image").isNull() || values.value("image").toString().length() == 0)) {
        loading[urlHash] = new Bookmark(url, data, this);
        loading[urlHash]->fetchContent();
        bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
        Q_ASSERT(res_loading_end);
        Q_UNUSED(res_loading_end);
    }

	return QVariant(values);
}

QVariant Backpack::loungeBookmark() {

    return loungeBookmark(0);
}

QVariant Backpack::loungeBookmark(int offset) {

	QSettings settings;
	QVariantMap values;
	QVariantList ids;

	if (settings.value("ignoreKeptLounge").isNull())
		settings.setValue("ignoreKeptLounge", true);

	if (settings.value("ignoreKeptLounge").toBool())
		ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND type IS NOT ? AND type IS NOT ? ORDER BY size DESC LIMIT 1 OFFSET ?", QVariantList() << false << Backpack::VIDEOS << Backpack::IMAGES << offset).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE type IS NOT ? AND type IS NOT ? ORDER BY size DESC LIMIT 1 OFFSET ?", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES << offset).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark ORDER BY size DESC LIMIT 1 OFFSET ?", QVariantList() << offset).toList();

	values = ids.value(0).toMap();
	if (values.value("size").isNull())
		values["label"] = 0;
	else
		values["label"] = values.value("size").toInt();

	QUrl url = values.value("url").toUrl();
	uint urlHash = Bookmark::cleanUrlHash(url);
    if (urlHash == 0) return NULL;

    if (!loading.contains(urlHash)
    && (values.value("image").isNull() || values.value("image").toString().length() == 0)) {
        loading[urlHash] = new Bookmark(url, data, this);
        loading[urlHash]->fetchContent();
        bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
        Q_ASSERT(res_loading_end);
        Q_UNUSED(res_loading_end);
    }

	return QVariant(values);
}

void Backpack::handleBookmarkComplete(QUrl page, int size) {

	if (!data->hasError() && iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(page));
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        bookmarkContent["size"] = size;
        bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
        bookmarksByDate->updateItem(indexPath, bookmarkContent);
	}
}

void Backpack::updateImage(QUrl page, QUrl image) {

    updateImage(Bookmark::cleanUrlHash(page), image);
}

void Backpack::updateImage(uint hashUrl, QUrl image) {

    Label *urlLabel = invokedForm->findChild<Label*>("invokedURL");
    ImageView *imageView = invokedForm->findChild<ImageView*>("invokedImage");

    if (hashUrl == Bookmark::cleanUrlHash(QUrl(urlLabel->text())) && imageView->imageSource().toString().contains("images/backpack.png")) {
        imageView->setImageSource(QString("file://").append(image.toString()));
    }

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["hash_url"] = QString::number(hashUrl);
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        if (bookmarkContent["image"].toString().length() <= 1) {
            bookmarkContent["image"] = image.toString();
            bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
            bookmarksByDate->updateItem(indexPath, bookmarkContent);
        }
	}
}

void Backpack::updateImage(const char *item, QUrl image) {

	Page *homePage = mainPage->findChild<Page*>("homePage");
	QVariantMap itemValues = homePage->property(item).toMap();
	itemValues["image"] = image.toString();
	homePage->setProperty(item, QVariant(itemValues));
}

void Backpack::updateFavicon(QUrl page, QUrl favicon) {

    Label *urlLabel = invokedForm->findChild<Label*>("invokedURL");

    if (page.toString() == urlLabel->text()) {
        invokedForm->findChild<ImageView*>("invokedFavicon")->setImageSource(QString("file://").append(favicon.toString()));
    }

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(page));
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        bookmarkContent["favicon"] = favicon.toString();
        bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
        bookmarksByDate->updateItem(indexPath, bookmarkContent);
	}
}

void Backpack::updateTitle(QUrl page, QString title) {

    Label *urlLabel = invokedForm->findChild<Label*>("invokedURL");

    if (page.toString() == urlLabel->text()) {
        Label *titleLabel = invokedForm->findChild<Label*>("title");
        if (invokedForm->findChild<QObject*>("item") == 0) {
            invokedForm->findChild<Label*>("status")->setText("Added!");
        }
        titleLabel->setText(title);
        invokedForm->findChild<QObject*>("activity")->setProperty("visible", false);
    }

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(page));
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        bookmarkContent["title"] = title;
        bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
        bookmarksByDate->updateItem(indexPath, bookmarkContent);
	}
}

void Backpack::keepBookmark(QString url, bool keep) {

	keepBookmark(QUrl(url), keep);
}

void Backpack::keepBookmark(QUrl url, bool keep) {

	Bookmark::setKept(data, url, keep);

	QSettings settings;
	if (!settings.value("pocketUser").isNull()) {

	    QVariantMap query;
	    query.insert("consumer_key", APIKEY);
	    query.insert("access_token", settings.value("pocketToken").toString());
	    QVariantMap action;
	    action["action"] = keep ? "favorite" : "unfavorite";
	    action["item_id"] = Bookmark::getPocketId(data, url);
	    query.insert("actions", QVariantList() << action);

	    QByteArray queryArray;
	    JsonDataAccess json;
	    json.saveToBuffer(query, &queryArray);

	    QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/send"));
	    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	    network->post(request, queryArray);
	}

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

	    // Update fav indicator on list
        QVariantMap queryMap;
        queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(url));
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        bookmarkContent["keep"] = keep ? "true" : "false";
        bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
        bookmarksByDate->updateItem(indexPath, bookmarkContent);

        // Update fav indicator on read preview
        Page *previewSheet = mainPage->findChild<Page*>("browseDialog");
        QVariantMap proposedBookmark = previewSheet->property("bookmark").toMap();
        proposedBookmark["keep"] = keep ? "true" : "false";
        previewSheet->setProperty("bookmark", proposedBookmark);
	}
}

void Backpack::removeBookmark(QString url) {

	removeBookmark(QUrl(url));
}

void Backpack::removeBookmark(QUrl url) {

    QVariantMap queryMap;
    queryMap["hash_url"] = QString::number(Bookmark::cleanUrlHash(url));
    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
    QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
    bookmarksByURL->removeAt(indexPath);
    bookmarksByDate->removeAt(indexPath);

    QSettings settings;
    if (!settings.value("pocketUser").isNull()) {
        pocketArchiveDelete(Bookmark::getPocketId(data, url));
    }

    uint urlHash = Bookmark::cleanUrlHash(url);
    if (loading.contains(urlHash)) {
        loading[urlHash]->remove();
        loading.remove(urlHash);
    } else {
        Bookmark::remove(data, url);
    }

    if (getOfflineMode()) {
        QDir downloadDir;
        downloadDir.setCurrent(QDir::homePath());
        downloadDir.setCurrent("..");
        if (downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
            QStringList hashFilter;
            hashFilter << QString("%1*").arg(urlHash);
            toBeRemovedFiles << downloadDir.entryInfoList(hashFilter);
        }
    }

    if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        bookmarksByURL->remove(queryMap);
        mainPage->findChild<Page*>("browseListPage")->setProperty("listSize", bookmarksByURL->size());
        mainPage->findChild<Page*>("settingsSheet")->setProperty("backpackCount", bookmarksByURL->size());
        if (bookmarksByURL->size() == 0) {
            updateActiveFrame(true);
            mainPage->findChild<Label*>("emptyHint")->setVisible(true);
            if (mainPage->findChild<NavigationPane*>("articlesPane")->count() == 1) {
                mainPage->setActiveTab(mainPage->findChild<Tab*>("putinTab"));
            }
        }
        activeFrame->update(true);
    }
}

void Backpack::launchSearchToPutin(QString query) {

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.search");
	request.setAction("bb.action.OPEN");
	request.setUri(QString("search://?term=").append(query));
	invokeSender.invoke(request);
}

void Backpack::loadOffline(QVariantMap content) {

    Page *readPage = mainPage->findChild<Page*>("readPage");
    readPage->titleBar()->setTitle(content.value("title").toString());
    readPage->findChild<Label*>("headerHost")->setText(content.value("host").toString());

    bool isFavourite = false;
    QString articleDates;
    if (content.contains("datePublished")) {
        QDate published = content.value("datePublished").toDate();
        articleDates.append(published.toString());
    }

    uint hashUrl = Bookmark::cleanUrlHash(QUrl(content.value("resolvedUrl").toString()));

    QVariantList existings = data->execute("SELECT time, keep FROM Bookmark WHERE hash_url = ?", QVariantList() << hashUrl).toList();
    if (existings.length() > 0) {
        QVariantMap bookmarkContent = existings.value(0).toMap();
        isFavourite = bookmarkContent["keep"].toBool();
        QDate added = bookmarkContent["time"].toDate();
        articleDates.append((articleDates.length() > 0 ? " (added " : "Added ") % added.toString() % (articleDates.length() > 0 ? ")" : ""));
    }

    if (content.contains("article") && !content["article"].isNull()) {
        // Online content
        readPage->findChild<WebView*>("articleBody")->setProperty("body", content["article"]);
    } else {
        // Offline content
        QFile *bodyFile = new QFile(QString("%1.html").arg(hashUrl));
        if (bodyFile->exists()) {
            checkOfflineImages(content);
            readPage->findChild<WebView*>("articleBody")->setProperty("url", QString("file:////accounts/1000/%1/%2/%3").arg(ARTICLES_DIR).arg(APP_SUBDIR).arg(bodyFile->fileName()));
        }
    }
    readPage->setProperty("isFavourite", isFavourite);
    readPage->findChild<Label*>("headerDate")->setText(articleDates);
    readPage->setProperty("hash", hashUrl);
    readPage->setProperty("link", content.value("resolvedUrl").toString());
}

void Backpack::pocketConnect() {

	QUrl query;
	query.addQueryItem("consumer_key", APIKEY);
	query.addQueryItem("redirect_uri", ACKURL);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/oauth/request"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	reply = network->post(request, query.encodedQuery());
	bool res_posted = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_posted);
    Q_UNUSED(res_posted);
}

void Backpack::emptyBackapck() {

    QList<uint> hashses = loading.keys();

    for (int i = 0; i < hashses.size(); i++) {
        loading[hashses.at(i)]->remove();
    }
    loading.clear();
    bookmarksByURL->clear();
    bookmarksByDate->clear();

    mainPage->findChild<Page*>("browseListPage")->setProperty("listSize", 0);
    mainPage->findChild<Page*>("settingsSheet")->setProperty("backpackCount", 0);

    data->execute("DELETE FROM Bookmark");

    mainPage->findChild<Label*>("emptyHint")->setVisible(true);
    if (mainPage->findChild<NavigationPane*>("articlesPane")->count() == 1) {
        mainPage->setActiveTab(mainPage->findChild<Tab*>("putinTab"));
    }

    updateActiveFrame(true);

    downloading.empty();
    downloadingImages.empty();
    if (imgReply != NULL && imgReply->isRunning()) imgReply->abort();
    if (betaReply != NULL && betaReply->isRunning()) betaReply->abort();

    downloadingSize = 0;
    downloadingProgress = 0;
    mainPage->setProperty("progress", 0);

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    if (downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
        QFileInfoList downloadedFiles = downloadDir.entryInfoList(QDir::Files);
        for (QFileInfoList::iterator df = downloadedFiles.begin(); df != downloadedFiles.end(); df++) {
            downloadDir.remove(df->fileName());
        }
    }
}

void Backpack::pocketCompleteAuth() {

	QUrl query;
	query.addQueryItem("consumer_key", APIKEY);
	query.addQueryItem("code", requestToken);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/oauth/authorize"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
	reply = network->post(request, query.encodedQuery());
	bool res_posted = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_posted);
    Q_UNUSED(res_posted);
}

void Backpack::pocketRetrieve() {

    pocketRetrieve(false);
}

void Backpack::pocketRetrieve(bool info) {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());

	offlineCompleting = info;

	if (info) {
	    query.insert("detailType", "complete");
	} else {
	    mainPage->findChild<ActivityIndicator*>("syncingActivity")->setRunning(true);
	    if (!settings.value("pocketSynced").isNull()) {
	        QDateTime debugSynced;
	        debugSynced.setTime_t(settings.value("pocketSynced").toUInt());
	        query.insert("since", debugSynced.toTime_t());
	    }
	}

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/get"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");

	bool res_posted;
	if (info) {
        infoReply = network->post(request, queryArray);
        res_posted = connect(infoReply, SIGNAL(finished()), this, SLOT(pocketHandleInfoPostFinished()));
	} else {
	    reply = network->post(request, queryArray);
	    res_posted = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
	}
    Q_ASSERT(res_posted);
    Q_UNUSED(res_posted);
}

void Backpack::pocketProgressiveDownload(QString url) {

    if (downloading.isEmpty()) {
        initializeDownloadProgress();
        pocketDownload(url);
    }
    downloading.append(url);
    downloadingSize++;
    mainPage->setProperty("progress", (float)downloadingProgress/(float)downloadingSize);
}

void Backpack::pocketDownload(QString url) {

    QUrl query;
    query.addEncodedQueryItem("url", QUrl::toPercentEncoding(url));
    query.addQueryItem("consumer_key", APIKEY);
    query.addQueryItem("images", (getOfflineMode() && getOfflineImages()) ? "0" : "1");
    query.addQueryItem("videos", "1");
    query.addQueryItem("refresh", "1");
    query.addQueryItem("output", "json");

    QNetworkRequest request(QUrl(QString("http://") % TEXT_HOST % "/v3beta/text?hash=" % QString::number(Bookmark::cleanUrlHash(QUrl(url)))));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    betaReply = network->post(request, query.encodedQuery());
    bool res_posted = connect(betaReply, SIGNAL(finished()), this, SLOT(pocketHandleBetaPostFinished()));
    Q_ASSERT(res_posted);
    Q_UNUSED(res_posted);
}

void Backpack::initializeDownloadProgress() {

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    if (!downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
        downloadingSize = 0;
        downloadingProgress = 0;
    } else {
        downloadDir.setNameFilters(QStringList() << "*.json");
        downloadingProgress = downloadingSize = downloadDir.entryInfoList(QDir::Files).count();
    }
}

void Backpack::pocketPost(QUrl url) {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());
	query.insert("url", url.toEncoded());

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/add"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	reply = network->post(request, queryArray);
	bool res_posted = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_posted);
    Q_UNUSED(res_posted);
}

void Backpack::pocketArchiveDelete(qlonglong pocketId) {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());
	QVariantMap action;
	action["action"] = getPocketDeleteMode() ? "delete" : "archive";
	action["item_id"] = pocketId;
	query.insert("actions", QVariantList() << action);

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/send"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	network->post(request, queryArray);
}

void Backpack::pocketHandlePostFinished() {

    if (!reply->isFinished()) {
        return;
    }

    if (reply->request().url().toString().indexOf("v3/oauth/request") > 0) {

        if (reply->rawHeader("Status").isNull() || reply->rawHeader("Status").indexOf("200 OK") != 0) {
            mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on connecting to server");
            return;
        }
        requestToken = reply->readAll();
        requestToken = requestToken.right(requestToken.length() - requestToken.indexOf("code=") - 5);

        QString uri("https://");
        uri.append(HOST);
        uri.append("/auth/authorize?");
        uri.append(QString("request_token=") % requestToken);
        uri.append(QString("&redirect_uri=") % ACKURL);

        InvokeManager invokeSender;
        InvokeRequest request;
        request.setTarget("sys.browser");
        request.setAction("bb.action.OPEN");
        request.setUri(uri);
        invokeSender.invoke(request);

    } else if (reply->request().url().toString().indexOf("v3/oauth/authorize") > 0) {

        if (reply->rawHeader("Status").isNull() || reply->rawHeader("Status").indexOf("200 OK") != 0) {
            mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on authorizing Backpack");
            return;
        }

        QByteArray response = reply->readAll();
        QString username = response.right(response.length() - response.lastIndexOf('=') - 1).replace("%40", "@");
        mainPage->setProperty("username", username);

        response = response.left(response.indexOf('&'));
        QString accessToken = response.right(response.length() - response.lastIndexOf('=') - 1);

        QSettings settings;
        settings.setValue("pocketUser", username);
        settings.setValue("pocketToken", accessToken);
        if (settings.value("offline").isNull()) {
            settings.setValue("offline", false);
        }
        mainPage->findChild<Page*>("pocketPage")->setProperty("username", username);

        QVariantList existing = data->execute("SELECT url, keep FROM Bookmark").toList();

        pocketRetrieve(settings.value("offline").toBool());

        QVariantMap bookmark;
        for (int i = 0; i < existing.size(); i++) {
            bookmark = existing.at(i).toMap();
            pocketPost(QUrl(bookmark.value("url").toString()));
            if (bookmark.value("keep").toBool()) {
                favLegacy << bookmark.value("url").toString();
            }
        }

    } else if (reply->request().url().toString().indexOf("v3/add") > 0) {

        if (reply->rawHeader("Status").isNull()
                || reply->rawHeader("Status").indexOf("200 OK") != 0
                || reply->error() != QNetworkReply::NoError) {
            mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on adding new item. It will be automatically synced when possible");
            return;
        }
        JsonDataAccess json;
        QVariantMap response = json.loadFromBuffer(reply->readAll()).toMap().value("item").toMap();

        qlonglong pocketId = response["item_id"].toLongLong();
        QString pocketUrl = response["normal_url"].toString();

        data->execute("UPDATE Bookmark SET pocket_id = ? WHERE hash_url = ?", QVariantList() << pocketId << Bookmark::cleanUrlHash(QUrl(pocketUrl)));

    } else if (reply->request().url().toString().indexOf("v3/get") > 0) {

        if (reply->rawHeader("Status").isNull() || reply->rawHeader("Status").indexOf("200 OK") != 0) {
            mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on retrieving contents");
            QList<QObject*> pocketSignals = mainPage->findChildren<QObject*>("pocketErrorSignal");
            for (int i = 0; i < pocketSignals.length(); i++) {
                pocketSignals.at(i)->setProperty("visible", true);
            }
            return;
        }

        JsonDataAccess json;
        QByteArray temp = reply->readAll();
        QVariantMap response = json.loadFromBuffer(temp).toMap();

        QVariantList retrieved = response["list"].toMap().values();
        QString lastSynced = response["since"].toString();

        QSettings settings;
        settings.setValue("pocketSynced", lastSynced);

        QDateTime debugSynced;
        debugSynced.setTime_t(lastSynced.toUInt());

        QVariantList toInsert;
        QVariantList toUpdate;
        QVariantList toDelete;

        for (int i = 0; i < retrieved.size(); i++) {
            QVariantMap item = retrieved.value(i).toMap();
            QUrl url(item.value("resolved_url").toString());
            uint urlHash = Bookmark::cleanUrlHash(url);
            if (urlHash == 0) continue;
            switch (item.value("status").toInt()) {
                case 1:
                case 2:
                    toDelete << QVariant::fromValue(QVariantList() << urlHash);
                    if (loading.contains(urlHash)) {
                        freeLoadingPage(urlHash);
                    }
                    break;
                default:
                    qlonglong pocketId = item.value("item_id").toLongLong();
                    int size = 10000 * item.value("word_count").toInt() / WORDS_MINUTE;
                    QString title = item.value("resolved_title").toString();
                    bool favorited = item.value("favorite").toBool()
                            || favLegacy.contains(item.value("resolved_url").toString());
                    QDateTime pocketAdded = QDateTime::fromTime_t(item.value("time_added").toInt());
                    if (1 > data->execute("SELECT COUNT(*) number FROM Bookmark WHERE hash_url = ?", QVariantList() << urlHash).toList().value(0).toMap().value("number").toInt()) {
                        toInsert << QVariant::fromValue(QVariantList() << url.toString() << urlHash << title << size << favorited << pocketId << pocketAdded);
                        if (getOfflineMode() && (!getOfflineWiFi() || onWiFi())) {
                            pocketProgressiveDownload(url.toString());
                        } else if (getOfflineMode() && !onWiFi()) {
                            startPendingTimer();
                        }
                    } else {
                        toUpdate << QVariant::fromValue(QVariantList() << pocketId << size << favorited << pocketAdded << urlHash);
                    }
                    break;
            }
        }
        data->executeBatch("INSERT INTO Bookmark (url, hash_url, title, size, keep, pocket_id, time) VALUES (?, ?, ?, ?, ?, ?, ?)", toInsert);
        data->executeBatch("UPDATE Bookmark SET pocket_id = ?, size = ?, keep = ?, time = ? WHERE hash_url = ?", toUpdate);
        data->executeBatch("DELETE FROM Bookmark WHERE hash_url = ?", toDelete);

        for (int i = 0; i < favLegacy.count(); i++) {
            keepBookmark(favLegacy.at(i), true);
        }
        favLegacy.empty();

        mainPage->findChild<Container*>("syncingIndicator")->setVisible(false);
        QList<QObject*> pocketSignals = mainPage->findChildren<QObject*>("pocketConnSignal");
        for (int i = 0; i < pocketSignals.length(); i++) {
            pocketSignals.at(i)->setProperty("visible", true);
        }

        refreshBookmarks();
        updateActiveFrame(true);
    }
}

void Backpack::pocketHandleInfoPostFinished() {

    if (!infoReply->isFinished()) {
        return;
    }

    if (infoReply->request().url().toString().indexOf("v3/get") > 0) {

        if (infoReply->rawHeader("Status").isNull() || infoReply->rawHeader("Status").indexOf("200 OK") != 0) {
            return;
        }

        JsonDataAccess json;
        QByteArray temp = infoReply->readAll();
        QVariantMap response = json.loadFromBuffer(temp).toMap();

        headingSize = 0;
        headingImagesSize = 0;
        headingImages.empty();
        QVariantList retrieved = response["list"].toMap().values();

        QVariantList images;
        for (QVariantList::iterator item = retrieved.begin(); item != retrieved.end(); item++) {
            headingSize += (*item).toMap().value("word_count").toInt() * BYTES_WORD;
            images = (*item).toMap().value("images").toMap().values();
            for (QVariantList::iterator image = images.begin(); image != images.end(); image++) {
                headingImages << (*image).toMap().value("src").toString();
            }
        }

        if (headingImages.count() < MAX_IMAGES) {
            imageInfoPost();
        } else {
            headingImagesSize = IMAGES_SIZE * 1024 * headingImages.count();
            mainPage->findChild<Page*>("pocketPage")->setProperty("images", headingImagesSize);
            mainPage->findChild<Page*>("pocketPage")->setProperty("fulldownload", headingSize + headingImagesSize);
        }
    }
}

void Backpack::imageInfoPost() {

    if (headingImages.isEmpty()) {
        mainPage->findChild<Page*>("pocketPage")->setProperty("images", headingImagesSize);
        mainPage->findChild<Page*>("pocketPage")->setProperty("fulldownload", headingSize + headingImagesSize);
        return;
    }

    QString resolvedUrl = headingImages.first().toString();
    headingImages.removeFirst();

    bool res_posted;
    infoReply = network->head(QNetworkRequest(QUrl(resolvedUrl)));
    res_posted = connect(infoReply, SIGNAL(finished()), this, SLOT(handleHeadFinished()));
    Q_ASSERT(res_posted);
    Q_UNUSED(res_posted);
}

void Backpack::handleHeadFinished() {

    if (!infoReply->isFinished()) {
        return;
    }

    QUrl redirect = infoReply->attribute(QNetworkRequest::RedirectionTargetAttribute).toUrl();
    if (redirect.isValid()) {
        if (redirect.isRelative()) {
            redirect.setScheme(reply->url().scheme());
            redirect.setHost(reply->url().host());
        }
        network->head(QNetworkRequest(redirect));
    } else {
        headingImagesSize += infoReply->header(QNetworkRequest::ContentLengthHeader).toUInt();
        imageInfoPost();
    }
}

void Backpack::pocketHandleBetaPostFinished() {

    if (!betaReply->isFinished())
        return;

    QString requestUrl = betaReply->request().url().toString();

    if (requestUrl.indexOf("v3beta/text") > 0) {

        if (betaReply->rawHeader("Status").isNull()
                || betaReply->rawHeader("Status").indexOf("200 OK") != 0
                || betaReply->error() != QNetworkReply::NoError) {
            return;
        }
        JsonDataAccess json;
        QVariantMap response = json.loadFromBuffer(betaReply->readAll()).toMap();

        bool finished = true;

        if (getOfflineMode()) {

            uint hashUrl = requestUrl.right(requestUrl.length() - requestUrl.indexOf('=') - 1).toUInt();

            QString article = response["article"].toString();
            QString body = QString("<html><head>%1</head><body><h1>%2</h1>%3</body></html>")
                                    .arg(HTML_STYLE)
                                    .arg(response["title"].toString())
                                    .arg(article);
            response["hash"] = hashUrl;
            response.remove("article");

            QVariantMap image;
            QVariantList images = response["images"].toMap().values();
            if (getOfflineImages()) {
                for (int i = 0; i < images.size(); i++) {
                    image = images.at(i).toMap();
                    QString ext = image["src"].toString();
                    ext = ext.right(ext.length() - ext.lastIndexOf('.') - 1);
                    body.replace(QString("<!--IMG_%1-->").arg(i + 1), QString("<img src=\"%1-%2.%3\"/>").arg(hashUrl).arg(i + 1).arg(ext));
                    image["hash"] = hashUrl;
                    downloadingImages.append(image);
                    downloadingSize++;
                    mainPage->setProperty("progress", (float)downloadingProgress/(float)downloadingSize);
                }
            }

            QDir downloadDir;
            downloadDir.setCurrent(QDir::homePath());
            downloadDir.setCurrent("..");
            if (!downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
                downloadDir.setCurrent(ARTICLES_DIR);
                downloadDir.mkdir(APP_SUBDIR);
                downloadDir.setCurrent(APP_SUBDIR);
            }

            QFile *jsonFile = new QFile(QString("%1.json").arg(hashUrl));
            jsonFile->remove();
            jsonFile->open(QIODevice::ReadWrite);
            json.save((QVariant)response, jsonFile);
            jsonFile->flush();
            jsonFile->close();
            jsonFile->deleteLater();

            QFile *htmlFile = new QFile(QString("%1.html").arg(hashUrl));
            htmlFile->remove();
            htmlFile->open(QIODevice::ReadWrite);
            htmlFile->write(body.toAscii());
            htmlFile->flush();
            htmlFile->close();
            htmlFile->deleteLater();

            if (!downloading.isEmpty()) {
                downloading.removeFirst();
                downloadingProgress++;
                mainPage->setProperty("progress", (float)downloadingProgress/(float)downloadingSize);
            }

            if (!downloading.isEmpty()) {
                finished = false;
                pocketDownload(downloading.first().toString());
            }

            if (getOfflineImages() && !images.empty() && downloadingImages.size() == images.size()) {
                finished = false;
                QNetworkRequest offlineImage(QUrl(images.first().toMap().value("src").toString()));
                imgReply = network->get(offlineImage);
            }
        }

        if (mainPage->findChild<NavigationPane*>("articlesPane")->count() > 1) {
            Page *readPage = mainPage->findChild<Page*>("readPage");
            if (readPage && readPage->property("hash").isNull()) {
                loadOffline(response);
            }
        }

        if (finished && downloadingImages.isEmpty()) {
            downloadingSize = 0;
            downloadingProgress = 0;
            mainPage->setProperty("progress", 1);
            mainPage->findChild<Page*>("aboutSheet")->setProperty("taken", getOfflineDirSize());
        }
    }
}

bool Backpack::pocketGetSynconstartup() {

	QSettings settings;
	if (settings.value("pocketSync").isNull())
		settings.setValue("pocketSync", true);

	return settings.value("pocketSync").toBool();
}

void Backpack::pocketSetSynconstartup(bool sync) {

	QSettings settings;
	settings.setValue("pocketSync", sync);
}

int Backpack::pocketInterval() {

	QSettings settings;
	return settings.value("pocketInterval", 1).toInt();
}

void Backpack::pocketSetInterval(int interval) {

	QSettings settings;
	if (settings.value("pocketUser").isNull() && interval > 0) {
	    settings.setValue("pocketInterval", interval);
		pocketUpdateTimer->setInterval(interval * 1000 * 60 * 60);
		pocketUpdateTimer->start();
	} else {
	    settings.remove("pocketInterval");
		pocketUpdateTimer->stop();
	}
}

void Backpack::pocketDisconnect() {

	QSettings settings;
	settings.remove("pocketUser");
	settings.remove("pocketTocken");
    settings.remove("pocketSynced");
    settings.remove("pocketInterval");
	mainPage->setProperty("username", "");

    pocketUpdateTimer->stop();

	data->execute("UPDATE Bookmark SET pocket_id = NULL");
}

bool Backpack::onWiFi() {

    foreach (QNetworkInterface interface, QNetworkInterface::allInterfaces()) {
        if (interface.flags().testFlag(QNetworkInterface::IsUp)
                && interface.name().compare(QString("bcm0")) == 0
                && interface.addressEntries().count() > 1) {
            return true;
        }
    }

    return false;
}

Backpack::~Backpack() {

    QSettings settings;
    if (offlineCompleting && settings.contains("pocketUser")) {
        settings.remove("pocketUser");
        settings.remove("pocketTocken");
        settings.remove("pocketSynced");
        settings.remove("pocketInterval");
        data->execute("UPDATE Bookmark SET pocket_id = NULL");
    }

    QDir downloadDir;
    downloadDir.setCurrent(QDir::homePath());
    downloadDir.setCurrent("..");
    if (downloadDir.setCurrent(QString("%1/%2").arg(ARTICLES_DIR).arg(APP_SUBDIR))) {
        for (QFileInfoList::iterator df = toBeRemovedFiles.begin(); df != toBeRemovedFiles.end(); df++) {
            downloadDir.remove(df->fileName());
        }
    }

	data->connection().close();
	dbFile.close();
}
