#include "widget_dce.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Widget_DCE w;
    QStringList args = a.arguments();
    if(args.count()>1){
       w.setPort(args.at(1).toDouble());
    }
   else {
        w.setPort(2324);

     }
    w.show();
    return a.exec();
}
