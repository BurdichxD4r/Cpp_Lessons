#ifndef EXCHAGEDCE_DIAL_H
#define EXCHAGEDCE_DIAL_H

#include <QDialog>
#include <QStringList>
#include <QJsonObject>
#include <QSettings>
#include "basecommand.h"
#include <QTimer>


/*struct ExchangeParamStruct{
    int dce_id;
    int dce_id_oppn;
    int msg_id{0};
    QByteArray data;
    int type;
};
*/
namespace Ui {
class ExchageDCE_dial;
}

class ExchageDCE_dial : public QDialog, public BaseCommand
{
    Q_OBJECT
    QString     CONNECT_SECTION_NAME = "Exchange-param";
    QString     SECTION_NAME = "Exchange-param";
    QString     CRC_KEY_NAME = "crc_exchange";

  /*  enum ExchangeModeEnum{ // тип для задания состояния ПАК МАС (тест на поврежд. символы или обмен информ.)
        TEST_EXCH = 0,
        INFO_EXCH
    };
    */

public:
    explicit ExchageDCE_dial(ExchangeParamStruct &data_struct,
                             int &lett_pass, int mode,
                             QWidget *parent = nullptr);
    ~ExchageDCE_dial();
protected:
     void showEvent(QShowEvent *ev) override;
private slots:
    void slotApplay();                      // кнопка ПРИНЯТЬ
    void slotCancel();                      // кнопка ОТМЕНИТЬ
    void slotDataParityLengthChange(int);         // изменение размера передаваемой последовательности
    void slotDataNoParityLengthChange(int);      // изменение размера передаваемой последовательности
    void slotDataGeneralLengthChange(int);         // изменение размера передаваемой последовательности
    void slotDataParityAmplChange(int);         // изменение амплитуды передаваемой последовательности
    void slotDataNoParityAmplChange(int);         // изменение амплитуды передаваемой последовательности
    void slotErrAdd(bool);


    //void on_te_Data_general_textChanged();

private:
    Ui::ExchageDCE_dial    *ui;
   // QJsonObject            *m_json_obj;         // переменная для хранения указателья внешнего json объекта
    ExchangeParamStruct     *m_exchange_param;
    int                     *m_lett_pass;
    QString                 m_str;

    QByteArray              m_test_ba_parity;      // чет. послед. в режиме теста
    QByteArray              m_test_ba_noparity;    // нечет. послед. в режиме теста
    QByteArray              m_exch_ba_general;     // послед. для передачи в режиме обмена инф.

    int                     m_mode_exch;
    int                     m_data_len_parity {32};         // длина передаваемой последовательности
    int                     m_data_len_noparity {32};         // длина передаваемой последовательности
    int                     m_data_ampl_parity {50};         // амплит. передаваемой последовательности
    int                     m_data_ampl_noparity {77};         // амплитуда передаваемой последовательности


    void           readSettFromIni();                 // считывание данных из файла ini на форму
    void           writeDataToIni();                  // запись данных с формы в ini файл

  //  QJsonObject    fillJsonObj();                      // заполнение json-объекта, переданого из вне, данными с формы
    void           tabOrderSett();

    void           initForm();                        // инициализация формы

    void           settKeyToVal(QSettings *sett);
    void           valToWgt();
    void           wgtToVal();

    void           slotDefaultSettings();       //
    void           defaultSettingsToVal();
    ExchangeParamStruct   findNetConnect_ID(QSettings  *sett);  // нахождение номера DCE_ID в секции команды "EnableDCE"

};

#endif // EXCHAGEDCE_DIAL_H
