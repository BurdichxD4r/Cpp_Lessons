#ifndef DISABLEDCE_DIAL_H
#define DISABLEDCE_DIAL_H

#include <QDialog>
#include <QStringList>
#include <QJsonObject>
#include <QSettings>
#include "basecommand.h"
#include "structs_list.h"

/*struct DisableParamStruct{
    int dce_id;
};
Q_DECLARE_METATYPE(DisableParamStruct)
*/

namespace Ui {
class DisableDCE_dial;
}

class DisableDCE_dial : public QDialog, public BaseCommand
{
    QString     ENABLE_SECTION_NAME = "Enable-param";
    QString     SECTION_NAME = "Enable-param";//"Disable-param";
    QString     CRC_KEY_NAME = "crc_enableDCE";//"crc_disableDCE";
    QString     CRC_ENABLE_SECTION = "crc_enableDCE";

    Q_OBJECT

public:
    explicit DisableDCE_dial(DisableParamStruct &data_struct,
                             QWidget *parent = nullptr);
    ~DisableDCE_dial();

private:
    Ui::DisableDCE_dial *ui;
private slots:

    void slotApplay();                      // кнопка ПРИНЯТЬ
    void slotCancel();                      // кнопка ОТМЕНИТЬ

private:  
     DisableParamStruct  *m_disable_param;


    void           readSettFromIni();                 // считывание данных из файла ini на форму
    void           writeDataToIni();                  // запись данных с формы в ini файл
    void           initForm();                        // инициализация формы   
    void           settKeyToVal(QSettings *sett);
    void           valToWgt();
    void           wgtToVal();
    void           defaultSettingsToVal();
    unsigned int   findEnableDCE_ID(QSettings  *sett);  // нахождение номера DCE_ID в секции команды "EnableDCE"
    void           tabOrderSett();
};

#endif // DISABLEDCE_DIAL_H
