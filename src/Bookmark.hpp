
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
    void pocketSync(qlonglong id, QDateTime added);
    void pocketSync(qlonglong id);
    void fetchContent();
    bool loading();
    QUrl getUrl();
    QString getTitle();
    QString getMemo();
    qlonglong getPocketId();
    bool alreadyExisted();
    bool isKept();
    void setKept(bool);
    void saveMemo(QString);
    void remove();
    virtual ~Bookmark();

public slots:
	void handleIcon(QUrl);
	void handleTitle(QString);
	void handleContent(QNetworkReply*);
	void downloadImage(QNetworkReply*);
	void downloadFavicon(QNetworkReply*);

signals:
	void sizeChanged();
	void titleChanged(QString);
	void imageChanged(QUrl);
	void faviconChanged(QUrl);

private:
	int id;
	qlonglong pocketId;
	QUrl url;
	QString title;
	QString memo;
	QString image;
	QString favicon;
	bool kept;
	bool existing;
	QFile dbFile;
	SqlDataAccess *data;
	WebPage *page;
	QTimer timeout;
	QNetworkAccessManager *network;
    QNetworkRequest bookmarkRequest;
    QNetworkRequest imageRequest;
    QNetworkRequest iconRequest;

private slots:
	void handleDownloadStatusChange(bb::cascades::WebLoadRequest*);
};

#endif /* Bookmark_HPP_ */
