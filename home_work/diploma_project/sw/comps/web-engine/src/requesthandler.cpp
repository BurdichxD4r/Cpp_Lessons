/**
  @file
  @author Stefan Frings
*/
//class that will switch between different request controllers
// организация различных вкладок веб приложения
#include "requesthandler.h"
#include "filelogger.h"
#include "logger.h"
//#include "staticfilecontroller.h"
#include "logincontroller.h"
#include "helpcontroller.h"
#include "secureservice.h"
#include "global.h"
#include "summarycontroller.h"


/** Logger class */
//extern FileLogger* logger;

extern Logger*                logger;
extern AdminController *      adminController;
extern StaticFileController * staticController;
extern SummaryController *    summaryController;
extern SettingController *    settingController;
extern PlotController *       plotController;

//-------------------------------------------------------------------------------
RequestHandler::RequestHandler(QObject* parent)
    :HttpRequestHandler(parent)
{

}
//--------------------------------------------------------------------------------
void RequestHandler::service(HttpRequest& request, HttpResponse& response)
{
    QByteArray path=request.getPath();
    qDebug("Conroller: path=%s",path.data());

    /// перенаправление, если пользователь зашел куда то кроме /login не осуществив вход по паролю
    QByteArray sessionId=sessionStore->getSessionId(request,response);
    HttpSession session=sessionStore->getSession(request,response,true);

    if (/*sessionId.isEmpty()*/ !session.contains("username") && path!="/login") {
        qDebug("RequestHandler: redirect to login page");
        response.redirect("/login");
        return;
    }

    if ((path=="/")||(path=="/login"))  {
         loginController.service(request, response);
     }
    else if (path.startsWith("/admining"))  {       // вкладка  Администрирование  работы системы MM
         adminController->service(request, response);
     }
    else if ((path=="/")||(path=="/setting"))  {    // вкладка  Настройка работы системы MM
         settingController->service(request, response);
     }    

    else if (path.startsWith("/summary")&& session.contains("username")){ //вкладка Состояние системы ММ
         summaryController->service(request,response);
    }

    else if (path.startsWith("/plot")&& session.contains("username")){       
         plotController->service(request,response);
    }


   /// если сообщение об ошибке должено быть на русском (или китайском)
    else {
           response.setStatus(404,"Not found");
           response.setHeader("Content-Type", "text/html; charset=UTF-8");
           QString russian = QString::fromUtf8("Запрашиваемого вами документа нет.");
           response.write(russian.toUtf8(),true);
       }

    qDebug("RequestHandler: finished request");

    // Clear the log buffer
    if (logger)
    {
       logger->clear();
    }
//------------------------------------------------------------------------------------
}
