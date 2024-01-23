// модуль вкладки ПОМОЩЬ
#ifndef HELPCONTROLLER_H
#define HELPCONTROLLER_H

#include <httprequesthandler.h>

using namespace stefanfrings;

class HelpController : public HttpRequestHandler
{
    Q_OBJECT
public:
    explicit HelpController(QObject *parent = nullptr);
    void service(HttpRequest& request, HttpResponse& response);
};

#endif // HELPCONTROLLER_H
