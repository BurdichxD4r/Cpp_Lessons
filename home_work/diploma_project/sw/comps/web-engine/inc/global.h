// модуль с глоб. переменными
#ifndef GLOBAL_H
#define GLOBAL_H

#include "httpsessionstore.h"
#include "staticfilecontroller.h"
#include "templatecache.h"
#include "summarycontroller.h"
#include "secureservice.h"
#include "settingcontroller.h"
#include "admincontroller.h"
#include "plotcontroller.h"


using namespace stefanfrings;

extern AdminController *     adminController;
extern HttpSessionStore*     sessionStore;
extern StaticFileController* staticController;
extern TemplateCache*        templateCache;
extern SummaryController *   summaryController;
extern SecureService   *     secureService;
extern SettingController *   settingController;
extern PlotController *      plotController;

#endif // GLOBAL_H
