/**
  @file
  @author Stefan Frings
*/
// модуль для запуска ПО в виде сервиса(службы)
#ifndef STARTUP_H
#define STARTUP_H

#include <QtCore/QCoreApplication>
#include "qtservice.h"

#include "webengine.h"

using namespace stefanfrings;

/**
  Helper class to install and run the application as a windows
  service.
*/
class Startup : public QtService<QCoreApplication>
{
public:

    /** Constructor */
    Startup(int argc, char *argv[]);

protected:

    /** Start the service */
    void start();

    /**
      Stop the service gracefully.
      @warning This method is not called when the program terminates
      abnormally, e.g. after a fatal error, or when killed from outside.
    */
    void stop();

private:

    /**
     * Listens for HTTP connections.
     */
     WebEngine * m_webEngine;    

};

#endif // STARTUP_H
