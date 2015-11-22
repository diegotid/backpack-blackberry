
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
    Q_INVOKABLE bool isPremium();
    Q_INVOKABLE void setPremium();
    Q_INVOKABLE bool onWiFi();
    Q_INVOKABLE bool getOfflineMode();
    Q_INVOKABLE void setOfflineMode(bool);
    Q_INVOKABLE bool getOfflineImages();
    Q_INVOKABLE void setOfflineImages(bool);
    Q_INVOKABLE bool getOfflineWiFi();
    Q_INVOKABLE void setOfflineWiFi(bool);
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
    Q_INVOKABLE void launchSearchToPutin(QString query);
    Q_INVOKABLE QString getAppVersion();
    Q_INVOKABLE void saveBackup();
    Q_INVOKABLE void showBackups();
    Q_INVOKABLE void deleteBackup(QString);
    Q_INVOKABLE void shareBackup(QString);
    Q_INVOKABLE void restoreBackup(QString);
    Q_INVOKABLE void importBackupFile(QString);
    Q_INVOKABLE void emptyBackapck();
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
    Q_INVOKABLE QString getReadStyle();
    Q_INVOKABLE QString getOfflineDir();
    Q_INVOKABLE uint getOfflineDirSize();
    Q_INVOKABLE void launchRating();
    void loadOffline(QVariantMap);
    void checkOfflineImages(QVariantMap);
    void pocketPost(QUrl);
    void initializeDownloadProgress();
    void pocketProgressiveDownload(QString url);
    void pocketArchiveDelete(qlonglong pocketId);
    void memoBookmark(QUrl url, QString memo);
    void keepBookmark(QUrl url, bool keep);
    void removeBookmark(QUrl url);
    virtual ~Backpack();

    Q_INVOKABLE static const int ALL;
    Q_INVOKABLE static const int FAVORITES;
    Q_INVOKABLE static const int ARTICLES;
    Q_INVOKABLE static const int VIDEOS;
    Q_INVOKABLE static const int IMAGES;

public Q_SLOTS:
	void handleInvoke(const bb::system::InvokeRequest&);
	void handleGet(QNetworkReply*);
	void handleHeadFinished();
	void handleDownloadFailed(QUrl url);
	void handleBookmarkComplete(QUrl, int);
	void updateTitle(QUrl, QString);
	void updateFavicon(QUrl, QUrl);
    void updateImage(QUrl, QUrl);
    void updateImage(uint, QUrl);
	void updateActiveFrame();
	void freeLoadingPage(uint);
    Q_INVOKABLE void pocketRetrieve();
    Q_INVOKABLE void pocketRetrieve(bool info);
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

    uint headingSize;
    uint headingImagesSize;
    QVariantList headingImages;
    QMap<uint, Bookmark*> loading;
    long downloadingSize;
    long downloadingProgress;
    QVariantList downloading;
    QVariantList downloadingImages;
    bool offlineCompleting;
    QStringList favLegacy;
    QFileInfoList toBeRemovedFiles;
    QString currentDir;

    SystemToast *backupToast;
    QTimer *timeout;

    bb::cascades::AbstractPane* mRoot;

    QNetworkAccessManager *network;
    QNetworkReply *reply;
    QNetworkReply *imgReply;
    QNetworkReply *infoReply;
    QNetworkReply *betaReply;
    QByteArray requestToken;
    QTimer *frameUpdateTimer;
	QTimer *pocketUpdateTimer;
    QTimer *downloadPendingTimer;
	bool downloadPending;

    ActiveFrame *activeFrame;

    void createSettings();
    void fetchPocketContent();
	void updateImage(const char*, QUrl);
    void updateActiveFrame(bool force);
    void startPendingTimer();
    void imageInfoPost();

private Q_SLOTS:
    void restoreFinishedFeedback(bb::system::SystemUiResult::Type);
	void deleteBackupFeedback(bb::system::SystemUiResult::Type);
	void deleteBackupConfirmation();
    void resumeOfflineDownload();
    void pocketHandlePostFinished();
    void pocketHandleInfoPostFinished();
    void pocketHandleBetaPostFinished();
//	void reloadQML(QUrl);
//    void cleanup();
};

#endif /* Backpack_HPP_ */
