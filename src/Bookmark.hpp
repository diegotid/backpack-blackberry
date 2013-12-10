
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
    void fetchContent();
    bool loading();
    QString getTitle();
    QString getMemo();
    bool alreadyExisted();
    bool isKept();
    void setKept(bool);
    void saveMemo(QString);
    void remove();
    virtual ~Bookmark();

public slots:
	void handleIcon(QUrl);
	void handleTitle(QString);
	void handleSize(QNetworkReply*);
	void downloadFavicon(QNetworkReply*);

signals:
	void sizeChanged();
	void iconChanged();
	void titleChanged(QString);

private:
	int id;
	QUrl url;
	QString title;
	QString memo;
	bool kept;
	bool existing;
	QFile dbFile;
	SqlDataAccess *data;
	WebPage *page;
	QTimer timeout;
    bool titleComplete;
    bool faviconComplete;
	QNetworkAccessManager *network;
    QNetworkRequest bookmarkRequest;
    QNetworkRequest iconRequest;

private slots:
	void handleDownloadStatusChange(bb::cascades::WebLoadRequest*);
};

#endif /* Bookmark_HPP_ */
