
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

#include <bb/cascades/core/developmentsupport.h>

using namespace bb::cascades;
using namespace bb::system;
using namespace bb::data;
using namespace bb;
using namespace std;

#define HOST "getpocket.com"
#define APIKEY "22109-9f0af838570499419e7b5886"
#define ACKURL "http://bbornot2b.com/backpack/auth"
#define FRAMEINTERVAL 10

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
	bool res_toast_act = connect(iManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);

	network = new QNetworkAccessManager(this);

    pocketUpdateTimer = new QTimer();
    pocketUpdateTimer->setSingleShot(false);
    bool updaterConnected = connect(pocketUpdateTimer, SIGNAL(timeout()), this, SLOT(pocketRetrieve()));
    Q_ASSERT(updaterConnected);
    Q_UNUSED(updaterConnected);

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

	    bookmarksByURL = new GroupDataModel(QStringList() << "url");
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

void Backpack::logEvent(QString mode) {

    Flurry::Analytics::LogEvent(mode);
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
    data->execute("ALTER TABLE Bookmark ADD hash_url VARCHAR(255)"); // Added on 2.0.1 for Pocket integration
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

	QVariantList alreadyAdded;
	QVariantList staticList;
	for (int i = 0; i < list.size(); i++) {
		QVariantMap bmMap = list.at(i).toMap();
		if (bmMap["url"].isNull()
		|| alreadyAdded.contains(bmMap["url"])) {
		    continue;
		}
        if (bmMap["title"].toString().length() == 0) {
            bmMap["title"] = QString("...");
            uint urlHash = Bookmark::cleanUrlHash(QUrl(bmMap["url"].toString()));
            loading[urlHash] = new Bookmark(QUrl(bmMap["url"].toString()), data, this);
            loading[urlHash]->fetchContent();
            bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
            Q_ASSERT(res_loading_end);
            Q_UNUSED(res_loading_end);
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
    bookmarks->setProperty("size", list.size());

    updateActiveFrame(true);

	if (type == 0 && query.isNull() && list.size() == 0) {
		mainPage->setActiveTab(mainPage->findChild<Tab*>("putinTab"));
	}
    mainPage->findChild<Label*>("emptyHint")->setVisible(type == 0 && query.isNull() && list.size() == 0);

    Tab *exploreTab = mainPage->findChild<Tab*>("exploreTab");
    bool prevExploreTab = exploreTab->isEnabled();
    exploreTab->setEnabled(type > 0 || !query.isNull() || list.size() > 0);
    if (!prevExploreTab && exploreTab->isEnabled()) {
        mainPage->setActiveTab(exploreTab);
    }
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

	Page *backupSheet = mainPage->findChild<Page*>("backupPage");
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
		if (backupData["url"].isNull())
			continue;
		QUrl url(backupData["url"].toString());
		uint urlHash = Bookmark::cleanUrlHash(url);
        if (urlHash == 0)
            continue;
		QVariantList existing = data->execute("SELECT id FROM Bookmark WHERE hash_url = ?", QVariantList() << urlHash).toList();
		if (existing.size() == 0) {
			QVariantMap currentId = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
			int id = currentId.value("MAX(id)").isNull() ? 1 : currentId.value("MAX(id)").toInt() + 1;
			data->execute("INSERT INTO Bookmark (id, url, hash_url, time, keep, memo) VALUES (?, ?, ?, ?, ?, ?)", QVariantList() << id << url.toString() << urlHash << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString());
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
        appDir.setCurrent("shared/documents/Backpack");
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
	appDir.setCurrent("shared/documents/Backpack");
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

void Backpack::handleInvoke(const bb::system::InvokeRequest& request) {

    invokedForm->setProperty("item", NULL);
    invokedForm->findChild<QObject*>("invokedURL")->setProperty("text", request.uri().toString());

    QVariantList bookmarks = data->execute("SELECT * FROM Bookmark WHERE hash_url = ?", QVariantList() << Bookmark::cleanUrlHash(request.uri())).toList();

    if (bookmarks.size() > 0) {
        QVariantMap bookmarkContent = bookmarks.value(0).toMap();
        invokedForm->titleBar()->setTitle("Edit item");
        invokedForm->findChild<QObject*>("status")->setProperty("text", "Article is already in your Backpack");
        invokedForm->findChild<QObject*>("title")->setProperty("text", bookmarkContent["title"]);
        invokedForm->findChild<QObject*>("memo")->setProperty("text", bookmarkContent["memo"]);
        invokedForm->findChild<QObject*>("memo")->setProperty("enabled", false);
        invokedForm->findChild<TextArea*>("memo")->setVisible(bookmarkContent["memo"].toString().trimmed().length() > 0);
        invokedForm->findChild<Container*>("toggleFav")->setVisible(false);
        invokedForm->findChild<ImageView*>("invokedImage")->setImageSource(QString("file://").append(bookmarkContent["image"].toString()));
        invokedForm->findChild<ToggleButton*>("keepCheck")->setProperty("invokeChecked", bookmarkContent["keep"]);
        invokedForm->findChild<Container*>("activity")->setVisible(false);
        if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
            mainPage->findChild<Sheet*>("bookmarkSheet")->open();
        }
        return;
    }
    invokedForm->findChild<ImageView*>("invokedImage")->setImageSource(QString("asset:///images/backpack.png"));

    logEvent("Add");

    uint urlHash = Bookmark::cleanUrlHash(request.uri());
    loading[urlHash] = new Bookmark(request.uri(), data, this);
    loading[urlHash]->fetchContent();
    bool res_loading_end = connect(loading[urlHash], SIGNAL(downloadComplete(uint)), this, SLOT(freeLoadingPage(uint)));
    Q_ASSERT(res_loading_end);
    Q_UNUSED(res_loading_end);

    QSettings settings;
    if (!settings.value("pocketUser").isNull()) {
        pocketPost(request.uri());
    }

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", true);
	invokedForm->findChild<QObject*>("status")->setProperty("text", "Fetching page content...");
	invokedForm->findChild<QObject*>("title")->setProperty("text", "");
	invokedForm->findChild<QObject*>("memo")->setProperty("text", "");
	invokedForm->findChild<TextArea*>("memo")->setText("");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

	    QVariantMap newMap;
        newMap["url"] = request.uri().toString();
        newMap["hash_url"] = Bookmark::cleanUrlHash(request.uri());
        newMap["time"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        newMap["date"] = newMap["time"].toDate().toString("yyyy-MM-dd");
        newMap["keep"] = 0;
        bookmarksByURL->insert(newMap);
        bookmarksByDate->insert(newMap);
        mainPage->findChild<Sheet*>("bookmarkSheet")->open();

        if (bookmarksByURL->size() > 0) {
            updateActiveFrame(true);
            mainPage->findChild<Label*>("emptyHint")->setVisible(false);
            mainPage->findChild<Tab*>("exploreTab")->setEnabled(true);
            mainPage->setActiveTab(mainPage->findChild<Tab*>("exploreTab"));
        }
	}

	invokedForm->findChild<ToggleButton*>("keepCheck")->setChecked(false);
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
    queryMap["url"] = url;
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
		if (memo.length() > 0) {
		    QVariantMap queryMap;
		    queryMap["url"] = url.toString();
		    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
            QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
            QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
            bookmarkContent["memo"] = memo;
            bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
            bookmarksByDate->updateItem(indexPath, bookmarkContent);
		}
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

    QVariantMap queryMap;
    queryMap["url"] = uri;
    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
	if (!getKeepAfterRead() && !bookmarkContent["keep"].toBool()) {
		removeBookmark(QUrl(uri));
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

    QVariantMap queryMap;
    queryMap["url"] = url.toString();
    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
    if (!getKeepAfterRead() && !bookmarkContent["keep"].toBool()) {
        removeBookmark(url);
    }

    InvokeManager invokeSender;
    InvokeRequest request;
    request.setTarget("sys.browser");
    request.setAction("bb.action.OPEN");
    request.setUri(url);
    invokeSender.invoke(request);
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
        queryMap["url"] = page.toString();
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        bookmarkContent["size"] = size;
        bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
        bookmarksByDate->updateItem(indexPath, bookmarkContent);
	}
}

void Backpack::updateImage(QUrl page, QUrl image) {

    invokedForm->findChild<ImageView*>("invokedImage")->setImageSource(QString("file://").append(image.toString()));

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["url"] = page.toString();
        QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
        QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
        QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
        bookmarkContent["image"] = image.toString();
        bookmarksByURL->updateItem(indexPathByURL, bookmarkContent);
        bookmarksByDate->updateItem(indexPath, bookmarkContent);
	}
}

void Backpack::updateImage(const char *item, QUrl image) {

	Page *homePage = mainPage->findChild<Page*>("homePage");
	QVariantMap itemValues = homePage->property(item).toMap();
	itemValues["image"] = image.toString();
	homePage->setProperty(item, QVariant(itemValues));
}

void Backpack::updateFavicon(QUrl page, QUrl favicon) {

    invokedForm->findChild<ImageView*>("invokedFavicon")->setImageSource(QString("file://").append(favicon.toString()));
    invokedForm->findChild<ImageView*>("invokedFavicon")->setVisible(true);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["url"] = page.toString();
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
        queryMap["url"] = page.toString();
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
	    reply = network->post(request, queryArray);
	}

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

	    // Update fav indicator on list
        QVariantMap queryMap;
        queryMap["url"] = url.toString();
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

	removeBookmark(QUrl(url), false);
}

void Backpack::removeBookmark(QString url, bool deliberate) {

	removeBookmark(QUrl(url), deliberate);
}

void Backpack::removeBookmark(QUrl url) {

	removeBookmark(url, false);
}

void Backpack::removeBookmark(QUrl url, bool deliberate) {

    QVariantMap queryMap;
    queryMap["url"] = url.toString();
    QVariantList indexPathByURL = bookmarksByURL->find(queryMap);
    QVariantMap bookmarkContent = bookmarksByURL->data(indexPathByURL).toMap();
    QVariantList indexPath = bookmarksByDate->findExact(bookmarkContent);
    bookmarksByURL->removeAt(indexPath);
    bookmarksByDate->removeAt(indexPath);

    QSettings settings;
    pocketArchiveDelete(Bookmark::getPocketId(data, url), deliberate && getPocketDeleteMode() > 0);

    uint urlHash = Bookmark::cleanUrlHash(url);
    if (loading.contains(urlHash)) {
        loading[urlHash]->remove();
        loading.remove(urlHash);
    } else {
        Bookmark::remove(data, url);
    }

    if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
        QVariantMap queryMap;
        queryMap["url"] = url.toString();
        bookmarksByURL->remove(queryMap);
        if (bookmarksByURL->size() == 0) {
            updateActiveFrame(true);
            mainPage->setActiveTab(mainPage->findChild<Tab*>("putinTab"));
            mainPage->findChild<Label*>("emptyHint")->setVisible(true);
            mainPage->findChild<Tab*>("exploreTab")->setEnabled(false);
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

void Backpack::pocketConnect() {

	QUrl query;
	query.addQueryItem("consumer_key", APIKEY);
	query.addQueryItem("redirect_uri", ACKURL);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/oauth/request"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
	reply = network->post(request, query.encodedQuery());
	bool res_toast_act = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);
}

void Backpack::pocketCleanContent() {

    QList<uint> hashses = loading.keys();

    for (int i = 0; i < hashses.size(); i++) {
        loading[hashses.at(i)]->remove();
    }
    loading.clear();
    bookmarksByURL->clear();
    bookmarksByDate->clear();

    data->execute("DELETE FROM Bookmark");

    mainPage->setActiveTab(mainPage->findChild<Tab*>("putinTab"));
    mainPage->findChild<Tab*>("exploreTab")->setEnabled(false);

    updateActiveFrame(true);
}

void Backpack::pocketCompleteAuth() {

	QUrl query;
	query.addQueryItem("consumer_key", APIKEY);
	query.addQueryItem("code", requestToken);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/oauth/authorize"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded; charset=UTF-8");
	reply = network->post(request, query.encodedQuery());
	bool res_toast_act = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);
}

void Backpack::pocketRetrieve() {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());

	if (!settings.value("pocketSynced").isNull()) {
	    QDateTime debugSynced;
	    debugSynced.setTime_t(settings.value("pocketSynced").toUInt());
	    query.insert("since", debugSynced.toTime_t());
	}

	mainPage->findChild<ActivityIndicator*>("syncingActivity")->setRunning(true);

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/get"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	reply = network->post(request, queryArray);
	bool res_toast_act = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);
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
	bool res_toast_act = connect(reply, SIGNAL(finished()), this, SLOT(pocketHandlePostFinished()));
    Q_ASSERT(res_toast_act);
    Q_UNUSED(res_toast_act);
}

void Backpack::pocketArchiveDelete(qlonglong pocketId, bool permanent) {

	QSettings settings;
	QVariantMap query;
	query.insert("consumer_key", APIKEY);
	query.insert("access_token", settings.value("pocketToken").toString());
	QVariantMap action;
	action["action"] = permanent ? "delete" : "archive";
	action["item_id"] = pocketId;
	query.insert("actions", QVariantList() << action);

	QByteArray queryArray;
	JsonDataAccess json;
	json.saveToBuffer(query, &queryArray);

	QNetworkRequest request(QUrl(QString("https://") % HOST % "/v3/send"));
	request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json; charset=UTF-8");
	reply = network->post(request, queryArray);
}

void Backpack::pocketHandlePostFinished() {

	if (!reply->isFinished())
		return;

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
			mainPage->findChild<Page*>("pocketPage")->setProperty("error", "Error on adding new item. It will be automatically synced when possible");
			return;
		}
		JsonDataAccess json;
		QVariantMap response = json.loadFromBuffer(reply->readAll()).toMap().value("item").toMap();

		qlonglong pocketId = response.value("item_id").toLongLong();
		QString pocketUrl = response.value("normal_url").toString();
		QString url = Bookmark::cleanUrl(QUrl(pocketUrl));

		int exists = data->execute("SELECT COUNT(*) number FROM Bookmark WHERE url = ?", QVariantList() << url).toList().value(0).toMap().value("number").toInt();
		if (exists < 1 && pocketUrl.indexOf("s://") > 0) {
			url = Bookmark::cleanUrl(QUrl(pocketUrl.replace("s://", "://")));

			exists = data->execute("SELECT COUNT(*) number FROM Bookmark WHERE url = ?", QVariantList() << url).toList().value(0).toMap().value("number").toInt();
			if (exists < 1 && pocketUrl.indexOf("://") > 0 && pocketUrl.indexOf("s://") < 0) {
			    url = Bookmark::cleanUrl(QUrl(pocketUrl.replace("://", "s://")));

			    exists = data->execute("SELECT COUNT(*) number FROM Bookmark WHERE url = ?", QVariantList() << url).toList().value(0).toMap().value("number").toInt();
			    if (exists < 1 && pocketUrl.indexOf("www") < 0) {
			        if (pocketUrl.indexOf("://") > 0) {
			            url = Bookmark::cleanUrl(QUrl(pocketUrl.left(pocketUrl.indexOf("://")) % QString("://www.") % pocketUrl.right(pocketUrl.length() - pocketUrl.indexOf("://") - 3)));
			        } else {
			            url = Bookmark::cleanUrl(QUrl(QString("www.") % pocketUrl));
			        }
			    }
            }
        }
		data->execute("UPDATE Bookmark SET pocket_id = ? WHERE url = ?", QVariantList() << pocketId << url);

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

		QVariantList retrieved = response.value("list").toMap().values();
		QString lastSynced = response.value("since").toString();

		QSettings settings;
		settings.setValue("pocketSynced", lastSynced);

		QDateTime debugSynced;
		debugSynced.setTime_t(lastSynced.toUInt());

        QVariantList toInsert;
        QVariantList toUpdate;
        QVariantList toDelete;

		for (int i = 0; i < retrieved.size(); i++) {
			QVariantMap item = retrieved.value(i).toMap();
			QUrl url = QUrl(item.value("resolved_url").toString());
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
                    int size = 10000 * item.value("word_count").toInt() / 180; // Average 180 words per minute on a monitor; Assumed 10000 bytes per minute
                    QString title = item.value("resolved_title").toString();
                    bool favorited = item.value("favorite").toBool();
                    QDateTime pocketAdded = QDateTime::fromTime_t(item.value("time_added").toInt());
                    if (1 > data->execute("SELECT COUNT(*) number FROM Bookmark WHERE hash_url = ?", QVariantList() << urlHash).toList().value(0).toMap().value("number").toInt()) {
                        toInsert << QVariant::fromValue(QVariantList() << url.toString() << urlHash << title << size << favorited << pocketId << pocketAdded);
                    } else {
                        toUpdate << QVariant::fromValue(QVariantList() << pocketId << size << favorited << pocketAdded << urlHash);
                    }
                    break;
			}
		}
        data->executeBatch("INSERT INTO Bookmark (url, hash_url, title, size, keep, pocket_id, time) VALUES (?, ?, ?, ?, ?, ?, ?)", toInsert);
        data->executeBatch("UPDATE Bookmark SET pocket_id = ?, size = ?, keep = ?, time = ? WHERE hash_url = ?", toUpdate);
        data->executeBatch("DELETE FROM Bookmark WHERE hash_url = ?", toDelete);

		mainPage->findChild<Container*>("syncingIndicator")->setVisible(false);
		QList<QObject*> pocketSignals = mainPage->findChildren<QObject*>("pocketConnSignal");
		for (int i = 0; i < pocketSignals.length(); i++) {
			pocketSignals.at(i)->setProperty("visible", true);
		}

		refreshBookmarks();
	    updateActiveFrame(true);
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

Backpack::~Backpack() {

	data->connection().close();
	dbFile.close();
}
