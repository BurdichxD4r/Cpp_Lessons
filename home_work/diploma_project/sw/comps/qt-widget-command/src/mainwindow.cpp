#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "enabledce_dial.h"
#include <QTableView>
#include <QJsonDocument>

//-------------------------------------------------------------------------------------
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->pb_com1->setText("ok");

   connect(ui->pb_com1, SIGNAL(clicked()), this, SLOT(commandDialogSlot()));

}
//--------------------------------------------------------------------------------------
MainWindow::~MainWindow()
{
    delete ui;
}
//--------------------------------------------------------------------------------------
void MainWindow::commandDialogSlot()
{ 
    QJsonObject json_obj;   // приготовили объект для заполнения (пока он пустой)

    m_dialog_com = new EnableDCE_dial(json_obj,this);
    if(m_dialog_com->exec() == QDialog::Accepted){

    }
    delete m_dialog_com;
    m_dialog_com = nullptr;
    QByteArray ba_out = QJsonDocument(json_obj).toJson();

}
//--------------------------------------------------------------------------------------

