#include <QApplication>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
   QApplication a(argc, argv);
   
    MainWindow w;
    w.resize(600, 300);
    w.show();

    return a.exec();
}
