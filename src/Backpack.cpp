
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
#include <bb/cascades/TitleBar>
#include <bb/cascades/ToggleButton>
#include <bb/system/SystemProgressToast>
#include <bb/data/XmlDataAccess>
#include <bb/PpsObject>

using namespace bb::cascades;
using namespace bb::system;
using namespace bb::data;
using namespace bb;
using namespace std;

Backpack::Backpack(bb::cascades::Application *app) : QObject(app) {

	dbFile.setFileName(QDir::home().absoluteFilePath("backpack.db"));
	dbFile.open(QIODevice::ReadWrite);
	data = new SqlDataAccess(dbFile.fileName(), "Backpack", this);

	if (!databaseExists()) createDatabase();

	iManager = new InvokeManager(this);
	connect(iManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

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

		invokedForm = mainPage->findChild<Page*>("invokedForm");
		bookmarks = mainPage->at(1)->content()->findChild<ListView*>("bookmarks");
		refreshBookmarks(true);
	}
}

QString Backpack::getAppVersion() {

	ApplicationInfo thisApp;
	return thisApp.version();
}

bool Backpack::databaseExists() {

	data->execute("SELECT COUNT(*) FROM Bookmark");
	return !data->hasError();
}

void Backpack::createDatabase() {

	data->execute("CREATE TABLE Bookmark (id INTEGER, title VARCHAR(255), url VARCHAR(255), favicon VARCHAR(255), memo VARCHAR(255), date DATE, time DATETIME, size INTEGER, keep BOOL)");
}

void Backpack::setBackgroundColour(float base, float red, float green, float blue) {

	QSettings settings;
	QVariantMap colours;
	colours["base"] = base;
	colours["red"] = red;
	colours["green"] = green;
	colours["blue"] = blue;
	settings.setValue("colour", colours);

	activeFrame->takeBackground(this);
}

float Backpack::getBackgroundColour(QString colour) {

	QSettings settings;
	QVariantMap colours;
	if (settings.value("colour").isNull()) {
		colours["base"] = 1.0;
		colours["red"] = 0.0;
		colours["green"] = 0.0;
		colours["blue"] = 0.0;
		settings.setValue("colour", colours);
	} else {
		colours = settings.value("colour").toMap();
	}
	return colours.value(colour).toFloat();
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

	mainPage->findChild<QObject*>("oldestLabel")->setProperty("text", getOldestDate().toString(Qt::ISODate));
	mainPage->findChild<QObject*>("oldestLabelZip")->setProperty("visible", isKeptOnly());

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

	mainPage->findChild<QObject*>("quickestLabel")->setProperty("text", getQuickestSize());
	mainPage->findChild<QObject*>("quickestLabelZip")->setProperty("visible", isKeptOnly());

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

	mainPage->findChild<QObject*>("loungeLabel")->setProperty("text", getLoungeSize());
	mainPage->findChild<QObject*>("loungeLabelZip")->setProperty("visible", isKeptOnly());

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

void Backpack::refreshBookmarks() {

	refreshBookmarks(false);
}

void Backpack::refreshBookmarks(bool reload) {

	QVariantList list = data->execute("SELECT * FROM Bookmark").toList();

	QVariantList staticList;
	for (QVariantList::const_iterator bm = list.begin(); bm != list.end(); bm++) {
		QVariantMap bmMap = bm->toMap();
		if (reload && (bmMap["title"].toString().length() == 0
				|| bmMap["favicon"].toString().length() == 0
				|| bmMap["size"].toInt() == 0)) {
			QUrl url = QUrl(bmMap["url"].toString());
			if (!bookmark.contains(url)) {
				bookmark[url] = new Bookmark(url, data, this);
				connect(bookmark[url], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
				connect(bookmark[url], SIGNAL(iconChanged()), this, SLOT(updateFavicon()));
				connect(bookmark[url], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
			}
			bookmark[url]->fetchContent();
		} else {
			if (bmMap["title"].toString().length() == 0) bmMap["title"] = QString("...");
			if (bmMap["favicon"].toString().length() == 0) bmMap["favicon"] = QString("asset:///images/favicon.png");
		}
		staticList << bmMap;
	}
	list.clear();
	list << staticList;

	GroupDataModel *model = new GroupDataModel(QStringList() << "date" << "time");
	model->insertList(list);
	model->setGrouping(ItemGrouping::ByFullValue);
	model->setSortedAscending(false);
    bookmarks->setDataModel(model);
    bookmarksNumber = list.size();

	mainPage->findChild<QObject*>("oldestLabel")->setProperty("text", getOldestDate().toString(Qt::ISODate));
	mainPage->findChild<QObject*>("quickestLabel")->setProperty("text", getQuickestSize());
	mainPage->findChild<QObject*>("loungeLabel")->setProperty("text", getLoungeSize());
	mainPage->findChild<QObject*>("oldestLabelZip")->setProperty("visible", isKeptOnly());
	mainPage->findChild<QObject*>("quickestLabelZip")->setProperty("visible", isKeptOnly());
	mainPage->findChild<QObject*>("loungeLabelZip")->setProperty("visible", isKeptOnly());

    activeFrame->update(true);
    activeFrame->takeFigures(this);

	mainPage->findChild<Tab*>("readTab")->setEnabled(bookmarksNumber > 0);
	mainPage->findChild<Tab*>("exploreTab")->setEnabled(bookmarksNumber > 0);
	if (bookmarksNumber == 0)
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
	bookmarksList["bookmark"] = data->execute("SELECT url, memo, time, keep FROM Bookmark").toList();
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

	SystemProgressToast *progress = new SystemProgressToast(this);
	progress->show();

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
		QVariantList existing = data->execute("SELECT id FROM Bookmark WHERE url = ?", QVariantList() << backupData["url"]).toList();
		if (existing.size() == 0) {
			QVariantMap currentId = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
			int id = currentId.value("MAX(id)").isNull() ? 1 : currentId.value("MAX(id)").toInt() + 1;
			data->execute("INSERT INTO Bookmark (id, url, date, time, keep, memo) VALUES (?, ?, ?, ?, ?, ?)", QVariantList() << id << backupData["url"].toString() << backupData["time"].toDate() << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString());
		} else {
			data->execute("UPDATE Bookmark SET date = ?, time = ?, keep = ?, memo = ? WHERE url = ?", QVariantList() << backupData["time"].toDate() << backupData["time"].toDateTime() << backupData["keep"].toBool() << backupData["memo"].toString() << backupData["url"].toString());
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
	if (!bookmark.contains(currentUrl)) {
		bookmark[currentUrl] = new Bookmark(currentUrl, data, this);
		connect(bookmark[currentUrl], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[currentUrl], SIGNAL(iconChanged()), this, SLOT(updateFavicon()));
		connect(bookmark[currentUrl], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}

	if (bookmark[currentUrl]->alreadyExisted()) {
		invokedForm->findChild<QObject*>("status")->setProperty("text", "Bookmark already exists");
		invokedForm->findChild<QObject*>("title")->setProperty("text", bookmark[currentUrl]->getTitle());
		invokedForm->findChild<QObject*>("memo")->setProperty("text", bookmark[currentUrl]->getMemo());
		invokedForm->findChild<ToggleButton*>("keepCheck")->setProperty("invokeChecked", bookmark[currentUrl]->isKept());
		invokedForm->findChild<Container*>("activity")->setVisible(false);
		mainPage->findChild<Sheet*>("bookmarkSheet")->open();
		if (bookmark[currentUrl]->getTitle().isNull())
			bookmark[currentUrl]->fetchContent();
		return;
	}

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", true);
	invokedForm->findChild<QObject*>("status")->setProperty("text", "Fetching page content...");
	invokedForm->findChild<QObject*>("title")->setProperty("text", "");
	invokedForm->findChild<QObject*>("memo")->setProperty("text", "");
	invokedForm->findChild<TextArea*>("memo")->setEnabled(true);
	invokedForm->findChild<TextArea*>("memo")->setText("");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		refreshBookmarks();
		mainPage->findChild<Sheet*>("bookmarkSheet")->open();
	}
	invokedForm->findChild<ToggleButton*>("keepCheck")->setChecked(false);

	bookmark[currentUrl]->fetchContent();
}

void Backpack::memoBookmark(QString memo) {

	memoBookmark(currentUrl.toString(), memo);
}

void Backpack::memoBookmark(QString url, QString memo) {

	QUrl bmUrl = QUrl(url);
	if (!bookmark.contains(bmUrl)) {
		bookmark[bmUrl] = new Bookmark(bmUrl, data, this);
		connect(bookmark[bmUrl], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[bmUrl], SIGNAL(iconChanged()), this, SLOT(updateFavicon()));
		connect(bookmark[bmUrl], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	bookmark[bmUrl]->saveMemo(memo);

	invokedForm->findChild<TextArea*>("memo")->setEnabled(false);
	if (memo.length() == 0)
		invokedForm->findChild<TextArea*>("memo")->setText(" ");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		if (memo.length() > 0)
			refreshBookmarks();
	} else {
		invokedForm->findChild<ActionItem*>("acceptButton")->setEnabled(false);
		if (memo.length() == 0 || !bookmark[bmUrl]->loading())
			invokedForm->findChild<ActionItem*>("dismissButton")->setTitle("Close");
		if (memo.length() > 0 && !bookmark[bmUrl]->loading())
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

    Flurry::Analytics::LogEvent("Browse");
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

	if (bookmark[url] && !bookmark[url]->isKept())
		bookmark[url]->remove();

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(url);
	invokeSender.invoke(request);

    Flurry::Analytics::LogEvent("Shuffle");
}

void Backpack::oldestBookmark() {

	QSettings settings;
	QVariantList ids;

	if (settings.value("ignoreKeptOldest").isNull())
		settings.setValue("ignoreKeptOldest", true);

	if (settings.value("ignoreKeptOldest").toBool())
		ids = data->execute("SELECT url FROM Bookmark WHERE keep = ? ORDER BY time LIMIT 1", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT url FROM Bookmark ORDER BY time LIMIT 1").toList();

	QUrl url = ids.value(0).toMap().value("url").toUrl();

	if (bookmark[url] && !bookmark[url]->isKept())
		bookmark[url]->remove();

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(url);
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Oldest");
}

void Backpack::quickestBookmark() {

	QSettings settings;
	QVariantList ids;

	if (settings.value("ignoreKeptQuickest").isNull())
		settings.setValue("ignoreKeptQuickest", true);

	if (settings.value("ignoreKeptQuickest").toBool()) {
		ids = data->execute("SELECT id, url FROM Bookmark WHERE keep = ? AND size IS NOT NULL AND size > 0 ORDER BY size LIMIT 1", QVariantList() << false).toList();
		if (ids.size() == 0)
			ids = data->execute("SELECT id, url FROM Bookmark WHERE keep = ? ORDER BY size LIMIT 1", QVariantList() << false).toList();
	}

	if (ids.size() == 0)
		ids = data->execute("SELECT id, url FROM Bookmark WHERE size IS NOT NULL AND size > 0 ORDER BY size LIMIT 1").toList();
	if (ids.size() == 0)
		ids = data->execute("SELECT id, url FROM Bookmark ORDER BY size LIMIT 1").toList();

	QUrl url = ids.value(0).toMap().value("url").toUrl();

	if (bookmark[url] && !bookmark[url]->isKept())
		bookmark[url]->remove();

	mainPage->findChild<QObject*>("quickestLabel")->setProperty("text", QString::number(getQuickestSize()));
	mainPage->findChild<QObject*>("quickestLabelZip")->setProperty("visible", isKeptOnly());

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(url);
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Quickest");
}

void Backpack::loungeBookmark() {

	QSettings settings;
	QVariantList ids;

	if (settings.value("ignoreKeptLounge").isNull())
		settings.setValue("ignoreKeptLounge", true);

	if (settings.value("ignoreKeptLounge").toBool())
		ids = data->execute("SELECT url FROM Bookmark WHERE keep = ? ORDER BY size DESC LIMIT 1", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT url FROM Bookmark ORDER BY size DESC LIMIT 1").toList();

	QUrl url = ids.value(0).toMap().value("url").toUrl();

	if (bookmark[url] && !bookmark[url]->isKept())
		bookmark[url]->remove();

	mainPage->findChild<QObject*>("loungeLabel")->setProperty("text", QString::number(getLoungeSize()));
	mainPage->findChild<QObject*>("loungeLabelZip")->setProperty("visible", isKeptOnly());

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(url);
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Lounge");
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

int Backpack::getLoungeSize() {

	QSettings settings;
	if (settings.value("ignoreKeptLounge").isNull())
		settings.setValue("ignoreKeptLounge", true);

	QVariant lounge;

	if (settings.value("ignoreKeptLounge").toBool())
		lounge = data->execute("SELECT MAX(size) FROM Bookmark WHERE keep = ? AND size IS NOT NULL", QVariantList() << false).toList().value(0).toMap().value("MAX(size)");

	if (lounge.isNull())
		lounge = data->execute("SELECT MAX(size) FROM Bookmark WHERE size IS NOT NULL").toList().value(0).toMap().value("MAX(size)");

	if (lounge.isNull())
		return 0;
	else
		return lounge.toInt();
}

bool Backpack::isKeptOnly() {

	return data->execute("SELECT COUNT(*) noKept FROM Bookmark WHERE keep IS NOT ?", QVariantList() << true).toList().value(0).toMap().value("noKept").toInt() == 0;
}

void Backpack::updateSize() {

	if (!data->hasError() && iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		refreshBookmarks();
		mainPage->findChild<QObject*>("quickestLabel")->setProperty("text", QString::number(getQuickestSize()));
		mainPage->findChild<QObject*>("loungeLabel")->setProperty("text", QString::number(getLoungeSize()));
		mainPage->findChild<QObject*>("quickestLabelZip")->setProperty("visible", isKeptOnly());
		mainPage->findChild<QObject*>("loungeLabelZip")->setProperty("visible", isKeptOnly());
	}
}

void Backpack::updateFavicon() {

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::updateTitle(QString title) {

	invokedForm->findChild<QObject*>("title")->setProperty("text", title);

	QObject *status = invokedForm->findChild<QObject*>("status");
	if (!status->property("text").toString().contains("exists"))
		status->setProperty("text", "Added!");

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", false);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::keepBookmark(bool keep) {

	keepBookmark(currentUrl.toString(), keep);
}

void Backpack::keepBookmark(QString url, bool keep) {

	QUrl bmUrl = QUrl(url);
	if (!bookmark.contains(bmUrl)) {
		bookmark[bmUrl] = new Bookmark(bmUrl, data, this);
		connect(bookmark[bmUrl], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[bmUrl], SIGNAL(iconChanged()), this, SLOT(updateFavicon()));
		connect(bookmark[bmUrl], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}
	bookmark[bmUrl]->setKept(keep);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::removeBookmark(QString url) {

	removeBookmark(url, false);
}

void Backpack::removeBookmark(QString url, bool deleteKept) {

	QUrl bmUrl = QUrl(url);
	if (!bookmark.contains(bmUrl)) {
		bookmark[bmUrl] = new Bookmark(bmUrl, data, this);
		connect(bookmark[bmUrl], SIGNAL(sizeChanged()), this, SLOT(updateSize()));
		connect(bookmark[bmUrl], SIGNAL(iconChanged()), this, SLOT(updateFavicon()));
		connect(bookmark[bmUrl], SIGNAL(titleChanged(QString)), this, SLOT(updateTitle(QString)));
	}

	if (!bookmark[bmUrl]->isKept() || deleteKept) {
		bookmark[bmUrl]->remove();
		bookmark.remove(bmUrl);

		if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
			refreshBookmarks();
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

Backpack::~Backpack() {

	data->connection().close();
	dbFile.close();
}
