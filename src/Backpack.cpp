
#include "Backpack.hpp"

#include <Flurry.h>

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
#include <bb/cascades/Page>
#include <bb/cascades/TitleBar>
#include <bb/cascades/WebSettings>
#include <bb/cascades/ToggleButton>

using namespace bb::cascades;
using namespace bb::data;
using namespace std;

Backpack::Backpack(bb::cascades::Application *app) : QObject(app) {

	QDir homeDir = QDir::home();
	dbFile.setFileName(homeDir.absoluteFilePath("backpack.db"));
	dbFile.open(QIODevice::ReadWrite);
	data = new SqlDataAccess(dbFile.fileName(), "Backpack", this);

	if (!databaseExists()) createDatabase();

    homeQml = QmlDocument::create("asset:///HomePage.qml").parent(this);
    homeQml->setContextProperty("app", this);

	invokedQml = QmlDocument::create("asset:///InvokedForm.qml").parent(this);
    invokedQml->setContextProperty("app", this);
	invokedForm = invokedQml->createRootObject<Page>();

	iManager = new InvokeManager(this);
	connect(iManager, SIGNAL(invoked(const bb::system::InvokeRequest&)), this, SLOT(handleInvoke(const bb::system::InvokeRequest&)));

	network = new QNetworkAccessManager(this);
	connect(network, SIGNAL(finished(QNetworkReply*)), this, SLOT(handleBookmarkSize(QNetworkReply*)));

	bookmark = new WebPage();
	WebSettings *settings = bookmark->settings();
	settings->setImageDownloadingEnabled(false);
	settings->setBinaryFontDownloadingEnabled(false);
	settings->setCookiesEnabled(false);
	settings->setJavaScriptEnabled(false);
	connect(bookmark, SIGNAL(titleChanged(QString)), this, SLOT(handleBookmarkTitle(QString)));

	if (iManager->startupMode() == ApplicationStartupMode::InvokeApplication) {

		app->setScene(invokedForm);

	} else {

	    homePage = homeQml->createRootObject<NavigationPane>();
		bookmarks = homePage->findChild<ListView*>("bookmarks");

		activeFrame = (ActiveFrame*)app->cover();

		refreshBookmarks();

	    app->setScene(homePage);

		homePage->findChild<QObject*>("buttons")->setProperty("visible", bookmarksNumber > 0);
		homePage->findChild<QObject*>("hint")->setProperty("visible", bookmarksNumber == 0);
	}
}

Backpack::~Backpack() {

	data->connection().close();
	dbFile.close();
}

bool Backpack::databaseExists() {

	data->execute("SELECT COUNT(*) FROM Bookmark");
	return !data->hasError();
}


void Backpack::createDatabase() {

	data->execute("CREATE TABLE Bookmark (id INTEGER, title VARCHAR(255), url VARCHAR(255), favicon VARCHAR(255), memo VARCHAR(255), date DATE, time DATETIME, size INTEGER, keep BOOL)");
	if (data->hasError())
		homePage->findChild<QObject*>("hint")->setProperty("text", data->error().errorMessage());
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

void Backpack::refreshBookmarks() {

	QVariant list = data->execute("SELECT * FROM Bookmark");
	GroupDataModel *model = new GroupDataModel(QStringList() << "date" << "time");
	model->insertList(list.value<QVariantList>());
	model->setGrouping(ItemGrouping::ByFullValue);
	model->setSortedAscending(false);
    bookmarks->setDataModel(model);
    bookmarksNumber = list.toList().size();

    activeFrame->update(true);
    activeFrame->takeFigures(this);
}

int Backpack::getSize() {

	return bookmarksNumber;
}

void Backpack::handleInvoke(const bb::system::InvokeRequest& request) {

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

		homePage->setBackButtonsVisible(false);
		homePage->push(invokedForm);
//      debugStack();
	}

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
	WebSettings *settings = bookmark->settings();
	settings->setImageDownloadingEnabled(false);
	settings->setBinaryFontDownloadingEnabled(false);
	settings->setCookiesEnabled(false);
	settings->setJavaScriptEnabled(false);
	connect(bookmark, SIGNAL(titleChanged(QString)), this, SLOT(handleBookmarkTitle(QString)));

	bookmark->setUrl(request.uri());

	bookmarkRequest = QNetworkRequest();
	bookmarkRequest.setUrl(request.uri());

	network->get(bookmarkRequest);

	id = data->execute("SELECT MAX(id) FROM Bookmark").toList().value(0).toMap();
	bookmarkId = id.value("MAX(id)").isNull() ? 1 : id.value("MAX(id)").toInt() + 1;

	invokedForm->findChild<ToggleButton*>("keepCheck")->setChecked(false);

	QVariantList bookmarkValues;
	data->execute("INSERT INTO Bookmark (id, url, keep, date, time) VALUES (?, ?, ?, CURRENT_DATE, CURRENT_TIMESTAMP)", bookmarkValues << bookmarkId << request.uri().toString() << false);

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

		refreshBookmarks();

		homePage->findChild<QObject*>("buttons")->setProperty("visible", true);
		homePage->findChild<QObject*>("hint")->setProperty("visible", false);
	}
}

void Backpack::handleBookmarkSize(QNetworkReply *reply) {

//	invokedForm->findChild<QObject*>("status")->setProperty("text", QString("Tamaño: ").append(QString::number(reply->size())));

	QVariantList sizeValues;
	data->execute("UPDATE Bookmark SET size = ? WHERE id = ?", sizeValues << reply->size() << bookmarkId);

	if (data->hasError())
		invokedForm->findChild<QObject*>("status")->setProperty("text", data->error().errorMessage());

	else if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {
		refreshBookmarks();
		homePage->findChild<QObject*>("quickestLabel")->setProperty("text", QString::number(getQuickestSize()));
	}

	reply->deleteLater();
}

void Backpack::handleBookmarkIcon(QUrl icon) {

	if (!bookmark->title().isEmpty()) {
		bookmark->stop();
		bookmark->deleteLater();
	}

	QString iconFile = QString("icon-").append(QString::number(bookmarkId));
//	WebDownloadRequest *downloadIcon = new WebDownloadRequest(icon);
//	downloadIcon->setAbsoluteFilePath(homeDir.absoluteFilePath(iconFile));
//	bookmark->download(downloadIcon);

	QVariantList iconValues;
	data->execute("UPDATE Bookmark SET favicon = ? WHERE id = ?", iconValues << iconFile << bookmarkId);
}

void Backpack::handleBookmarkTitle(QString title) {

//	if (!bookmark->icon().isEmpty())
	bookmark->stop();
	bookmark->deleteLater();

	if (bookmark->loadProgress() > 0) {
		QVariantList titleValues;
		data->execute("UPDATE Bookmark SET title = ? WHERE id = ?", titleValues << title << bookmarkId);
		invokedForm->findChild<QObject*>("title")->setProperty("text", title);
	} else {
		invokedForm->findChild<QObject*>("status")->setProperty("text", title);
	}

	invokedForm->findChild<QObject*>("activity")->setProperty("visible", false);

	QObject *status = invokedForm->findChild<QObject*>("status");
	if (!status->property("text").toString().contains("exists"))
		status->setProperty("text", "Added!");

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication)
		refreshBookmarks();
}

void Backpack::memoBookmark(QString memo) {

	memoBookmark(memo, bookmarkId);
}

void Backpack::memoBookmark(QString memo, int id) {

	bool empty = memo.compare("") == 0;

	if (!empty) {
		QVariantList memoValues;
		data->execute(QString("UPDATE Bookmark SET memo = ? WHERE id = ?"), memoValues << memo << id);
	}

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

	if (iManager->startupMode() == ApplicationStartupMode::LaunchApplication) {

		refreshBookmarks();

		if (bookmarksNumber == 0) {
			homePage->findChild<QObject*>("buttons")->setProperty("visible", false);
			homePage->findChild<QObject*>("hint")->setProperty("visible", true);
		}
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

	QVariantList ids = data->execute("SELECT id, url FROM Bookmark").toList();

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

	QVariantList ids = data->execute("SELECT id, url FROM Bookmark ORDER BY time LIMIT 1").toList();

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

	QVariantList ids = data->execute("SELECT id, url FROM Bookmark ORDER BY size LIMIT 1").toList();

	removeBookmark(ids.value(0).toMap().value("id").toInt());

	homePage->findChild<QObject*>("quickestLabel")->setProperty("text", QString::number(getQuickestSize()));

	InvokeManager invokeSender;
	InvokeRequest request;
	request.setTarget("sys.browser");
	request.setAction("bb.action.OPEN");
	request.setUri(ids.value(0).toMap().value("url").toString());
	invokeSender.invoke(request);

	Flurry::Analytics::LogEvent("Quickest");
}

QDate Backpack::getOldestDate() {

	return data->execute("SELECT MIN(date) FROM Bookmark WHERE date IS NOT NULL").toList().value(0).toMap().value("MIN(date)").toDate();
}

int Backpack::getQuickestSize() {

	return data->execute("SELECT MIN(size) FROM Bookmark WHERE size IS NOT NULL").toList().value(0).toMap().value("MIN(size)").toInt();
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

void Backpack::debugStack() {

	if (iManager->startupMode() == ApplicationStartupMode::InvokeApplication) {

		qDebug() << "****** App invoked ******";

	} else {

		qDebug() << "";
		qDebug() << "";
		qDebug() << "****** Stack size: " << homePage->count() << " ******";

		for (int i = 0; i < homePage->count(); i++) {

			qDebug() << homePage->at(i)->objectName();
		}
		qDebug() << "";
		qDebug() << "";
		qDebug() << "";
	}
}
