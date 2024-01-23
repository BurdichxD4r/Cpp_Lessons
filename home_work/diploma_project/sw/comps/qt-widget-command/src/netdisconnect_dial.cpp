#include "netdisconnect_dial.h"
#include "ui_netdisconnect_dial.h"

#include <QMessageBox>
#include <QIODevice>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QCommonStyle>

//-----------------------------------------------------------------------------
NetDisconnect_dial::NetDisconnect_dial(NetDISCONParamStruct &data_struct, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetDisconnect_dial)
{
    ui->setupUi(this);
    m_discon_param = &data_struct;     // в нашу переменную помещаем адрес внешнего объекта (не выделяем память динамически)

    initForm();                 // инициализация формы

    readSettFromIni();          // данные из файла ini считываются и заносятся на форму после прохождения проверки

    tabOrderSett();             // задание порядка обхода

}
//-----------------------------------------------------------------------------
NetDisconnect_dial::~NetDisconnect_dial()
{
    delete ui;
}
//---------------------------------------------------------------------------------
void NetDisconnect_dial::defaultSettingsToVal()
{
    m_discon_param->dce_id = 1;
    m_discon_param->dce_id_corr = 2;
}
//---------------------------------------------------------------------------------
// поиск без проверки целостности секции "EnableDCE" в ини файле
NetDISCONParamStruct NetDisconnect_dial::findNetConnect_ID(QSettings  *sett)
{
    NetDISCONParamStruct param{0,0}; // значения id по умолчанию
    if(checkDataBeforRead(sett, CONNECT_SECTION_NAME, CRC_CONNECT_SECTION)){
       QStringList group = sett->childGroups();  // получаем список секций в ини-файле
       if (group.size() >0){
          for (int i = 0; i < group.size(); i++ ) {
             sett->beginGroup(group.at(i));
             if (group.at(i) == CONNECT_SECTION_NAME){  // основные настройки команды Enable
             QStringList keys = sett->childKeys();  // получаем список ключей данной секции
             for (int j = 0; j < keys.size(); j++ ) {
                 if (keys.at(j)=="DCE_ID")
                    param.dce_id = sett->value("DCE_ID").toInt();
                 else if (keys.at(j)=="DCE_ID_OPPN")
                     param.dce_id_corr = sett->value("DCE_ID_OPPN").toInt();
             }   // end for keys
          }    // end if disable-param
      sett->endGroup(); // завершение обхода группы "DisableDCE"
    }  // end for group
  }    // end if group size > 0
}
    return param;
}
//---------------------------------------------------------------------------------
// передача данных из объекта Настроек в поле класса
void NetDisconnect_dial::settKeyToVal(QSettings *sett)
{ 
    // уточним значение DCE_ID в секции NetCONNECT
    int param = findNetConnect_ID(sett).dce_id;
    if(param)
      m_discon_param->dce_id = param;

    param = findNetConnect_ID(sett).dce_id_corr;
    if(param)
      m_discon_param->dce_id_corr = param;
}
//---------------------------------------------------------------------------------
// передача значений из виджетов формы в поле класса
void NetDisconnect_dial:: wgtToVal()
{
    m_discon_param->dce_id = ui->sb_DCE_ID->value();
    m_discon_param->dce_id_corr = ui->sb_DCE_ID_OPPN->value();
}
//---------------------------------------------------------------------------------
// передача данных из поля класса на форму
void NetDisconnect_dial::valToWgt()
{
    ui->sb_DCE_ID->setValue(m_discon_param->dce_id);
    ui->sb_DCE_ID_OPPN->setValue(m_discon_param->dce_id_corr);
}
//---------------------------------------------------------------------------------
// считывание данных из файла conf. на форму
void NetDisconnect_dial::readSettFromIni()
{
    QString path_ini = settFileExist();
    if(path_ini!=""){
        QSettings *sett = new QSettings(path_ini, QSettings::IniFormat); // создаем объект настроек(либо считываем данные из файла либо заготавливаем по умолчанию)

        if(checkDataBeforRead(sett, CONNECT_SECTION_NAME, CRC_CONNECT_SECTION)){                // проверка целостности ини файла пройдена

            if(sett->status()==QSettings::NoError){  //  в формате объекта настроек ошибок не обнаружено
               settKeyToVal(sett);                  //  данные из объекта настоек помещаем в поле класса
               valToWgt();                          //  данные из поля класса помещаем на форму
            }
            else{

            }
            ; // !   QMessageBox::critical(this, "Сообщение", "Ошибка в формате файла настроек","");
        }
    }
}
//---------------------------------------------------------------------------------
// запись данных с формы в файл настроек
void NetDisconnect_dial::writeDataToIni()
{
          // создадим объект настроек заново и считаем в него данные из виджетов формы
       QSettings  *sett = new QSettings(getPathToIniFile(), QSettings::IniFormat/*NativeFormat*/);

       sett->remove(SECTION_NAME);
       sett->setValue(SECTION_NAME+"/DCE_ID", ui->sb_DCE_ID->value());
       sett->setValue(SECTION_NAME+"/DCE_ID_OPPN", ui->sb_DCE_ID_OPPN->value());

       unsigned short crc = calcCRCForSettings(sett, SECTION_NAME, CRC_KEY_NAME);
       sett->setValue(SECTION_NAME+"/"+ CRC_KEY_NAME, crc);
}
//---------------------------------------------------------------------------------
void NetDisconnect_dial::initForm()
{
    // настройка ComboBox_regim в зависимости от выбранного этапа

    connect (ui->pb_applay, SIGNAL(clicked()), this, SLOT(slotApplay()));     // кнопка Применить
    connect (ui->pb_cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));     // кнопка ОТМЕНИТЬ

}
//--------------------------------------------------------------------------------------------
// кнопка ПРИНЯТЬ
// здесь наш внешний объект должен быть заполнен целиком
void NetDisconnect_dial::slotApplay()
{
    wgtToVal();
                                    // и надеемся, что ничего не упадет!
   // writeDataToIni();               // кроме этого данные с формы должны быть сохранены в conf файле с настройками

    accept();                       // закрываем форму с флагом ПРИНЯТО
}
//--------------------------------------------------------------------------------------------
//  кнопка ОТМЕНИТЬ
void NetDisconnect_dial::slotCancel()
{
    reject();                      // отменяем все наши действия на форме, выходим с флагом ОТМЕНЕНО
}
//--------------------------------------------------------------------------------------------
// Задание порядка обхода элементов формы
void NetDisconnect_dial::tabOrderSett()
{
    setTabOrder(this, ui->sb_DCE_ID);
    setTabOrder(ui->sb_DCE_ID, ui->sb_DCE_ID_OPPN);
    setTabOrder(ui->sb_DCE_ID_OPPN, ui->pb_applay);
    setTabOrder(ui->pb_applay, ui->pb_cancel);
}
//--------------------------------------------------------------------------------------

