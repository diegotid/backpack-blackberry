
#include "Backpack.hpp"

#include <Flurry.h>

#include <bb/ApplicationInfo>
#include <bb/cascades/Application>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/ActivityIndicator>
#include <bb/cascades/ActionItem>
#include <bb/cascades/ImageView>
#include <bb/cascades/ListView>
#include <bb/cascades/TextArea>
#include <bb/cascades/Sheet>
#include <bb/cascades/Page>
#include <bb/cascades/Label>
#include <bb/cascades/Button>
#include <bb/cascades/TitleBar>
#include <bb/cascades/ToggleButton>
#include <bb/cascades/DropDown>
#include <bb/data/XmlDataAccess>
#include <bb/data/JsonDataAccess>
#include <bb/PpsObject>

//#include <bb/cascades/core/developmentsupport.h>

using namespace bb::cascades;
using namespace bb::system;
using namespace bb::data;
using namespace bb;
using namespace std;

#define HOST "getpocket.com"
#define APIKEY "22109-9f0af838570499419e7b5886"
#define ACKURL "http://bbornot2b.com/backpack/auth"

const int Backpack::ALL = 0;
const int Backpack::FAVORITES = 1;
const int Backpack::ARTICLES = 2;
const int Backpack::VIDEOS = 3;
const int Backpack::IMAGES = 4;

Backpack::Backpack(bb::cascades::Application *app) : QObject(app) {

	dbFile.setFileName(QDir::home().absoluteFilePath("backpack.db"));
	dbFile.open(QIODevice::ReadWrite);
	data = new SqlDataAccess(dbFile.fileName(), "Backpack", this);

	if (!databaseExists()) createDatabase();

	iManager = new InvokeManager(this);
	connect(iManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

	network = new QNetworkAccessManager(this);

	updateTimer = new QTimer();
	updateTimer->setSingleShot(false);
	connect(updateTimer, SIGNAL(timeout()), this, SLOT(pocketRetrieve()));

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

		QSettings settings;
		QVariant username = settings.value("pocketUser");
		if (!username.isNull()) {
			mainPage->setProperty("username", username.toString());
			mainPage->findChild<Page*>("pocketPage")->setProperty("username", username.toString());
		}
		invokedForm = mainPage->findChild<Page*>("invokedForm");
		bookmarks = mainPage->at(1)->content()->findChild<ListView*>("bookmarks");

		if (!username.isNull()
				&& !settings.value("pocketSync").isNull()
				&& settings.value("pocketSync").toBool()) {
			refreshBookmarks(false);
			pocketRetrieve();
		} else { // pocketRetrieve already does refreshBookmarks when finished
			refreshBookmarks(true);
		}

		if (settings.value("pocketInterval", 1).toInt() > 0) {
			updateTimer->setInterval(settings.value("pocketInterval", 1).toInt() * 1000 * 60 * 60);
			updateTimer->start();
		} else {
			updateTimer->stop();
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

uint Backpack::cleanUrlHash(QUrl url) {

	QString clean = url.toString();
	if (clean.indexOf("://") > 0)
		clean = clean.right(clean.length() - clean.indexOf("://") - 3);
	if (clean.indexOf("www") == 0)
		clean = clean.right(clean.length() - 4);
	if (clean.at(clean.length() - 1) == '/')
		clean = clean.left(clean.length() - 1);

	return qHash(clean);
}

void Backpack::cleanup() {
	bb::cascades::Application::instance()->setScene(0);
    mRoot->setParent(0);
    delete mRoot;
}

void Backpack::reloadQML(QUrl mainFile) {

    QDeclarativeContext* context = QDeclarativeEngine::contextForObject(mRoot);
    QDeclarativeEngine* appEngine = context->engine();
    appEngine->clearComponentCache();

    QmlDocument* qml = QmlDocument::create(mainFile);
    AbstractPane *root = qml->createRootObject<AbstractPane>(context);
    qml->setParent(root);
    bb::cascades::Application::instance()->setScene(root);
}

QString Backpack::getAppVersion() {

	ApplicationInfo thisApp;
	return thisApp.version();
}

bool Backpack::databaseExists() {

	data->execute("SELECT COUNT(pocket_id) FROM Bookmark");

	return !data->hasError();
}

void Backpack::createDatabase() {

	data->execute("CREATE TABLE Bookmark (id INTEGER, "
										"title VARCHAR(255), "
										"url VARCHAR(255), "
										"favicon VARCHAR(255), "
										"memo VARCHAR(255), "
										"time DATETIME, "
										"size INTEGER, "
										"keep BOOL)");

	data->execute("ALTER TABLE Bookmark ADD image VARCHAR(255)"); // Added on 2.0 for Pocket integration
	data->execute("ALTER TABLE Bookmark ADD pocket_id INTEGER"); // Added on 2.0 for Pocket integration
	data->execute("ALTER TABLE Bookmark ADD type INTEGER"); // Added on 2.0 for Pocket integration
}

void Backpack::setKeepAfterRead(int mode) {

	QSettings settings;
	settings.setValue("keepMode", mode);
}

int Backpack::getKeepAfterRead() {

	QSettings settings;
	if (settings.value("keepMode").isNull())
		settings.setValue("keepMode", 0);
	return settings.value("keepMode").toInt();
}

void Backpack::setPocketDeleteMode(int mode) {

	QSettings settings;
	settings.setValue("pocketDelMode", mode);
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

void Backpack::setIgnoreKeptOldest(bool ignore) {

	QSettings settings;
	settings.setValue("ignoreKeptOldest", ignore);

	takeOldestBookmark();

	activeFrame->takeFigures(this);
}

bool Backpack::getIgnoreKeptOldest() {

	QSettings settings;
	if (settings.value("ignoreKeptOldest").isNull())
		settings.setValue("ignoreKeptOldest", true);
	return settings.value("ignoreKeptOldest").toBool();
}

void Backpack::setIgnoreKeptQuickest(bool ignore) {

	QSettings settings;
	settings.setValue("ignoreKeptQuickest", ignore);

	takeQuickestBookmark();

    activeFrame->takeFigures(this);
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

	takeLoungeBookmark();

	activeFrame->takeFigures(this);
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
		noConn->setBody("Unable to connect to the Internet to retreive content");
		closeButton->setLabel("Ok");
		noConn->show();
		if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
			mainPage->findChild<Sheet*>("bookmarkSheet")->close();
		else {
			invokedForm->findChild<QObject*>("status")->setProperty("text", url.toString());
			invokedForm->findChild<Container*>("activity")->setVisible(false);
		}
	}
	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

QDate Backpack::getOldestDate() {

	QSettings settings;
	if (settings.value("ignoreKeptOldest").isNull())
		settings.setValue("ignoreKeptOldest", true);

	QVariant oldest;

	if (settings.value("ignoreKeptOldest").toBool())
		oldest = data->execute("SELECT MIN(time) FROM Bookmark WHERE keep = ? AND time IS NOT NULL", QVariantList() << false).toList().value(0).toMap().value("MIN(time)");

	if (oldest.isNull())
		oldest = data->execute("SELECT MIN(time) FROM Bookmark WHERE time IS NOT NULL").toList().value(0).toMap().value("MIN(time)");

	if (oldest.isNull())
		return QDate::currentDate();
	else
		return oldest.toDate();
}

int Backpack::getQuickestSize() {

	QSettings settings;
	if (settings.value("ignoreKeptQuickest").isNull())
		settings.setValue("ignoreKeptQuickest", true);

	QVariant quickest;

	if (settings.value("ignoreKeptQuickest").toBool()) {
		quickest = data->execute("SELECT MIN(size) FROM Bookmark WHERE keep = ? AND size IS NOT NULL AND size > 0", QVariantList() << false).toList().value(0).toMap().value("MIN(size)");
		if (quickest.isNull() || quickest.toInt() == 0)
			quickest = data->execute("SELECT MIN(size) FROM Bookmark WHERE keep = ?", QVariantList() << false).toList().value(0).toMap().value("MIN(size)");
	}

	if (quickest.isNull() || quickest.toInt() == 0)
		quickest = data->execute("SELECT MIN(size) FROM Bookmark WHERE size IS NOT NULL AND size > 0").toList().value(0).toMap().value("MIN(size)");
	if (quickest.isNull() || quickest.toInt() == 0)
		quickest = data->execute("SELECT MIN(size) FROM Bookmark").toList().value(0).toMap().value("MIN(size)");

	if (quickest.isNull())
		return 0;
	else
		return quickest.toInt();
}

void Backpack::refreshBookmarks() {

	refreshBookmarks(false);
}

void Backpack::refreshBookmarks(bool reload) {

	refreshBookmarks(reload, NULL);
}

void Backpack::refreshBookmarks(QString query) {

	refreshBookmarks(false, query);
}

void Backpack::refreshBookmarks(bool reload, QString query) {

	int type = mainPage->findChild<DropDown*>("filterType")->selectedValue().toInt();

	QVariantList list;
	if (query.isNull())
		if (type == Backpack::ALL)
			list = data->execute("SELECT * FROM Bookmark").toList();
		else if (type == Backpack::FAVORITES)
			list = data->execute("SELECT * FROM Bookmark WHERE keep = ?", QVariantList() << true).toList();
		else
			list = data->execute("SELECT * FROM Bookmark WHERE type = ?", QVariantList() << type).toList();
	else {
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

	QVariantList staticList;
	for (int i = 0; i < list.size(); i++) {
		QVariantMap bmMap = list.at(i).toMap();
		if (bmMap["url"].isNull()) continue;

		QUrl url(bmMap["url"].toString());
		uint hashUrl = cleanUrlHash(url);
		if (!bookmark.contains(hashUrl)) {
			bookmark[hashUrl] = new Bookmark(url, data, this);
		}
		QSettings settings;
		if (reload) {
			bool pending = false;
			if (bmMap["size"].toInt() == 0) {
				connect(bookmark[hashUrl], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
				pending = true;
			}
			if (bmMap["favicon"].toString().length() == 0) {
				connect(bookmark[hashUrl], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
				pending = true;
			}
			if (bmMap["image"].toString().length() == 0) {
				connect(bookmark[hashUrl], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
				pending = true;
			}
			if (bmMap["title"].toString().length() == 0) {
				connect(bookmark[hashUrl], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
				pending = true;
			}
			if (pending) {
				bookmark[hashUrl]->fetchContent();
			}
			if (bmMap["pocket_id"].isNull()
					&& !settings.value("pocketUser").isNull()
					&& !url.isEmpty()) {
				pocketPost(url);
			}
		} else {
			if (bmMap["title"].toString().length() == 0) bmMap["title"] = QString("...");
			if (bmMap["favicon"].toString().length() == 0) bmMap["favicon"] = QString("asset:///images/favicon.png");
			if (bmMap["size"].toInt() == 0) bmMap["size"] = QString("");
		}
		bmMap["date"] = bmMap["time"].toDate().toString("yyyy-MM-dd");
		staticList << bmMap;
	}
	list.clear();
	list << staticList;

	GroupDataModel *model = new GroupDataModel(QStringList() << "date" << "time");
	model->insertList(list);
	model->setGrouping(ItemGrouping::ByFullValue);
	model->setSortedAscending(false);
    bookmarks->setDataModel(model);

    takeLoungeBookmark();
    takeOldestBookmark();
    takeQuickestBookmark();
	takeFigures();

    activeFrame->update(true);
    activeFrame->takeFigures(this);

	mainPage->findChild<Tab*>("readTab")->setEnabled(type > 0 || !query.isNull() || list.size() > 0);
	mainPage->findChild<Tab*>("exploreTab")->setEnabled(type > 0 || !query.isNull() || list.size() > 0);
	if (type == 0 && query.isNull() && list.size() == 0)
		mainPage->setActiveTab(mainPage->at(2));
}

void Backpack::saveBackup() {

	QDir appDir;
	if (appDir.currentPath().indexOf("shared/documents/Backpack") < 0
			&& !appDir.setCurrent("shared/documents/Backpack")) {
		appDir.setCurrent("shared/documents");
		appDir.mkdir("Backpack");
		appDir.setCurrent("Backpack");
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

	Page *backupSheet = mainPage->findChild<Page*>("backupSheet");
	ListView *backupsList = backupSheet->findChild<ListView*>("backupsList");

	QDir appDir;
	if (appDir.currentPath().indexOf("shared/documents/Backpack") < 0
			&& !appDir.setCurrent("shared/documents/Backpack")) {
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
	appDir.setCurrent("shared/documents/Backpack");

	InvokeManager invokeSender;
	InvokeRequest sending;
	sending.setAction("bb.action.COMPOSE");
	sending.setMimeType("message/rfc822");
	QFileInfo backupFile(backupFilename);
	QString date = QDate(backupFilename.left(4).toInt(), backupFilename.mid(4, 2).toInt(), backupFilename.mid(6, 2).toInt()).toString();
	date = date.left(1).toUpper() % date.right(date.length() - 1);
	QVariantMap data;
	data["to"] = (QVariantList() << "diegoriveranunez@gmail.com");
	data["subject"] = "Backpack backup file";
	data["attachment"] = (QVariantList() << QString(QUrl(backupFile.absoluteFilePath()).toEncoded()));
	data["body"] = QString("Find attached Backpack backup file dated ").append(date);
	QVariantMap emailData;
	emailData["data"] = data;
	bool ok;
	sending.setData(bb::PpsObject::encode(emailData, &ok));
	invokeSender.invoke(sending);
}

void Backpack::restoreBackup(QString backupFilename) {

	QSettings settings;
	QDir appDir;
	appDir.setCurrent("shared/documents/Backpack");

	XmlDataAccess backup;
	QVariantList bookmarksList;
	QVariant backupContent = backup.load(backupFilename, "/bookmarks/bookmark");
	if (backupContent.canConvert(QVariant::List))
		bookmarksList << backupContent.toList();
	else
		bookmarksList << backupContent.toMap();

	for (int i = 0; i < bookmarksList.size(); i++) {
		QVariantMap backupData = bookmarksList.at(i).toMap();
		if (backupData["url"].isNull()) continue;
		QVariantList existing = data->execute("SELECT id FROM Bookmark WHERE url = ?", QVariantList() << backupData["url"].toString()).toList();
		if (existing.size() == 0) {
			QVariantMap currentId = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
			int id = currentId.value("MAX(id)").isNull() ? 1 : currentId.value("MAX(id)").toInt() + 1;
			data->execute("INSERT INTO Bookmark (id, url, time, keep, memo) VALUES (?, ?, ?, ?, ?)", QVariantList() << id << backupData["url"].toString() << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString());
			if (!settings.value("pocketUser").isNull()) {
				pocketPost(QUrl(backupData["url"].toString()));
//				pocketSetFavourite(backupData["keep"].toBool());
			}
		} else {
			data->execute("UPDATE Bookmark SET time = ?, keep = ?, memo = ? WHERE url = ?", QVariantList() << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString() << backupData["url"].toString());
		}
	}
	refreshBookmarks(true);

	backupToast = new SystemToast();
	QString date = QDate(backupFilename.left(4).toInt(), backupFilename.mid(4, 2).toInt(), backupFilename.mid(6, 2).toInt()).toString();
	date = date.left(1).toUpper() % date.right(date.length() - 1);
	backupToast->setBody(QString::number(bookmarksList.size()).append(" bookmarks restored"));
	SystemUiButton *viewButton = backupToast->button();
	viewButton->setLabel("View");
	timeout = new QTimer();
	timeout->setSingleShot(true);
	timeout->start(7500);
	connect(timeout, SIGNAL(timeout()), backupToast, SLOT(cancel()));
	connect(backupToast, SIGNAL(finished(bb::system::SystemUiResult::Type)), this, SLOT(restoreFinishedFeedback(bb::system::SystemUiResult::Type)));
	backupToast->show();

	QList<Button*> buttons = mainPage->findChildren<Button*>("restoreButton");
	for (int i = 0; i < buttons.size(); i++) {
		buttons.at(i)->setEnabled(true);
	}
}

void Backpack::importBackupFile(QString backupFilePath) {

	QDir appDir;
	if (appDir.currentPath().indexOf("shared/documents/Backpack") < 0
			&& !appDir.setCurrent("shared/documents/Backpack")) {
		appDir.setCurrent("shared/documents");
		appDir.mkdir("Backpack");
		appDir.setCurrent("Backpack");
	}
	QFileInfo backupFile(backupFilePath);
	QFile::copy(backupFilePath, backupFile.fileName());
	showBackups();
}

void Backpack::deleteBackup(QString backupFilename) {

	QDir appDir;
	appDir.setCurrent("shared/documents/Backpack");
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
	connect(timeout, SIGNAL(timeout()), this, SLOT(deleteBackupConfirmation()));
	connect(backupToast, SIGNAL(finished(bb::system::SystemUiResult::Type)), this, SLOT(deleteBackupFeedback(bb::system::SystemUiResult::Type)));
	backupToast->show();
}

void Backpack::restoreFinishedFeedback(bb::system::SystemUiResult::Type value) {

	if (value != SystemUiResult::ButtonSelection)
		return;

	mainPage->setActiveTab(mainPage->at(1));
	mainPage->findChild<Sheet*>("backupSheet")->close();
}

void Backpack::deleteBackupFeedback(bb::system::SystemUiResult::Type value) {

	if (value != SystemUiResult::ButtonSelection)
		return;

	QDir appDir;
	appDir.setCurrent("shared/documents/Backpack");
	QStringList filesList = appDir.entryList();

	for (int i = 0; i < filesList.size(); i++) {
		QString fileName = filesList.value(i);
		if (fileName.indexOf(".tmp") > 0)
			appDir.rename(fileName, fileName.left(fileName.indexOf(".tmp")));
	}
	showBackups();
}

void Backpack::deleteBackupConfirmation() {

	QDir appDir;
	appDir.setCurrent("shared/documents/Backpack");
	QStringList filesList = appDir.entryList();

	for (int i = 0; i < filesList.size(); i++) {
		QString fileName = filesList.value(i);
		if (fileName.indexOf(".tmp") > 0)
			appDir.remove(fileName);
	}
	backupToast->cancel();
}

void Backpack::handleInvoke(const bb::system::InvokeRequest& request) {

	currentUrl = request.uri();
	uint url = cleanUrlHash(currentUrl);
	if (!bookmark.contains(url)) {
		bookmark[url] = new Bookmark(currentUrl, data, this);
		connect(bookmark[url], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[url], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[url], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[url], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
		QSettings settings;
		if (!settings.value("pocketUser").isNull()) {
			pocketPost(currentUrl);
		}
	}
	invokedForm->findChild<QObject*>("invokedURL")->setProperty("text", currentUrl);

	if (bookmark[url]->alreadyExisted()) {
		invokedForm->findChild<QObject*>("status")->setProperty("text", "Bookmark already exists");
		invokedForm->findChild<QObject*>("title")->setProperty("text", bookmark[url]->getTitle());
		invokedForm->findChild<QObject*>("memo")->setProperty("text", bookmark[url]->getMemo());
		invokedForm->findChild<ToggleButton*>("keepCheck")->setProperty("invokeChecked", bookmark[url]->isKept());
		invokedForm->findChild<Container*>("activity")->setVisible(false);
		mainPage->findChild<Sheet*>("bookmarkSheet")->open();
		if (bookmark[url]->getTitle().isNull())
			bookmark[url]->fetchContent();
		return;
	}

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", true);
	invokedForm->findChild<QObject*>("status")->setProperty("text", "Fetching page content...");
	invokedForm->findChild<QObject*>("title")->setProperty("text", "");
	invokedForm->findChild<QObject*>("memo")->setProperty("text", "");
	invokedForm->findChild<TextArea*>("memo")->setText("");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		refreshBookmarks();
		mainPage->findChild<Sheet*>("bookmarkSheet")->open();
	}
	invokedForm->findChild<ToggleButton*>("keepCheck")->setChecked(false);

	bookmark[url]->fetchContent();
}

void Backpack::memoBookmark(QString memo) {

	memoBookmark(currentUrl, memo);
}

void Backpack::memoBookmark(QString url, QString memo) {

	memoBookmark(QUrl(url), memo);
}

void Backpack::memoBookmark(QUrl url, QString memo) {

	uint ulrHash = cleanUrlHash(url);
	if (!bookmark.contains(ulrHash)) {
		bookmark[ulrHash] = new Bookmark(url, data, this);
		connect(bookmark[ulrHash], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[ulrHash], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[ulrHash], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[ulrHash], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	bookmark[ulrHash]->saveMemo(memo);

	if (memo.length() == 0)
		invokedForm->findChild<TextArea*>("memo")->setText(" ");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		if (memo.length() > 0)
			refreshBookmarks();
	} else {
		invokedForm->findChild<ActionItem*>("acceptButton")->setEnabled(false);
		if (memo.length() == 0 || !bookmark[ulrHash]->loading())
			invokedForm->findChild<ActionItem*>("dismissButton")->setTitle("Close");
		if (memo.length() > 0 && !bookmark[ulrHash]->loading())
			invokedForm->findChild<QObject*>("status")->setProperty("text", "Updated!");
	}
}

void Backpack::browseBookmark(QString uri) {

	browseBookmark(uri, "Browse");
}

void Backpack::browseBookmark(QString uri, QString action) {

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(uri);
	invokeSender.invoke(request);

    Flurry::Analytics::LogEvent(action);

    QUrl url(uri);
	if (!getKeepAfterRead() && !bookmark[cleanUrlHash(url)]->isKept()) {
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
	QUrl url = ids.value(randomNumber).toMap().value("url").toUrl();

	removeBookmark(url);

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(url);
	invokeSender.invoke(request);

    Flurry::Analytics::LogEvent("Shuffle");
}

void Backpack::takeOldestBookmark() {

	QSettings settings;
	QVariantMap values;
	QVariantList ids;

	if (settings.value("ignoreKeptOldest").isNull())
		settings.setValue("ignoreKeptOldest", true);

	if (settings.value("ignoreKeptOldest").toBool())
		ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? ORDER BY time LIMIT 1", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark ORDER BY time LIMIT 1").toList();

	values = ids.value(0).toMap();
	if (values.value("time").isNull())
		values["label"] = QDate::currentDate().toString("yyyy-MM-dd");
	else
		values["label"] = values.value("time").toDate().toString("yyyy-MM-dd");

	QUrl url = values.value("url").toUrl();
	uint urlHash = cleanUrlHash(url);
	if (!bookmark.contains(urlHash)) {
		bookmark[urlHash] = new Bookmark(url, data, this);
		connect(bookmark[urlHash], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[urlHash], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[urlHash], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[urlHash], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	mainPage->findChild<Page*>("homePage")->setProperty("oldestItem", QVariant(values));
}

void Backpack::takeQuickestBookmark() {

	QSettings settings;
	QVariantMap values;
	QVariantList ids;

	if (settings.value("ignoreKeptQuickest").isNull())
		settings.setValue("ignoreKeptQuickest", true);

	if (settings.value("ignoreKeptQuickest").toBool()) {
		ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND size IS NOT NULL AND size > 0 AND type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1", QVariantList() << false << Backpack::VIDEOS << Backpack::IMAGES).toList();
		if (ids.size() == 0)
			ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1", QVariantList() << false << Backpack::VIDEOS << Backpack::IMAGES).toList();
	}
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE size IS NOT NULL AND size > 0 AND type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES).toList();
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE type IS NOT ? AND type IS NOT ? ORDER BY size LIMIT 1", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES).toList();

	if (settings.value("ignoreKeptQuickest").toBool()) {
		if (ids.size() == 0)
			ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND size IS NOT NULL AND size > 0 ORDER BY size LIMIT 1", QVariantList() << false).toList();
		if (ids.size() == 0)
			ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? ORDER BY size LIMIT 1", QVariantList() << false).toList();
	}
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE size IS NOT NULL AND size > 0 ORDER BY size LIMIT 1").toList();
	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark ORDER BY size LIMIT 1").toList();

	values = ids.value(0).toMap();
	if (values.value("size").isNull())
		values["label"] = 0;
	else
		values["label"] = values.value("size").toInt();

	QUrl url = values.value("url").toUrl();
	uint urlHash = cleanUrlHash(url);
	if (!bookmark.contains(urlHash)) {
		bookmark[urlHash] = new Bookmark(url, data, this);
		connect(bookmark[urlHash], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[urlHash], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[urlHash], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[urlHash], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	mainPage->findChild<Page*>("homePage")->setProperty("quickestItem", QVariant(values));
}

void Backpack::takeFigures() {

	QVariantMap figures;
	figures["articles"] = data->execute("SELECT count(*) FROM Bookmark WHERE type IS NOT ? AND type IS NOT ?", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES).toList().value(0).toMap().value("count(*)").toInt();
	figures["videos"] = data->execute("SELECT count(*) FROM Bookmark WHERE type = ?", QVariantList() << Backpack::VIDEOS).toList().value(0).toMap().value("count(*)").toInt();
	figures["images"] = data->execute("SELECT count(*) FROM Bookmark WHERE type = ?", QVariantList() << Backpack::IMAGES).toList().value(0).toMap().value("count(*)").toInt();

	mainPage->findChild<Page*>("homePage")->setProperty("figures", QVariant(figures));
}

void Backpack::takeLoungeBookmark() {

	QSettings settings;
	QVariantMap values;
	QVariantList ids;

	if (settings.value("ignoreKeptLounge").isNull())
		settings.setValue("ignoreKeptLounge", true);

	if (settings.value("ignoreKeptLounge").toBool())
		ids = data->execute("SELECT * FROM Bookmark WHERE keep = ? AND type IS NOT ? AND type IS NOT ? ORDER BY size DESC LIMIT 1", QVariantList() << false << Backpack::VIDEOS << Backpack::IMAGES).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark WHERE type IS NOT ? AND type IS NOT ? ORDER BY size DESC LIMIT 1", QVariantList() << Backpack::VIDEOS << Backpack::IMAGES).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT * FROM Bookmark ORDER BY size DESC LIMIT 1").toList();

	values = ids.value(0).toMap();
	if (values.value("size").isNull())
		values["label"] = 0;
	else
		values["label"] = values.value("size").toInt();

	QUrl url = values.value("url").toUrl();
	uint urlHash = cleanUrlHash(url);
	if (!bookmark.contains(urlHash)) {
		bookmark[urlHash] = new Bookmark(url, data, this);
		connect(bookmark[urlHash], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[urlHash], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[urlHash], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[urlHash], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	mainPage->findChild<Page*>("homePage")->setProperty("loungeItem", QVariant(values));
}

void Backpack::updateSize() {

	if (!data->hasError() && iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		refreshBookmarks();
	}
}

void Backpack::updateImage(QUrl image) {

	invokedForm->findChild<ImageView*>("invokedImage")->setImageSource(QString("file://").append(image.toString()));

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::updateFavicon(QUrl favicon) {

	invokedForm->findChild<ImageView*>("invokedFavicon")->setImageSource(QString("file://").append(favicon.toString()));

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::updateTitle(QString title) {

	Label *titleLable = invokedForm->findChild<Label*>("title");
	if (invokedForm->findChild<QObject*>("item") == 0) {
		invokedForm->findChild<Label*>("status")->setText("Added!");
	}
	titleLable->setText(title);

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", false);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::keepBookmark(bool keep) {

	keepBookmark(currentUrl, keep);
}

void Backpack::keepBookmark(QString url, bool keep) {

	keepBookmark(QUrl(url), keep);
}

void Backpack::keepBookmark(QUrl url, bool keep) {

	uint urlHash = cleanUrlHash(url);
	if (!bookmark.contains(urlHash)) {
		bookmark[urlHash] = new Bookmark(url, data, this);
		connect(bookmark[urlHash], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[urlHash], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[urlHash], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[urlHash], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	bookmark[urlHash]->setKept(keep);

	QSettings settings;
	if (!settings.value("pocketUser").isNull())
		pocketSetFavourite(bookmark[urlHash]->getPocketId(), bookmark[urlHash]->isKept());

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::removeBookmark(QString url) {

	removeBookmark(QUrl(url), false);
}

void Backpack::removeBookmark(QString url, bool deliberate) {

	removeBookmark(QUrl(url), deliberate, false);
}

void Backpack::removeBookmark(QUrl url) {

	removeBookmark(url, false);
}

void Backpack::removeBookmark(QUrl url, bool deliberate) {

	removeBookmark(url, deliberate, false);
}

void Backpack::removeBookmark(QUrl url, bool deliberate, bool pocketCleaning) {

	uint urlHash = cleanUrlHash(url);
	if (!bookmark.contains(urlHash)) {
		bookmark[urlHash] = new Bookmark(url, data, this);
		connect(bookmark[urlHash], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[urlHash], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
		connect(bookmark[urlHash], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
		connect(bookmark[urlHash], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}

	if (!bookmark[urlHash]->isKept() || deliberate) {
		QSettings settings;
		if (!pocketCleaning && !settings.value("pocketUser").isNull()) { // If we are cleaning Backpack from Pocket account content, we want Pocket on server to keep its bookmarks
			pocketArchive(bookmark[urlHash]->getPocketId());
		}
		bookmark[urlHash]->remove();
		bookmark.remove(urlHash);

		if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
			if (!deliberate) {
				refreshBookmarks();
			} else {
				if (bookmark.size() == 0) {
					mainPage->setActiveTab(mainPage->at(2));
					mainPage->findChild<Tab*>("readTab")->setEnabled(false);
					mainPage->findChild<Tab*>("exploreTab")->setEnabled(false);
					activeFrame->update(true);
					activeFrame->takeFigures(this);
				} else {
					takeLoungeBookmark();
					takeOldestBookmark();
					takeQuickestBookmark();
					takeFigures();
				}
			}
		}
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

void Backpack::launchRating() {

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.appworld");
	request.setAction("bb.action.OPEN");
	request.setUri("http://appworld.blackberry.com/webstore/content/20399673");
	invokeSender.invoke(request);
}

void Backpack::pocketConnect() {

	QUrl query;
	query.addQueryItem("consumer_key", APIKEY);
	query.addQueryItem("redirect_uri", ACKURL);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/oauth/request"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
	reply = network->post(request, query.encodedQuery());
	connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
}

void Backpack::pocketCleanContent() {

	QList<Bookmark*> bookmarks = bookmark.values();
	for (int i = 0; i < bookmarks.size(); i++) {
		removeBookmark(bookmarks.at(i)->getUrl(), true, true);
	}
}

void Backpack::pocketCompleteAuth() {

	QUrl query;
	query.addQueryItem("consumer_key", APIKEY);
	query.addQueryItem("code", requestToken);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/oauth/authorize"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
	reply = network->post(request, query.encodedQuery());
	connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
}

void Backpack::pocketRetrieve() {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());

	mainPage->findChild<ActivityIndicator*>("syncingActivity")->setRunning(true);

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/get"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	reply = network->post(request, queryArray);
	connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
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
	connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
}

void Backpack::pocketArchive(qlonglong pocketId) {

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
	reply = network->post(request, queryArray);
//	connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
}

void Backpack::pocketSetFavourite(qlonglong pocketId, bool favourite) {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());
	QVariantMap action;
	action["action"] = favourite ? "favorite" : "unfavorite";
	action["item_id"] = pocketId;
	query.insert("actions", QVariantList() << action);

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/send"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	reply = network->post(request, queryArray);
//	connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
}

void Backpack::pocketHandlePostFinished() {

	if (!reply->isFinished())
		return;

	if (reply->request().url().toString().indexOf("v3/oauth/request") > 0) {

		if (reply->rawHeader("Status").isNull() || reply->rawHeader("Status").indexOf("200 OK") != 0) {
			qDebug() << "Pocket error request: " << reply->rawHeader("X-Error");
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
			qDebug() << "Pocket error authorize: " << reply->rawHeader("X-Error");
			mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on authorizing Backpack");
			return;
		}
		QByteArray response = reply->readAll();
		QString username = response.right(response.length() - response.lastIndexOf('=') - 1).replace("%40", "@");
		mainPage->setProperty("username", username);
		mainPage->findChild<Page*>("pocketPage")->setProperty("username", username);

		response = response.left(response.indexOf('&'));
		QString accessToken = response.right(response.length() - response.lastIndexOf('=') - 1);

		QSettings settings;
		settings.setValue("pocketUser", username);
		settings.setValue("pocketToken", accessToken);

		QVariantList existing = data->execute("SELECT url FROM Bookmark").toList();
		for (int i = 0; i < existing.size(); i++) {
			pocketPost(QUrl(existing.at(i).toMap().value("url").toString()));
		}
		pocketRetrieve();

	} else if (reply->request().url().toString().indexOf("v3/add") > 0) {

		if (reply->rawHeader("Status").isNull()
				|| reply->rawHeader("Status").indexOf("200 OK") != 0
				|| reply->error() != QNetworkReply::NoError) {
			qDebug() << "Pocket error add (" << reply->rawHeader("Status") << "): " << reply->error() << " | " << reply->rawHeader("X-Error");
			mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on adding new item. It will be automatically synced when possible");
			return;
		}
		JsonDataAccess json;
		QVariantMap response = json.loadFromBuffer(reply->readAll()).toMap().value("item").toMap();

		QString pocketId = response.value("item_id").toString();
		QString pocketUrl = response.value("normal_url").toString();
		uint url = cleanUrlHash(QUrl(pocketUrl));

		if (!bookmark.contains(url) && pocketUrl.indexOf("s://") > 0) {
			url = cleanUrlHash(QUrl(pocketUrl.replace("s://", "://")));
		}
		if (!bookmark.contains(url) && pocketUrl.indexOf("://") > 0 && pocketUrl.indexOf("s://") < 0) {
			url = cleanUrlHash(QUrl(pocketUrl.replace("://", "s://")));
		}
		if (!bookmark.contains(url) && pocketUrl.indexOf("www") < 0) {
			if (pocketUrl.indexOf("://") > 0) {
				url = cleanUrlHash(QUrl(pocketUrl.left(pocketUrl.indexOf("://")) % QString("://www.") % pocketUrl.right(pocketUrl.length() - pocketUrl.indexOf("://") - 3)));
			} else {
				url = cleanUrlHash(QUrl(QString("www.") % pocketUrl));
			}
		}
		bookmark[url]->pocketSync(pocketId.toLongLong());

	} else if (reply->request().url().toString().indexOf("v3/get") > 0) {

		if (reply->rawHeader("Status").isNull() || reply->rawHeader("Status").indexOf("200 OK") != 0) {
			qDebug() << "Pocket error get: " << reply->rawHeader("X-Error");
			mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on retrieving contents");
			QList<QObject*> pocketSignals = mainPage->findChildren<QObject*>("pocketErrorSignal");
			for (int i = 0; i < pocketSignals.length(); i++) {
				pocketSignals.at(i)->setProperty("visible", true);
			}
			return;
		}
		JsonDataAccess json;
		QVariantMap response = json.loadFromBuffer(reply->readAll()).toMap();

		QVariantList retrieved = response.value("list").toMap().values();
		QString lastSynced = response.value("since").toString();

		QSettings settings;
		settings.setValue("pocketSynced", lastSynced);

		QSet<qlonglong> currentlyAtPocket;
		for (int i = 0; i < retrieved.size(); i++) {
			QUrl url = QUrl(retrieved.value(i).toMap().value("given_url").toString());
			QUrl resolvedUrl = QUrl(retrieved.value(i).toMap().value("resolved_url").toString());
			uint hashUrl = cleanUrlHash(url);
			qlonglong pocketId = retrieved.value(i).toMap().value("item_id").toLongLong();
			currentlyAtPocket << pocketId;
			if (!bookmark.contains(hashUrl)
					&& (resolvedUrl.isEmpty() || !bookmark.contains(cleanUrlHash(resolvedUrl)))) {
				bookmark[hashUrl] = new Bookmark(url, data, this);
				connect(bookmark[hashUrl], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
				connect(bookmark[hashUrl], SIGNAL(imageChanged(QUrl)), this, SLOT(updateImage(QUrl)));
				connect(bookmark[hashUrl], SIGNAL(faviconChanged(QUrl)), this, SLOT(updateFavicon(QUrl)));
				connect(bookmark[hashUrl], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
				QDateTime pocketAdded = QDateTime::fromTime_t(retrieved.value(i).toMap().value("time_added").toInt());
				bookmark[hashUrl]->pocketSync(pocketId, pocketAdded);
				bookmark[hashUrl]->fetchContent();
			}
			if (!retrieved.value(i).toMap().value("favorite").isNull()) {
				bookmark[hashUrl]->setKept(retrieved.value(i).toMap().value("favorite").toBool());
			}
		}
		QList<uint> urls = bookmark.keys();
		for (int i = 0; i < urls.size(); i++) {
			uint url = urls.at(i);
			if (bookmark[url]->getPocketId()
					&& !currentlyAtPocket.contains(bookmark[url]->getPocketId())) {
				bookmark[url]->remove();
			}
		}
		mainPage->findChild<Container*>("syncingIndicator")->setVisible(false);
		QList<QObject*> pocketSignals = mainPage->findChildren<QObject*>("pocketConnSignal");
		for (int i = 0; i < pocketSignals.length(); i++) {
			pocketSignals.at(i)->setProperty("visible", true);
		}
		refreshBookmarks(true);

//	} else if (reply->request().url().toString().indexOf("v3/send") > 0) {
//
//		if (reply->rawHeader("Status").isNull()
//				|| reply->rawHeader("Status").indexOf("200 OK") != 0
//				|| reply->error() != QNetworkReply::NoError) {
//			qDebug() << "Pocket error archive (" << reply->rawHeader("Status") << "): " << reply->error() << " | " << reply->rawHeader("X-Error");
//			mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on archiving item on Pocket");
//			return;
//		}
//		JsonDataAccess json;
//		QVariantMap response = json.loadFromBuffer(reply->readAll()).toMap();
//		QList<QVariant> result = response.value("action_results").toList();
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
	settings.setValue("pocketInterval", interval);
	if (interval > 0) {
		updateTimer->setInterval(interval * 1000 * 60 * 60);
		updateTimer->start();
	} else {
		updateTimer->stop();
	}
}

void Backpack::pocketDisconnect() {

	QSettings settings;
	settings.remove("pocketUser");
	settings.remove("pocketTocken");
	mainPage->setProperty("username", "");
}

Backpack::~Backpack() {

	data->connection().close();
	dbFile.close();
}
