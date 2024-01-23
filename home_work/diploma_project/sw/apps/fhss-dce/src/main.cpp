#include "mainwindDCE.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);   
    MainWindDCE w;
    QStringList args = a.arguments();
    QString  show_GUI{"0"};

    if(args.count()>1){  // пользов. задан список вход. парам.
        w.setParam(args);
    }
   else {               // список вход парам. отсутстует
        QStringList list_param;        
        list_param <<"11"<<"1"                                    // all_param_count, DCE_ID,
                   << QString::number(2525)                       // port_tcp DCE
                   << QString::number(2323) << "127.0.0.1"        // port-udp corr57201,  ip-udp corr192.168.102.1,
                   << QString::number(2327) << QString::number(1) // port_udp DCE57501,   index_zpch
                   <<"JSON"                                       // type of msg
                   << QString::number(0)                            // queue_depth
                   << QString::number(0)                          // auto-start-tcp (setting from .ini)
                   << QString::number(1);                         // GUI_mode(1-visible, 0-invisible) mast be last

        show_GUI=list_param.last();
        w.setParam(list_param);

   }

    if(args.last()=="1" || show_GUI=="1") // если последний параметр - GUI_mode =1,  загружаем интерфейс
        w.show();
    return a.exec();
}
