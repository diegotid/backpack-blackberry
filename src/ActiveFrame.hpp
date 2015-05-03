/*
 * ActiveFrame.hpp
 *
 *  Created on: 25/03/2013
 *      Author: Diego
 */

#ifndef ACTIVEFRAME_HPP_
#define ACTIVEFRAME_HPP_

#include <bb/cascades/Container>
#include <bb/cascades/QmlDocument>
#include <bb/cascades/SceneCover>
#include <bb/data/SqlDataAccess>

using namespace bb::data;
using namespace bb::cascades;

class ActiveFrame: public SceneCover
{
	Q_OBJECT

public:
	ActiveFrame(QObject *parent=0);
	virtual ~ActiveFrame();

public slots:
	QUrl update(bool force);

private:
	Container *coverContainer;
	QmlDocument *qmlCover;
	QFile dbFile;
	SqlDataAccess *sql;
	QVariantList bookmarks;
	QVariantMap previous;
};

#endif /* ACTIVEFRAME_HPP_ */
