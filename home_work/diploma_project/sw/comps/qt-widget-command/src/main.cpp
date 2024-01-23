#include "mainwindow.h"

#include <QApplication>

//QString APP_ABS_PATH;     // appAbsPath;
//QString INI_PATH;        // дир-я ../INI/
//QString INI_FL;          // имя файла конфигур.

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
   
    //APP_ABS_PATH = QCoreApplication::applicationDirPath();
   // INI_PATH  ="/INI/";         // дир. с файлами конфигур.
    //INI_FL    = "command.ini";  // имя файла конфигур.
    
    
    w.show();
    return a.exec();
}
