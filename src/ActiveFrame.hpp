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
	void update();
	void update(bool force);
	void takeFigures(QObject *parent);
	void takeBackground(QObject *parent);

private:
	Container *coverContainer;
	QmlDocument *qmlCover;
	QTimer *updateTimer;
	QFile dbFile;
	SqlDataAccess *sql;
	QVariantList bookmarks;
};

#endif /* ACTIVEFRAME_HPP_ */
