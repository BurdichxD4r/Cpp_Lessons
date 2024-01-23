#ifndef NETCONNECT_DIAL_H
#define NETCONNECT_DIAL_H

#include <QDialog>
#include <QSpinBox>
#include <QSettings>
#include <QStandardItemModel>
#include <QComboBox>
#include <QTableWidgetItem>
#include "basecommand.h"
#include "structs_list.h"

/*struct NetCONNECTParamStruct{
   int        dceId;         // позывной сервера
   int        dceIdOppn;     // позывной оппонента
   int        priority;      // приоритет оппонента
   int        modeBW;        // полоса пропускания
   int        modeSwitch;    // наименование переключения
   int        modeTrans;     // уточнение режима (синхро, асинхро)
   int        modeEx2;       // уточнение режима (симплекс, дуплекс, полудуплекс
   int        modeChangeFreq; // режим переключения частоты РС (75,3)
   int        modeRateSwitch;   // скорость переключения (200, 50)
   int        rate;          // скорость
   int        modeAdapt;     // адаптация по частоте
   int        autoDiscon;    // авто разрыв соединения
   int        haveInfo;      // есть ли информ.для передачи
   int        radioNetType;  // тип комбинированной сети
   int        freeParam;     // предлагаемый канал передачи
   uint32_t   imitIns;       // имитовставка
   int        factorNet;     // полоса пропускания
   int        factorUser;    // полоса пропускания
   QVector<uint8_t> syncPack;   // синхропакет
};
*/

namespace Ui {
class NetCONNECT_dial;
}

class NetCONNECT_dial : public QDialog, public BaseCommand
{
    Q_OBJECT

    QString     ENABLE_SECTION_NAME = "Enable-param";
    QString     SECTION_NAME = "NetCONNECT-param";
    QString     CRC_KEY_NAME = "crc_netCONNECT";
    QString     CRC_ENABLE_SECTION = "crc_enableDCE";

public:
    explicit NetCONNECT_dial(NetCONNECTParamStruct &data_struct,
                             QWidget *parent = nullptr);
    ~NetCONNECT_dial();

private slots:
    void slotApplay();                      // кнопка ПРИНЯТЬ
    void slotCancel();                      // кнопка ОТМЕНИТЬ
    void slotDefaultSettings();             // кнопка Настройки по умолчанию
    void modelRegimSett();            //

private:
    Ui::NetCONNECT_dial   *ui;
    QJsonObject           *m_json_obj;       // переменная для хранения указателья внешнего json объекта
    QVector<QSpinBox*>     m_sync_pack;
    NetCONNECTParamStruct *m_netCONNECT_param;

    void        initForm();
    QString     fileModelRead(QString file_name);  // загрузка данных из файла mode1.txt c данными модели для cb_regim
    void        createModel(QString content,       // создание модели для загрузки ее в cb_regim
                               QStandardItemModel & model);
    void        addModelToWidg(QStandardItemModel & model, // подключение модели с данными к виджетам формы
                                  QComboBox* combo_box);
    void        tabOrderSett();
    void        readSettFromIni();
    void        defaultSettingsToVal();
    void        settKeyToVal(QSettings *sett);
    unsigned int findEnableDCE_ID(QSettings  *sett);
    void        valToWgt();
    void        writeDataToIni();    
    void        wgtToVal();   
    void        addColToRow(QString str, QColor cl,         //
                                 QList<QStandardItem *> &items); //
};

#endif // NETCONNECT_DIAL_H
