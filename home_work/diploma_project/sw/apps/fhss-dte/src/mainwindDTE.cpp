#include "mainwindDTE.h"
#include "ui_mainwindDTE.h"
#include <QHostInfo>
#include <QMessageBox>
#include "widgetDTE.h"

//------------------------------------------------------------------
MainWindDTE::MainWindDTE(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindDTE)
{
    ui->setupUi(this);
    m_grid = new QGridLayout(this);
    m_wdg = new QWidget();
    setCentralWidget(m_wdg);

    WidgetDTE *wdg_dte = new WidgetDTE(m_wdg);
    connect (this, SIGNAL(sigDiscon()), wdg_dte, SLOT(slotCloseApp()));
    m_vec_wdg_dte.append(wdg_dte);

    for (int i = 0; i <m_vec_wdg_dte.count(); ++i){
        m_grid->addWidget(m_vec_wdg_dte.at(i),0,i, 1,1);
    }
    m_wdg->setLayout(m_grid);  

    // определение ip адреса машины на которой запущено приложение
    QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();
    ipAdr = "";    //listAddr.first().toString();

    for(int i = 0; i <listAddr.count(); ++i ){
        ipAdr = ipAdr+" * " + listAddr.at(i).toString()+" * ";
    }
  // установка иконки
    QPixmap icon_a {":/icon/pics/a_blue.png"};

    QApplication::setWindowIcon( icon_a );
    QApplication::setApplicationDisplayName("ip[ "+ ipAdr+" ]" );
    QString nameApp = " ПАК АУ АСРЦ ";
   // MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК АУ АСРЦ  ip[ "+ ipAdr+" ]");
  }
//-----------------------------------------------------------------
MainWindDTE::~MainWindDTE()
{
    emit sigDiscon();
    delete ui;   
}
//-------------------------------------------------------------------
void MainWindDTE::setParam(QStringList port_ip_list)
{

    m_vec_wdg_dte.at(0)->setPort(port_ip_list.at(1).toInt());
    m_vec_wdg_dte.at(0)->setIP(port_ip_list.at(2));
}
//-------------------------------------------------------------------
