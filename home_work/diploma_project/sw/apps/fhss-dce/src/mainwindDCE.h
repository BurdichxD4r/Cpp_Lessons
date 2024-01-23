// модуль главной формы ПАУ УВС (сервера)

#ifndef MAINWINDDCE_H
#define MAINWINDDCE_H

#include <QMainWindow>
#include <QDialog>
#include <QTime>
#include <QComboBox>
#include <QGridLayout>

#include "widget_dce.h"
#include "widget-exch-IQ.h"
#include "modempanel.h"


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindDCE; }
QT_END_NAMESPACE


class MainWindDCE : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindDCE(QWidget *parent = nullptr);
    ~MainWindDCE();

    void setParam(QStringList);                           // метод  класса для настройки динам. виджетов

private slots:

    void                slotPortChanged(QString port);  // порт прослушки
    void                slotDCEIdChanged();  // номер dce_id
    void                slotServerStatus(); // состояние сервера (запущен или нет)
    void                slotTypeMsg();      // тип сообщения


private:
    Ui::MainWindDCE *ui;
    QVector<Widget_DCE *>      m_vec_wdg_dce;             // вектор виджетов ПАК УВС


    QGridLayout       *m_grid;             // сетка компоновки
    QWidget           *m_wdg;              // динам. виджет ПАК УВС

    QStringList        m_other_params;     // другие параметры для настройки УВС из cmd
    int                m_port_tcp;         // порт прослушки tcp
    int                m_auto_start;       // ввод в раб. состояние без команды EnableDCE, используя файл настроек "dce.ini"
    QString            m_net_role;         // роль в сети (TX or RX)
    QString            m_type_msg;         // тип сообщения (JSON, XML)



    NetS               m_adr_corr;         // адрес (ip - адрес, порт)корресп
    int                m_port_bind;        // порт прослушки для udp соед.
    int                m_index_zpch;       // индекс ЗПЧ
    int                m_start_buf;        // старт. буфферизация
    int                m_que_depth;        // глубина очереди отправки


};

#endif // MAINWINDDCE_H
