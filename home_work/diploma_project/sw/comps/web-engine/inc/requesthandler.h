/**
  @file
  @author Stefan Frings
*/
//модуль выполняющий переключения между различными вкладками веб интерфейса
#ifndef REQUESTHANDLER_H
#define REQUESTHANDLER_H

#include "httprequesthandler.h"
#include "logincontroller.h"
#include "helpcontroller.h"
#include "summarycontroller.h"
#include "settingcontroller.h"
#include "global.h"

using namespace stefanfrings;

/**
  The request handler receives incoming HTTP requests and generates responses.
*/

class RequestHandler : public HttpRequestHandler {
    Q_OBJECT
    Q_DISABLE_COPY(RequestHandler)
public:

    /**
      Constructor.
      @param parent Parent object
    */
    RequestHandler(QObject* parent=0);
private:
    LoginController loginController;
    HelpController  helpController;

    /**
      Process an incoming HTTP request.
      @param request The received HTTP request
      @param response Must be used to return the response
    */
    virtual void service(HttpRequest& request, HttpResponse& response) override;

};

#endif // REQUESTHANDLER_H
