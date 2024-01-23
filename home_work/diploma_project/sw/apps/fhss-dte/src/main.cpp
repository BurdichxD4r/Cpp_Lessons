#include "mainwindDTE.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QStringList args = a.arguments();
    MainWindDTE w;
    if(args.count()>1){
      // w.setParam(args.at(1).toDouble());
        w.setParam(args);

    }
   else {
        QStringList list_param;
        list_param <<"3"<< QString::number(2324) <<"127.0.0.1";
        w.setParam(list_param);

     }     

    w.show();
    return a.exec();
}
