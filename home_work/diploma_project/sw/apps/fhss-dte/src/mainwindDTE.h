// модуль MW_DTE - главное окно приложения иммитатор ПАК АУ АСРЦ

#ifndef MAINWINDDTE_H
#define MAINWINDDTE_H

#include <QMainWindow>
#include <QDialog>
#include <QMap>
#include <QTimer>
#include <QTime>
#include <QGridLayout>
#include "widgetDTE.h"
/*
struct AUParams{   // структура задающее состояние АУ
    uint32_t           DCE_ID{0};         // позывной привязанного УВС
    uint32_t           DCE_ID_OPPN{0};    // позывной получателя (оппонента)
    QString            create_time{""};   // время создания
    QString            enable_time{""};   // время включения
    PACStatusEnum      status{INIT};      // статус на текущий момент (INIT = 0, ENABLE, NET_SYNC, NET_CONNECT, EXCHENGE, NET_DISCON)
    Version            version;           // версия ПО, загруженная на ПАК АУ/МАС

};
*/

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindDTE; }
QT_END_NAMESPACE

class MainWindDTE : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindDTE(QWidget *parent = nullptr);
    ~MainWindDTE();

    void setParam(QStringList port_ip_list);

private:
    Ui::MainWindDTE *ui;
    QVector<WidgetDTE *>      m_vec_wdg_dte;             // вектор виджетов ПАК AU
    QWidget                  *m_wdg;              // динам. виджет ПАК AU/MAS
    QGridLayout              *m_grid;             // сетка компоновки

signals:
    void sigDiscon();
};
#endif // MAINWINDDTE_H
