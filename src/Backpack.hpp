
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
#include <bb/cascades/GroupDataModel>
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
    Q_INVOKABLE void logEvent(QString mode);
    Q_INVOKABLE bool getSettingsUnderstood();
    Q_INVOKABLE void setSettingsUnderstood();
    Q_INVOKABLE void setKeepAfterRead(bool keep);
    Q_INVOKABLE bool getKeepAfterRead();
    Q_INVOKABLE void setPocketDeleteMode(bool pocketDelete);
    Q_INVOKABLE int getPocketDeleteMode();
    Q_INVOKABLE void setIgnoreKeptShuffle(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptShuffle();
    Q_INVOKABLE void setIgnoreKeptQuickest(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptQuickest();
    Q_INVOKABLE void setIgnoreKeptLounge(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptLounge();
    Q_INVOKABLE void readBookmark(QString uri);
    Q_INVOKABLE void browseBookmark(QString uri);
    Q_INVOKABLE void shuffleBookmark();
    Q_INVOKABLE QVariant quickestBookmark();
    Q_INVOKABLE QVariant quickestBookmark(int offset);
    Q_INVOKABLE QVariant loungeBookmark();
    Q_INVOKABLE QVariant loungeBookmark(int offset);
    Q_INVOKABLE void memoBookmark(QString url, QString memo);
    Q_INVOKABLE void keepBookmark(QString url, bool keep);
    Q_INVOKABLE void removeBookmark(QString url);
    Q_INVOKABLE void removeBookmark(QString url, bool deliberate);
    Q_INVOKABLE void launchSearchToPutin(QString query);
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
    Q_INVOKABLE void fetchContent(QString url);
    Q_INVOKABLE void add(QString url);
    Q_INVOKABLE void add(QUrl url);
    void pocketPost(QUrl);
    void pocketArchiveDelete(qlonglong pocketId);
    void memoBookmark(QUrl url, QString memo);
    void keepBookmark(QUrl url, bool keep);
    void removeBookmark(QUrl url);
    void removeBookmark(QUrl url, bool deliberate);
    virtual ~Backpack();

    Q_INVOKABLE static const int ALL;
    Q_INVOKABLE static const int FAVORITES;
    Q_INVOKABLE static const int ARTICLES;
    Q_INVOKABLE static const int VIDEOS;
    Q_INVOKABLE static const int IMAGES;

public Q_SLOTS:
	void handleInvoke(const bb::system::InvokeRequest&);
	void handleDownloadFailed(QUrl url);
	void handleBookmarkComplete(QUrl, int);
	void updateTitle(QUrl, QString);
	void updateFavicon(QUrl, QUrl);
	void updateImage(QUrl, QUrl);
	void updateActiveFrame();
	void freeLoadingPage(uint);
    Q_INVOKABLE void pocketRetrieve();
    Q_INVOKABLE void pocketDownload(QString);

private:
    TabbedPane *mainPage;
    Page *invokedForm;
	QFile dbFile;
	SqlDataAccess *data;
    InvokeManager *iManager;
    ListView *bookmarks;
    GroupDataModel *bookmarksByURL;
    GroupDataModel *bookmarksByDate;
    QMap<uint, Bookmark*> loading;
    QTimer *timeout;
    SystemToast *backupToast;

    bb::cascades::AbstractPane* mRoot;

    QNetworkAccessManager *network;
    QNetworkReply *reply;
    QByteArray requestToken;
	QTimer *pocketUpdateTimer;
    QTimer *frameUpdateTimer;
	bool refreshRequested;

    ActiveFrame *activeFrame;

   	bool databaseExists();
    void createDatabase();
    void fetchPocketContent();
	void updateImage(const char*, QUrl);
    void updateActiveFrame(bool force);

private Q_SLOTS:
    void restoreFinishedFeedback(bb::system::SystemUiResult::Type);
	void deleteBackupFeedback(bb::system::SystemUiResult::Type);
	void deleteBackupConfirmation();
	void pocketHandlePostFinished();
//	void reloadQML(QUrl);
//    void cleanup();
};

#endif /* Backpack_HPP_ */
