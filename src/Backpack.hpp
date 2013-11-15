
#ifndef Backpack_HPP_
#define Backpack_HPP_

#include "Bookmark.hpp"
#include "ActiveFrame.hpp"

#include <bb/cascades/QmlDocument>
#include <bb/cascades/TabbedPane>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/ListView>
#include <bb/cascades/Page>
#include <bb/cascades/WebPage>
#include <bb/cascades/WebLoadRequest>
#include <bb/cascades/WebPageCompositor>
#include <bb/data/SqlDataAccess>
#include <bb/system/InvokeManager>
#include <bb/system/InvokeRequest>

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
    Q_INVOKABLE void setBackgroundColour(float base, float red, float green, float blue);
    Q_INVOKABLE float getBackgroundColour(QString colour);
    Q_INVOKABLE void setIgnoreKeptShuffle(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptShuffle();
    Q_INVOKABLE void setIgnoreKeptOldest(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptOldest();
    Q_INVOKABLE void setIgnoreKeptQuickest(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptQuickest();
    Q_INVOKABLE void setIgnoreKeptLounge(bool ignore);
    Q_INVOKABLE bool getIgnoreKeptLounge();
    Q_INVOKABLE void browseBookmark(QString uri);
    Q_INVOKABLE void shuffleBookmark();
    Q_INVOKABLE void oldestBookmark();
    Q_INVOKABLE void quickestBookmark();
    Q_INVOKABLE void loungeBookmark();
    Q_INVOKABLE QDate getOldestDate();
    Q_INVOKABLE int getQuickestSize();
    Q_INVOKABLE int getLoungeSize();
    Q_INVOKABLE bool isKeptOnly();
    Q_INVOKABLE void memoBookmark(QString memo);
    Q_INVOKABLE void memoBookmark(QString url, QString memo);
    Q_INVOKABLE void keepBookmark(bool keep);
    Q_INVOKABLE void keepBookmark(QString url, bool keep);
    Q_INVOKABLE void removeBookmark(QString url);
    Q_INVOKABLE void removeBookmark(QString url, bool deleteKept);
    Q_INVOKABLE void launchSearchToPutin(QString query);
    Q_INVOKABLE void launchRating();
    Q_INVOKABLE QString getAppVersion();
    virtual ~Backpack();

public Q_SLOTS:
	void handleInvoke(const bb::system::InvokeRequest&);
	void handleDownloadFailed(QUrl url);
	void updateSize();
	void updateFavicon();
	void updateTitle(QString);

private:
    TabbedPane *mainPage;
    Page *invokedForm;
	QFile dbFile;
	SqlDataAccess *data;
    InvokeManager *iManager;
    ListView *bookmarks;
    QMap<QUrl, Bookmark*> bookmark;
    QUrl currentUrl;

    ActiveFrame *activeFrame;

    int bookmarkId;
    int bookmarksNumber;
    bool databaseExists();
    void createDatabase();
    void refreshBookmarks();
    void refreshBookmarks(bool reload);
};

#endif /* Backpack_HPP_ */
