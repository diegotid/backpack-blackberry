
#ifndef Backpack_HPP_
#define Backpack_HPP_

#include "ActiveFrame.hpp"

#include <bb/cascades/QmlDocument>
#include <bb/cascades/NavigationPane>
#include <bb/cascades/AbstractPane>
#include <bb/cascades/ListView>
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

/*!
 * @brief Application pane object
 *
 *Use this object to create and init app UI, to create context objects, to register the new meta types etc.
 */
class Backpack : public QObject
{
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
    Q_INVOKABLE void memoBookmark(QString);
    Q_INVOKABLE void memoBookmark(QString, int);
    Q_INVOKABLE void removeBookmark(int);
    Q_INVOKABLE void removeBookmark(int, bool);
    Q_INVOKABLE void shuffleBookmark();
    Q_INVOKABLE void oldestBookmark();
    Q_INVOKABLE void quickestBookmark();
    Q_INVOKABLE QDate getOldestDate();
    Q_INVOKABLE int getQuickestSize();
    Q_INVOKABLE void keepBookmark(bool);
    Q_INVOKABLE void keepBookmark(bool, int);
    virtual ~Backpack();

public Q_SLOTS:
	void handleInvoke(const bb::system::InvokeRequest&);
	void handleBookmarkTitle(QString);
	void handleBookmarkIcon(QUrl);
	void handleBookmarkSize(QNetworkReply*);

private:
	QmlDocument *homeQml;
	QmlDocument *invokedQml;
    NavigationPane *homePage;
    Page *invokedForm;
	QFile dbFile;
	SqlDataAccess *data;
    InvokeManager *iManager;
    QNetworkAccessManager *network;
    QNetworkRequest bookmarkRequest;
    WebPage *bookmark;
    ListView *bookmarks;

    ActiveFrame *activeFrame;

    int bookmarkId;
    int bookmarksNumber;
    bool databaseExists();
    void createDatabase();
    void refreshBookmarks();
};

#endif /* Backpack_HPP_ */
