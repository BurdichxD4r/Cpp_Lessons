#ifndef ADMINCONTROLLER_H
#define ADMINCONTROLLER_H

#include <httprequesthandler.h>

using namespace stefanfrings;

class AdminController : public HttpRequestHandler
{
    Q_OBJECT
public:
    AdminController(QObject *parent = nullptr);
    void service(HttpRequest& request, HttpResponse& response);

private:   

    void createUsersList();
    void showUsersList();

    QString m_login_ch{""};
    QString m_pass_ch{""};
    int     m_num_ch{-1};
};

#endif // ADMINCONTROLLER_H
