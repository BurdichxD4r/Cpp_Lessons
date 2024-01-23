#include "logincontroller.h"
#include "secureservice.h"
#include <QTime>
#include "global.h"
#include "staticfilecontroller.h"
#include "template.h"

extern SecureService * secureService;

//-----------------------------------------------------------------------------------
LoginController::LoginController(QObject* parent)
    :HttpRequestHandler(parent) {
    // empty

}
//-----------------------------------------------------------------------------------
void LoginController::service(HttpRequest &request, HttpResponse &response)
{
    HttpSession session=sessionStore->getSession(request,response,true);
    qDebug()<<session.getId();

    QByteArray username=request.getParameter("username");
    QByteArray password=request.getParameter("password");

    qDebug("username=%s",username.constData());
    qDebug("password=%s",password.constData());

    response.setHeader("Content-Type", "text/html; charset=UTF-8");  

    QByteArray language=request.getHeader("Accept-Language");
    Template t=templateCache->getTemplate("authen_templ",language);

    bool login_in{false};
    login_in = session.contains("username");
    t.setCondition("login_in",login_in);

    bool secure_login{false};

    if (session.contains("username")) {// если сессия подтвержденна для данного аккаунта
            QByteArray username=session.get("username").toByteArray();
            QTime logintime=session.get("logintime").toTime();

            t.setVariable("username",username);
            t.setVariable("logintime",logintime.toString("HH:mm:ss"));

        }
    else { // сессия не подтверждена для данного аккаунта (либо входим впервые , либо неправильный логин / пароль)            
           secure_login = secureLogin(username, password);
           t.setCondition("secure_login",secure_login);

           if(secureLogin(username, password)){   // логин пароль верный
               QTime logintime=session.get("logintime").toTime();
               t.setVariable("username",username);
               t.setVariable("logintime",logintime.toString("HH:mm:ss"));

               session.set("username",username);
               session.set("logintime",QTime::currentTime());

           }
          else {             // логин пароль не верный или не заполнен
               t.setCondition("empty_login",username.isEmpty());
         }
    }  
    response.write(t.toUtf8(),true);
}
//-------------------------------------------------------------------------------------
bool LoginController::secureLogin(QByteArray & username, QByteArray & pass){

    return secureService->checkAuthentication(username, pass);
}
//-------------------------------------------------------------------------------------
