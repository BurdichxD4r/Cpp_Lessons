#ifndef NETSYNC_DIAL_H
#define NETSYNC_DIAL_H

#include <QDialog>
#include <QSpinBox>
#include <QSettings>
#include "basecommand.h"
#include "structs_list.h"

/*struct NetSYNCParamStruct{
    int             dce_id;
    int             dce_id_dst;
    uint32_t        imitIns;
    QVector<uint8_t> syncPack;
};
*/


namespace Ui {
class NetSYNC_dial;
}

class NetSYNC_dial : public QDialog, public BaseCommand
{
    Q_OBJECT
    QString     ENABLE_SECTION_NAME = "Enable-param";
    QString     CRC_ENABLE_SECTION = "crc_enableDCE";
    QString     SECTION_NAME = "NetSYNC-param";
    QString     CRC_KEY_NAME = "crc_netSYNC";

public:
    explicit NetSYNC_dial(NetSYNCParamStruct &data_struct,
                          QWidget *parent = nullptr);
    ~NetSYNC_dial();
private slots:
    void slotApplay();                      // кнопка ПРИНЯТЬ
    void slotCancel();                      // кнопка ОТМЕНИТЬ
    void slotDefaultSettings();             // кнопка Настройки по умолчанию
private:
    Ui::NetSYNC_dial    *ui;   
    QVector<QSpinBox*>   m_sync_pack;
    NetSYNCParamStruct   *m_netSYNC_param;

    void      initForm();
    void      tabOrderSett();
    void      readSettFromIni();
    void      defaultSettingsToVal();
    void      settKeyToVal(QSettings *sett);
    unsigned int findEnableDCE_ID(QSettings  *sett);
    void      valToWgt();
    void      writeDataToIni();   
    void      wgtToVal();
  //  QJsonObject fillJsonObj();
};

#endif // NETSYNC_DIAL_H

