
#ifndef Backpack_HPP_
#define Backpack_HPP_

#include "Bookmark.hpp"
#include "ActiveFrame.hpp"

#include <bb/cascades/QmlDocument>
#include <bb/cascades/TabbedPane>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/ListView>
#include <bb/cascades/Page>
#include <bb/cascades/Sheet>
#include <bb/cascades/WebPage>
#include <bb/cascades/WebLoadRequest>
#include <bb/cascades/WebPageCompositor>
#include <bb/data/SqlDataAccess>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>
#include <bb/system/SystemUiResult>
#include <bb/system/SystemToast>

using namespace bb::cascades;
using namespace bb::system;
using namespace bb::data;

namespace bb {
	namespace cascades {
		class Application;
	}
}

class Backpack : public QObject {
    Q_OBJECT

public:
    Backpack(bb::cascades::Application *app);
    Q_INVOKABLE void setKeepAfterRead(int mode);
    Q_INVOKABLE int getKeepAfterRead();
    Q_INVOKABLE void setPocketDeleteMode(int mode);
    Q_INVOKABLE int getPocketDeleteMode();
    Q_INVOKABLE void setIgnoreKeptShuffle(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptShuffle();
    Q_INVOKABLE void setIgnoreKeptOldest(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptOldest();
    Q_INVOKABLE void setIgnoreKeptQuickest(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptQuickest();
    Q_INVOKABLE void setIgnoreKeptLounge(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptLounge();
    Q_INVOKABLE void browseBookmark(QString uri);
    Q_INVOKABLE void browseBookmark(QString uri, QString action);
    Q_INVOKABLE void shuffleBookmark();
    Q_INVOKABLE void takeFigures();
    Q_INVOKABLE void takeOldestBookmark();
    Q_INVOKABLE void takeQuickestBookmark();
    Q_INVOKABLE void takeLoungeBookmark();
    Q_INVOKABLE void memoBookmark(QString memo);
    Q_INVOKABLE void memoBookmark(QString url, QString memo);
    Q_INVOKABLE void keepBookmark(bool keep);
    Q_INVOKABLE void keepBookmark(QString url, bool keep);
    Q_INVOKABLE void removeBookmark(QString url);
    Q_INVOKABLE void removeBookmark(QString url, bool deliberate);
    Q_INVOKABLE void launchSearchToPutin(QString query);
    Q_INVOKABLE void launchRating();
    Q_INVOKABLE QString getAppVersion();
    Q_INVOKABLE void saveBackup();
    Q_INVOKABLE void showBackups();
    Q_INVOKABLE void deleteBackup(QString);
    Q_INVOKABLE void shareBackup(QString);
    Q_INVOKABLE void restoreBackup(QString);
    Q_INVOKABLE void importBackupFile(QString);
    Q_INVOKABLE void pocketCleanContent();
    Q_INVOKABLE void pocketConnect();
    Q_INVOKABLE void pocketDisconnect();
    Q_INVOKABLE void pocketCompleteAuth();
    Q_INVOKABLE bool pocketGetSynconstartup();
    Q_INVOKABLE void pocketSetSynconstartup(bool);
    Q_INVOKABLE int pocketInterval();
    Q_INVOKABLE void pocketSetInterval(int);
    Q_INVOKABLE void refreshBookmarks();
    Q_INVOKABLE void refreshBookmarks(QString query);
    QDate getOldestDate();
    int getQuickestSize();
    void pocketPost(QUrl);
    void pocketArchive(qlonglong pocketId);
    void pocketSetFavourite(qlonglong pocketId, bool favourite);
    void memoBookmark(QUrl url, QString memo);
    void keepBookmark(QUrl url, bool keep);
    void removeBookmark(QUrl url);
    void removeBookmark(QUrl url, bool deliberate);
    void removeBookmark(QUrl url, bool deliberate, bool pocketCleaning);
    virtual ~Backpack();

    Q_INVOKABLE static const int ALL;
    Q_INVOKABLE static const int FAVORITES;
    Q_INVOKABLE static const int ARTICLES;
    Q_INVOKABLE static const int VIDEOS;
    Q_INVOKABLE static const int IMAGES;

public Q_SLOTS:
	void handleInvoke(const bb::system::InvokeRequest&);
	void handleDownloadFailed(QUrl url);
	void updateSize();
	void updateImage(QUrl);
	void updateFavicon(QUrl);
	void updateTitle(QString);
    Q_INVOKABLE void pocketRetrieve();

private:
    TabbedPane *mainPage;
    Page *invokedForm;
	QFile dbFile;
	SqlDataAccess *data;
    InvokeManager *iManager;
    ListView *bookmarks;
    QMap<uint, Bookmark*> bookmark;
    QUrl currentUrl;
    QTimer *timeout;
    SystemToast *backupToast;

    bb::cascades::AbstractPane* mRoot;

    QNetworkAccessManager *network;
    QNetworkReply *reply;
    QByteArray requestToken;
	QTimer *updateTimer;

    ActiveFrame *activeFrame;

    uint cleanUrlHash(QUrl url);
   	bool databaseExists();
    void createDatabase();
    void refreshBookmarks(bool reload);
    void refreshBookmarks(bool reload, QString query);

private Q_SLOTS:
	void reloadQML(QUrl mainFile);
    void cleanup();
    void restoreFinishedFeedback(bb::system::SystemUiResult::Type);
	void deleteBackupFeedback(bb::system::SystemUiResult::Type);
	void deleteBackupConfirmation();
	void pocketHandlePostFinished();
};

#endif /* Backpack_HPP_ */
