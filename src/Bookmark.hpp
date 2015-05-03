
#ifndef Bookmark_HPP_
#define Bookmark_HPP_

#include <bb/cascades/Page>
#include <bb/cascades/WebPage>
#include <bb/cascades/WebLoadRequest>
#include <bb/cascades/WebPageCompositor>
#include <bb/data/SqlDataAccess>

using namespace bb::cascades;
using namespace bb::data;

namespace bb {
	namespace cascades {
		class Application;
	}
}

class Bookmark : public QObject {

	Q_OBJECT

public:
    Bookmark(QUrl url, SqlDataAccess *data, QObject *parent=0);
    Bookmark(QUrl url, qlonglong pocketId, QString title, bool favorited, SqlDataAccess *data, QObject *parent=0);
    virtual ~Bookmark();

    void pocketSync(qlonglong id, QDateTime added);
    void pocketSync(qlonglong id);
    void fetchContent();
    bool loading();
    QUrl getUrl();
    QString getTitle();
    QString getMemo();
    QString getImagePath();
    bool alreadyExisted();
    bool isKept();
    void remove();

    QString static cleanUrl(QUrl url);
    uint static cleanUrlHash(QUrl url);
    void static setKept(SqlDataAccess *data, QUrl, bool);
    void static saveMemo(SqlDataAccess *data, QUrl, QString);
    qlonglong static getPocketId(SqlDataAccess *data, QUrl url);

public slots:
	void handleTitle(QString);
	void handleContent(QNetworkReply*);
	void downloadImage(QNetworkReply*);
	void downloadFavicon(QNetworkReply*);

signals:
	void downloadComplete(uint);

private:
	QUrl url;
	uint hashUrl;
	qlonglong pocketId;
	QString title;
	QString memo;
	QString image;
	QString favicon;
	bool kept;
	bool existing;
	SqlDataAccess *data;
	WebPage *page;
	QTimer timeout;
	QNetworkAccessManager *network;
    QNetworkRequest bookmarkRequest;
    QNetworkRequest imageRequest;
    QNetworkRequest iconRequest;
    bool fetchingContent;
    bool fetchingImage;
    bool fetchingIcon;

    void initiate();

private slots:
	void handleDownloadStatusChange(bb::cascades::WebLoadRequest*);
};

#endif /* Bookmark_HPP_ */
