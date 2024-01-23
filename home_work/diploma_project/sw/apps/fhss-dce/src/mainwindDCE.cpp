#include "mainwindDCE.h"
#include "ui_mainwindDCE.h"
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>
#include <QComboBox>
#include <QLabel>

//------------------------------------------------------------------
MainWindDCE::MainWindDCE(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindDCE)
{
    ui->setupUi(this);
    m_grid = new QGridLayout(this);
    m_wdg = new QWidget();
    setCentralWidget(m_wdg);  

    // определение ip адреса машины на которой запущено приложение
 /*   QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();
    ipAdr = "";    //listAddr.first().toString();
    for(int i = 0; i <listAddr.count(); ++i ){
        ipAdr = ipAdr+" * " + listAddr.at(i).toString()+" * ";
    }
*/
  // установка иконки
    QPixmap icon_a {":/icon/pics/y_red.png"};
    QApplication::setWindowIcon( icon_a );

   /* QApplication::setApplicationDisplayName("ip[ "+ ipAdr+" ]" );
    QString nameApp = " ПАК АУ АСРЦ ";
    MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК АУ АСРЦ  ip[ "+ ipAdr+" ]");
    */
}
//-----------------------------------------------------------------
MainWindDCE::~MainWindDCE()
{
    delete ui;   
}
//-----------------------------------------------------------------
void MainWindDCE::setParam(QStringList port_ip_list)
{
    // значения параметров по умолчанию


    m_other_params << port_ip_list.at(1)<< port_ip_list.at(3)   // DCE_ID,  port_udp_corr
                   << port_ip_list.at(4)<< port_ip_list.at(5)   // ip_udp_corr, port_udp
                   << port_ip_list.at(6)<< port_ip_list.at(8);  // index_zpch, queue_depth

    m_port_tcp =   port_ip_list.at(2).toInt();
    m_type_msg =   port_ip_list.at(7);
    m_auto_start = port_ip_list.at(9).toInt();   // загрузка настроек из файла и вкл. автоматическое без ожидания EnableDCE

    Widget_DCE *wdg_dce = new Widget_DCE(m_wdg);
    wdg_dce->setPort(m_port_tcp);
    m_vec_wdg_dce.append(wdg_dce);

    connect(m_vec_wdg_dce.at(0), &Widget_DCE::sigPortChng, this, &MainWindDCE::slotPortChanged);
    connect(m_vec_wdg_dce.at(0), &Widget_DCE::sigDCEIdChng, this, &MainWindDCE::slotDCEIdChanged);
    connect(m_vec_wdg_dce.at(0), &Widget_DCE::sigDCEChangeStatus, this, &MainWindDCE::slotServerStatus);
    connect(m_vec_wdg_dce.at(0), &Widget_DCE::sigDCETypeChange, this, &MainWindDCE::slotTypeMsg);

    if (m_auto_start){     // если задан автомат. режим настройки DCE
        m_vec_wdg_dce.at(0)->settAndStartDCE(m_type_msg, m_other_params);   // настраиваем и запускаем сервер при окончании ввода параметров из ini файла
     }
    else   // если запуск от команды EnableDCE, тогда
           // запускаем только порт прослушки для приема управляющих команд от АУ
        m_vec_wdg_dce.at(0)->servDCEStartOnly(m_port_tcp, m_type_msg);
    // отображаем в заголовке глав. формы какой tcp-порт прослушиваем, роль в сети,DCE_ID
   // MainWindDCE::setWindowTitle("ID: " +QString::number(m_vec_wdg_dce.at(0)->getID_DCE())+
   //                        " (port:"+QString::number(m_vec_wdg_dce.at(0)->getPort())+ ") "+
   //                        " ("+m_vec_wdg_dce.at(0)->getRadioRole()+ ") ");

    for (int i = 0; i <m_vec_wdg_dce.count(); ++i){
       m_grid->addWidget(m_vec_wdg_dce.at(i),0,i, 1,1);    
    }
    m_wdg->setLayout(m_grid);
}

//------------------------------------------------------------------------------------------------------------
void MainWindDCE::slotDCEIdChanged()
{

    QString radioRole = m_vec_wdg_dce.at(0)->getRadioRole();
    QString labelRole{""};
    if (radioRole == "TX")
       labelRole="ПРД";
    else if (radioRole == "RX")
        labelRole ="ПРМ";

    if (m_vec_wdg_dce.at(0)->getServerStatus()) //если сервер запущен
       MainWindDCE::setWindowTitle("ID: " +QString::number(m_vec_wdg_dce.at(0)->getID_DCE())+
                                " (p:"+QString::number(m_vec_wdg_dce.at(0)->getPort())+ ") "+
                                " ("+labelRole+ ") ");
    else
        MainWindDCE::setWindowTitle("Stop");
}
//------------------------------------------------------------------------------------------------------------
void MainWindDCE::slotPortChanged(QString port)
{
    Q_UNUSED(port);

    QString radioRole = m_vec_wdg_dce.at(0)->getRadioRole();
    QString labelRole{""};
    if (radioRole == "TX")
       labelRole="ПРД";
    else if (radioRole == "RX")
        labelRole ="ПРМ";

    if (m_vec_wdg_dce.at(0)->getServerStatus()) //если сервер запущен
        MainWindDCE::setWindowTitle("ID: " +QString::number(m_vec_wdg_dce.at(0)->getID_DCE())+
                                " (p:"+QString::number(m_vec_wdg_dce.at(0)->getPort())+ ") "+
                                " ("+labelRole+ ") ");
    else
        MainWindDCE::setWindowTitle("Stop");

}
//--------------------------------------------------------------------------------------------------------------
void  MainWindDCE::slotServerStatus()
{
    if (!m_vec_wdg_dce.at(0)->getServerStatus()) //если сервер остановлен
    {
        MainWindDCE::setWindowTitle("Stop");  // убираем надпись с параметрами запуска сервера в заголовке ПО
    }
}
//---------------------------------------------------------------------------------------------------------------
void MainWindDCE::slotTypeMsg()
{
    QString radioRole = m_vec_wdg_dce.at(0)->getRadioRole();
    QString labelRole{""};
    if (radioRole == "TX")
       labelRole="ПРД";
    else if (radioRole == "RX")
        labelRole ="ПРМ";

    if (m_vec_wdg_dce.at(0)->getServerStatus()) //если сервер запущен
        MainWindDCE::setWindowTitle("ID: " +QString::number(m_vec_wdg_dce.at(0)->getID_DCE())+
                                " (p:"+QString::number(m_vec_wdg_dce.at(0)->getPort())+ ") "+
                                " ("+labelRole+ ") ");
    else
        MainWindDCE::setWindowTitle("Stop");
}
//---------------------------------------------------------------------------------------------------------------
