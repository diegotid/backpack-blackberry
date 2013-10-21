
#include "Backpack.hpp"

#include <Flurry.h>

#include <bb/ApplicationInfo>
#include <bb/cascades/Application>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/GroupDataModel>
#include <bb/cascades/WebDownloadRequest>
#include <bb/cascades/ActivityIndicator>
#include <bb/cascades/ActionItem>
#include <bb/cascades/ImageView>
#include <bb/cascades/ListView>
#include <bb/cascades/TextArea>
#include <bb/cascades/WebPage>
#include <bb/cascades/Sheet>
#include <bb/cascades/Page>
#include <bb/cascades/TitleBar>
#include <bb/cascades/WebSettings>
#include <bb/cascades/ToggleButton>

using namespace bb::cascades;
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

	network = new QNetworkAccessManager(this);
	connect(network, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleBookmarkSize(QNetworkReply*)));

//	bookmark = new WebPage();
//	WebSettings *settings = bookmark->settings();
//	settings->setImageDownloadingEnabled(false);
//	settings->setBinaryFontDownloadingEnabled(false);
//	settings->setCookiesEnabled(false);
//	settings->setJavaScriptEnabled(false);
//	connect(bookmark, SIGNAL(titleChanged(QString)), this, SLOT(handleBookmarkTitle(QString)));

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
		refreshBookmarks();
	}
}

Backpack::~Backpack() {

	data->connection().close();
	dbFile.close();
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

void Backpack::refreshBookmarks() {

	QVariant list = data->execute("SELECT * FROM Bookmark");
	GroupDataModel *model = new GroupDataModel(QStringList() << "date" << "time");
	model->insertList(list.value<QVariantList>());
	model->setGrouping(ItemGrouping::ByFullValue);
	model->setSortedAscending(false);
    bookmarks->setDataModel(model);
    bookmarksNumber = list.toList().size();

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

void Backpack::handleInvoke(const bb::system::InvokeRequest& request) {

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		mainPage->findChild<Sheet*>("bookmarkSheet")->open();

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", true);
	invokedForm->findChild<QObject*>("status")->setProperty("text", "Fetching page content...");
	invokedForm->findChild<QObject*>("title")->setProperty("text", "");
	invokedForm->findChild<QObject*>("memo")->setProperty("text", "");
	invokedForm->findChild<TextArea*>("memo")->setEnabled(true);
	invokedForm->findChild<TextArea*>("memo")->setText("");

	QVariantList urlValues;
	QVariantMap id = data->execute("SELECT id, title, memo, keep FROM Bookmark WHERE url = ?", urlValues << request.uri().toString()).toList().value(0).toMap();

	if (!id.value("id").isNull()) {
		invokedForm->findChild<QObject*>("status")->setProperty("text", "Bookmark already exists");
		invokedForm->findChild<QObject*>("title")->setProperty("text", id.value("title").toString());
		invokedForm->findChild<QObject*>("memo")->setProperty("text", id.value("memo").toString());
		invokedForm->findChild<ToggleButton*>("keepCheck")->setProperty("invokeChecked", id.value("keep").toBool());
		invokedForm->findChild<Container*>("activity")->setVisible(false);
		bookmarkId = id.value("id").toInt();
		return;
	}

	bookmark = new WebPage();
	titleComplete = false;
	faviconComplete = false;
	WebSettings *settings = bookmark->settings();
	settings->setImageDownloadingEnabled(true);
	settings->setBinaryFontDownloadingEnabled(false);
	settings->setCookiesEnabled(false);
	settings->setJavaScriptEnabled(false);
	connect(bookmark, SIGNAL(titleChanged(QString)), this, SLOT(handleBookmarkTitle(QString)));
	connect(bookmark, SIGNAL(iconChanged(QUrl)), this, SLOT(handleBookmarkIcon(QUrl)));

	bookmark->setUrl(request.uri());

	bookmarkRequest = QNetworkRequest();
	bookmarkRequest.setUrl(request.uri());
	network->get(bookmarkRequest);

	id = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
	bookmarkId = id.value("MAX(id)").isNull() ? 1 : id.value("MAX(id)").toInt() + 1;

	invokedForm->findChild<ToggleButton*>("keepCheck")->setChecked(false);

	QVariantList bookmarkValues;
	data->execute("INSERT INTO Bookmark (id, url, keep, date, time) VALUES (?, ?, ?, CURRENT_DATE, CURRENT_TIMESTAMP)", bookmarkValues << bookmarkId << request.uri().toString() << false);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::handleBookmarkSize(QNetworkReply *reply) {

	if (reply->url() != bookmarkRequest.url()) {
		downloadFavicon(reply);
		return;
	}

	QVariantList sizeValues;
	data->execute("UPDATE Bookmark SET size = ? WHERE id = ?", sizeValues << reply->size() << bookmarkId);

	if (!data->hasError() && iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		refreshBookmarks();
		mainPage->findChild<QObject*>("quickestLabel")->setProperty("text", QString::number(getQuickestSize()));
		mainPage->findChild<QObject*>("loungeLabel")->setProperty("text", QString::number(getLoungeSize()));
		mainPage->findChild<QObject*>("quickestLabelZip")->setProperty("visible", isKeptOnly());
		mainPage->findChild<QObject*>("loungeLabelZip")->setProperty("visible", isKeptOnly());
	}
}

void Backpack::handleBookmarkIcon(QUrl icon) {

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

void Backpack::downloadFavicon(QNetworkReply *reply) {

	QFile *iconFile = new QFile(QString("data/icon-") % QString::number(bookmarkId) % QString(".png"));
	iconFile->remove();
	iconFile->open(QIODevice::ReadWrite);
	iconFile->write(reply->readAll());
	iconFile->flush();
	iconFile->close();

	QFileInfo iconInfo(*iconFile);
	data->execute("UPDATE Bookmark SET favicon = ? WHERE id = ?", QVariantList() << QString("file://").append(iconInfo.absoluteFilePath()) << bookmarkId);

	faviconComplete = true;
	if (titleComplete) {
		bookmark->stop();
	}

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::handleBookmarkTitle(QString title) {

	if (title.length() == 0)
		return;

	data->execute("UPDATE Bookmark SET title = ? WHERE id = ?", QVariantList() << title << bookmarkId);
	invokedForm->findChild<QObject*>("title")->setProperty("text", title);
	titleComplete = true;

	if (faviconComplete) {
		bookmark->stop();
	}

	QObject *status = invokedForm->findChild<QObject*>("status");
	if (!status->property("text").toString().contains("exists"))
		status->setProperty("text", "Added!");

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", false);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::memoBookmark(QString memo) {

	memoBookmark(memo, bookmarkId);
}

void Backpack::memoBookmark(QString memo, int id) {

	bool empty = memo.compare("") == 0;

	if (!empty)
		data->execute(QString("UPDATE Bookmark SET memo = ? WHERE id = ?"), QVariantList() << memo << id);

	invokedForm->findChild<TextArea*>("memo")->setEnabled(false);
	if (empty)
		invokedForm->findChild<TextArea*>("memo")->setText(" ");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

		if (!empty) refreshBookmarks();

	} else {

		invokedForm->findChild<ActionItem*>("acceptButton")->setEnabled(false);

		if (empty || !bookmark->loading())
			invokedForm->findChild<ActionItem*>("dismissButton")->setTitle("Close");

		if (!empty && !bookmark->loading())
			invokedForm->findChild<QObject*>("status")->setProperty("text", "Updated!");
	}
}

void Backpack::removeBookmark(int id) {

	removeBookmark(id, false);
}

void Backpack::removeBookmark(int id, bool deleteKeepers) {

	QVariantList idValues;

	if (deleteKeepers)
		data->execute("DELETE FROM Bookmark WHERE id = ?", idValues << id);
	else
		data->execute("DELETE FROM Bookmark WHERE id = ? AND keep = ?", idValues << id << deleteKeepers);

	QFile icon;
	icon.setFileName(QDir::home().absoluteFilePath(QString("icon-") % QString::number(id) % QString(".png")));
	icon.remove();

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
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
		ids = data->execute("SELECT id, url FROM Bookmark WHERE keep = ?", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT id, url FROM Bookmark").toList();

	srand((unsigned)time(0));
	int randomNumber = rand() % ids.size();

	removeBookmark(ids.value(randomNumber).toMap().value("id").toInt());

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(ids.value(randomNumber).toMap().value("url").toString());
	invokeSender.invoke(request);

    Flurry::Analytics::LogEvent("Shuffle");
}

void Backpack::oldestBookmark() {

	QSettings settings;
	QVariantList ids;

	if (settings.value("ignoreKeptOldest").isNull())
		settings.setValue("ignoreKeptOldest", true);

	if (settings.value("ignoreKeptOldest").toBool())
		ids = data->execute("SELECT id, url FROM Bookmark WHERE keep = ? ORDER BY time LIMIT 1", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT id, url FROM Bookmark ORDER BY time LIMIT 1").toList();

	removeBookmark(ids.value(0).toMap().value("id").toInt());

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(ids.value(0).toMap().value("url").toString());
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Oldest");
}

void Backpack::quickestBookmark() {

	QSettings settings;
	QVariantList ids;

	if (settings.value("ignoreKeptQuickest").isNull())
		settings.setValue("ignoreKeptQuickest", true);

	if (settings.value("ignoreKeptQuickest").toBool())
		ids = data->execute("SELECT id, url FROM Bookmark WHERE keep = ? ORDER BY size LIMIT 1", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT id, url FROM Bookmark ORDER BY size LIMIT 1").toList();

	removeBookmark(ids.value(0).toMap().value("id").toInt());

	mainPage->findChild<QObject*>("quickestLabel")->setProperty("text", QString::number(getQuickestSize()));
	mainPage->findChild<QObject*>("quickestLabelZip")->setProperty("visible", isKeptOnly());

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(ids.value(0).toMap().value("url").toString());
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Quickest");
}

void Backpack::loungeBookmark() {

	QSettings settings;
	QVariantList ids;

	if (settings.value("ignoreKeptLounge").isNull())
		settings.setValue("ignoreKeptLounge", true);

	if (settings.value("ignoreKeptLounge").toBool())
		ids = data->execute("SELECT id, url FROM Bookmark WHERE keep = ? ORDER BY size DESC LIMIT 1", QVariantList() << false).toList();

	if (ids.size() == 0)
		ids = data->execute("SELECT id, url FROM Bookmark ORDER BY size DESC LIMIT 1").toList();

	removeBookmark(ids.value(0).toMap().value("id").toInt());

	mainPage->findChild<QObject*>("loungeLabel")->setProperty("text", QString::number(getLoungeSize()));
	mainPage->findChild<QObject*>("loungeLabelZip")->setProperty("visible", isKeptOnly());

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(ids.value(0).toMap().value("url").toString());
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Lounge");
}

QDate Backpack::getOldestDate() {

	QSettings settings;
	if (settings.value("ignoreKeptOldest").isNull())
		settings.setValue("ignoreKeptOldest", true);

	QVariant oldest;

	if (settings.value("ignoreKeptOldest").toBool())
		oldest = data->execute("SELECT MIN(date) FROM Bookmark WHERE keep = ? AND date IS NOT NULL", QVariantList() << false).toList().value(0).toMap().value("MIN(date)");

	if (oldest.isNull())
		oldest = data->execute("SELECT MIN(date) FROM Bookmark WHERE date IS NOT NULL").toList().value(0).toMap().value("MIN(date)");

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

	if (settings.value("ignoreKeptQuickest").toBool())
		quickest = data->execute("SELECT MIN(size) FROM Bookmark WHERE keep = ? AND size IS NOT NULL", QVariantList() << false).toList().value(0).toMap().value("MIN(size)");

	if (quickest.isNull())
		quickest = data->execute("SELECT MIN(size) FROM Bookmark WHERE size IS NOT NULL").toList().value(0).toMap().value("MIN(size)");

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

void Backpack::keepBookmark(bool keep) {

	keepBookmark(keep, bookmarkId);
}

void Backpack::keepBookmark(bool keep, int id) {

	QVariantList keepValues;
	data->execute(QString("UPDATE Bookmark SET keep = ? WHERE id = ?"), keepValues << keep << id);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
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
