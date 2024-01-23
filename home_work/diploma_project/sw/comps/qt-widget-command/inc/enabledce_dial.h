#ifndef ENABLEDCE_DIAL_H
#define ENABLEDCE_DIAL_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QComboBox>

#include <QStringList>
#include <QJsonObject>
#include <tablefreq.h>
#include <QSettings>

#include "basecommand.h"
#include "widgetrc.h"
#include "structs_list.h"

/*
struct RadioParams{
    QString    name;
    QString    ipv4;
    int        tcp_port;
    int        udp_port1;
    int        udp_port2;
};

struct EnableParamStruct{
    int                 dce_id;
    int                 mode_bw;    // полоса пропускания
    int                 modeSw;     // наименование режима
    int                 tm_switch;  // время перестройки частоты
  //  int mode1;      // режим mode1
    int                 modeEx;     // уточнение режима (симплекс, дуплекс, полудуплекс
    int                 stage;      // этап
    int                 fr_offset;  // смещение частот
    int                 fr_all;     // кол-во частот
    int                 fr_active;  // кол-во активных частот
    int                 fr_adapt;   //
    int                 cons_adapt;  //
    bool                use_freq_list;     // используется ли список частот
    QList <int>         freq_list;  // список частот
    QList <int>         freq_list_prim;  // список частот НПП
    QList <int>         freq_list_add;  // список частот ЗЧ
    QList <RadioParams> radio_param;   // список радисредств
    int                 pcp_key;        // ключ ПСП
    int                 initPcpTime;    // нач. момнет времени для ПСП
    int                 modeFreqPcp;    // номер режима для рассчета РАб. частоты по ПСП
};
*/


enum StageFormEnum{  // стадия функционирования формы
    INIT_FORM = 0,   // стадия загрузки (для загрузки данных из ини файла)
    WORK             // стадия работы   (данные уже загружены)
};

namespace Ui {
class EnableDCE_dial;
}

class EnableDCE_dial : public QDialog, public BaseCommand
{
    Q_OBJECT

public:   
    explicit EnableDCE_dial(EnableParamStruct &data_struct,            // передаем в форму json-объект для заполнения данными
                             QWidget *parent = nullptr);
    ~EnableDCE_dial();

private slots:

    void slotRCSett();                // слот настройка радиосредств (внешний вид и доступность вкладки радио средства)
    void slotDHS(int index);          // определяет поведение группу "Уточнение режима" (в зав. от "Режим")
    void slotDHS_bw();                // определяет поведение группу "Уточнение режима" (в зав. от "Режим", "Полосы частот", "смены радиоЧастот"(3,75))
    void slotUseFreqList(bool);       // состояние chBox_use_freq_list

    void slotRX_check();
    void slotTX_check();

    void slotNumAllChange(int);       //
    void slotNumActiveChange(int);    //
    void slotNumOffSetChange(int);    //
    void slotChangeFreqAll();         //
 //   void slotStageRegim();          //

    void slotAddRC();                 // доб. новой еденицы радио средств
    void slotDelRC();                 // удал. еденицы радио средств

    void slotApplay();                // кнопка ПРИНЯТЬ
    void slotCancel();                // кнопка ОТМЕНИТЬ
    void slotDefaultSettings();       //
   // void modelRegimSett();            //

    void on_rb_radio_dir_tx_clicked(bool checked);

    void on_rb_radio_dir_rx_clicked(bool checked);


private:
    Ui::EnableDCE_dial *ui;
    QVector <widgetRC*> m_vec_widg_rc;          // вектор радиосредств
    EnableParamStruct  *m_enable_param;         // структура хранящая данные полей команды
    int                 m_rc_count{0};          // количество вкладок радио средств на форме
    StageFormEnum       m_stage{INIT_FORM};     // стадия функционирования формы
    int                 m_mas_port;           // номер порта к которому подключен ПАК АУ и по которому должен быть подключен ПАК МАС через команду

    void               readSettFromIni();           // считывание данных из файла ini на форму
    void               writeDataToIni();            // запись данных с формы в ini файл
    void               useFreqList(bool state);     // настройка дострупности группы "Список частот" и "кол-во частот"
    void               tabOrderSett();              //
    void               initForm();                  // инициализация формы
    void               settKeyToVal(QSettings *sett);      //
    void               valToWgt();                         //
    void               wgtToVal();
    //

  //  void           addColToRow(QString str, QColor cl,         //
  //                             QList<QStandardItem *> &items); //
    void               defaultSettingsToVal();             //
};

#endif // ENABLEDCE_DIAL_H
