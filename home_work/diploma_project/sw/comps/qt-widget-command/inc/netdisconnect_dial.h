#ifndef NETDISCONNECT_DIAL_H
#define NETDISCONNECT_DIAL_H

#include <QDialog>
#include <QStringList>
#include <QJsonObject>
#include <QSettings>
#include "structs_list.h"
#include "basecommand.h"

/*
struct NetDiscParamStruct{
    int dce_id;
    int dce_id_oppn;
};
*/

namespace Ui {
class NetDisconnect_dial;
}

class NetDisconnect_dial : public QDialog, public BaseCommand
{
    Q_OBJECT

    QString     CONNECT_SECTION_NAME = "NetCONNECT-param";
    QString     SECTION_NAME = "NetDisconnect-param";
    QString     CRC_KEY_NAME = "crc_netDiscon";
    QString     CRC_CONNECT_SECTION = "crc_netCONNECT";

public:
    explicit NetDisconnect_dial(NetDISCONParamStruct &data_struct,
                                QWidget *parent = nullptr);
    ~NetDisconnect_dial();

private slots:

    void slotApplay();                      // кнопка ПРИНЯТЬ
    void slotCancel();                      // кнопка ОТМЕНИТЬ

private:
    Ui::NetDisconnect_dial *ui;
    NetDISCONParamStruct   *m_discon_param;

    void                   readSettFromIni();                 // считывание данных из файла ini на форму
    void                   writeDataToIni();                  // запись данных с формы в ini файл
    void                   tabOrderSett();
    void                   initForm();                        // инициализация формы
    void                   settKeyToVal(QSettings *sett);
    void                   valToWgt();
    void                   wgtToVal();
    void                   defaultSettingsToVal();
    NetDISCONParamStruct   findNetConnect_ID(QSettings  *sett);  // нахождение номера DCE_ID в секции команды "EnableDCE"

};

#endif // NETDISCONNECT_DIAL_H

