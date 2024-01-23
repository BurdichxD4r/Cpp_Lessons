#include "webengine.h"
#include "filelogger.h"
#include "logger.h"
#include "requesthandler.h"
#include "staticfilecontroller.h"
#include "secureservice.h"
//#include <QDir>
//#include <QFile>

/** Logger class */
//FileLogger* logger;
Logger* logger;
extern StaticFileController * staticController;
extern SecureService* secureService;

//----------------------------------------------------------------------------------------
WebEngine::WebEngine(QString configFileName, QObject *parent)
    : QObject{parent}
{
    QSettings* logSettings=new QSettings(configFileName,QSettings::IniFormat,parent);
    logSettings->beginGroup("logging");
    logger=new Logger(parent);
    logger->installMsgHandler();

    // Session store
    QSettings* sessionSettings=new QSettings(configFileName, QSettings::IniFormat,parent);
    sessionSettings->beginGroup("sessions");
    sessionStore=new HttpSessionStore(sessionSettings,parent);


    // Configure template cache
    QSettings* templateSettings=new QSettings(configFileName,QSettings::IniFormat,parent);
    templateSettings->beginGroup("templates");
    templateCache=new TemplateCache(templateSettings,parent);

   // Configure and start the TCP listener
   qDebug("ServiceHelper: Starting service");
   QSettings* listenerSettings=new QSettings(configFileName,QSettings::IniFormat,parent);
   listenerSettings->beginGroup("listener");
   requestHandler = new RequestHandler(parent);
   listener=new HttpListener(listenerSettings,requestHandler,parent);

   // Static file controller
   QSettings* staticSettings=new QSettings(configFileName,QSettings::IniFormat,parent);
   staticSettings->beginGroup("docroot");
   staticController = new StaticFileController(staticSettings, parent);

   QSettings* secureSettings=new QSettings(configFileName,QSettings::IniFormat,parent);
   secureSettings->beginGroup("secure");
   secureService = new SecureService(secureSettings, parent);

   QSettings* testingSettings=new QSettings(configFileName,QSettings::IniFormat,parent);
   testingSettings->beginGroup("test");
   summaryController = new SummaryController(testingSettings, parent);

   settingController = new SettingController(testingSettings, parent);

   plotController = new PlotController(testingSettings, parent);

   adminController = new AdminController(parent);
}
//---------------------------------------------------------------------------------------
