#ifndef WEBENGINE_H
#define WEBENGINE_H

#include <QObject>
#include "global.h"
#include "httplistener.h"

using namespace stefanfrings;

class WebEngine : public QObject
{
    Q_OBJECT
public:
    explicit WebEngine(QString configFileName, QObject *parent = nullptr);

private:

    /**
     * Listens for HTTP connections.
     */
    HttpListener* listener;
    HttpRequestHandler *requestHandler;
signals:

};

#endif // WEBENGINE_H
