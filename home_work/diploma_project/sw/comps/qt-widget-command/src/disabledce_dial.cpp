#include "disabledce_dial.h"
#include "ui_disabledce_dial.h"
#include <QFile>
#include <QMessageBox>
#include <QIODevice>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QSettings>
#include <QCommonStyle>
#include <QDir>

//-----------------------------------------------------------------------------
DisableDCE_dial::DisableDCE_dial(DisableParamStruct &data_struct,
                                 QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DisableDCE_dial)
{
    ui->setupUi(this);

    m_disable_param = &data_struct;

    initForm();                 // инициализация формы

    readSettFromIni();          // данные из файла ini считываются и заносятся на форму после прохождения проверки

    tabOrderSett();             // задание порядка обхода

}
//-----------------------------------------------------------------------------
DisableDCE_dial::~DisableDCE_dial()
{
    delete ui;
}
//---------------------------------------------------------------------------------
void DisableDCE_dial::defaultSettingsToVal()
{
    m_disable_param->dce_id = 1;
}
//---------------------------------------------------------------------------------
// поиск без проверки целостности секции "EnableDCE" в ини файле
unsigned int DisableDCE_dial::findEnableDCE_ID(QSettings  *sett)
{
    unsigned int param{0};
   if(checkDataBeforRead(sett, ENABLE_SECTION_NAME, CRC_ENABLE_SECTION)){
      QStringList group = sett->childGroups();  // получаем список секций в ини-файле
      if (group.size() >0){
        for (int i = 0; i < group.size(); i++ ) {
          sett->beginGroup(group.at(i));
          if (group.at(i) == ENABLE_SECTION_NAME){  // основные настройки команды Enable
            QStringList keys = sett->childKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
               if (keys.at(j)=="DCE_ID")
                  param = sett->value("DCE_ID").toInt();

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
void DisableDCE_dial::settKeyToVal(QSettings *sett)
{ 
    // уточним значение DCE_ID в секции Enable_DCE
    int param = findEnableDCE_ID(sett);
    if(param)
      m_disable_param->dce_id = param;
}
//---------------------------------------------------------------------------------
// передача значений из виджетов формы в поле класса
void DisableDCE_dial:: wgtToVal()
{
    m_disable_param->dce_id = ui->sb_DCE_ID->value();
}
//---------------------------------------------------------------------------------
// передача данных из поля класса на форму
void DisableDCE_dial::valToWgt()
{
    ui->sb_DCE_ID->setValue(m_disable_param->dce_id);
}
//---------------------------------------------------------------------------------
// считывание данных из файла conf. на форму
void DisableDCE_dial::readSettFromIni()
{
    QString path_ini = settFileExist();
    if(path_ini!=""){
        QSettings *sett = new QSettings(path_ini, QSettings::IniFormat); // создаем объект настроек(либо считываем данные из файла либо заготавливаем по умолчанию)

        if(checkDataBeforRead(sett, SECTION_NAME, CRC_KEY_NAME)){                // проверка целостности ини файла пройдена

            if(sett->status()==QSettings::NoError){  //  в формате объекта настроек ошибок не обнаружено
               settKeyToVal(sett);                  //  данные из объекта настоек помещаем в поле класса
               valToWgt();                          //  данные из поля класса помещаем на форму
            }
            else{
            }
             ; // ! QMessageBox::critical(this, "Сообщение", "Ошибка в формате файла настроек","");
        }   
    }
}
//---------------------------------------------------------------------------------
// запись данных с формы в файл настроек
void DisableDCE_dial::writeDataToIni()
{
     // создадим объект настроек заново и считаем в него данные из виджетов формы
       QSettings  *sett = new QSettings(getPathToIniFile(), QSettings::IniFormat/*NativeFormat*/);

       sett->remove(SECTION_NAME);
       sett->setValue(SECTION_NAME+"/DCE_ID", ui->sb_DCE_ID->value());

       unsigned short crc = calcCRCForSettings(sett, SECTION_NAME, CRC_KEY_NAME);
       sett->setValue(SECTION_NAME+"/"+CRC_KEY_NAME, crc);
}
//---------------------------------------------------------------------------------
void DisableDCE_dial::initForm()
{
    // настройка ComboBox_regim в зависимости от выбранного этапа

    connect (ui->pb_applay, SIGNAL(clicked()), this, SLOT(slotApplay()));     // кнопка Применить
    connect (ui->pb_cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));     // кнопка ОТМЕНИТЬ

}
//--------------------------------------------------------------------------------------------
// кнопка ПРИНЯТЬ
// здесь наш внешний объект должен быть заполнен целиком
void DisableDCE_dial::slotApplay()
{
    wgtToVal();

   // writeDataToIni();               // кроме этого данные с формы должны быть сохранены в conf файле с настройками

    accept();                       // закрываем форму с флагом ПРИНЯТО
}
//--------------------------------------------------------------------------------------------
//  кнопка ОТМЕНИТЬ
void DisableDCE_dial::slotCancel()
{
    reject();                      // отменяем все наши действия на форме, выходим с флагом ОТМЕНЕНО
}
//--------------------------------------------------------------------------------------------
// Задание порядка обхода элементов формы
void DisableDCE_dial::tabOrderSett()
{
    setTabOrder(this, ui->sb_DCE_ID);
    setTabOrder(ui->sb_DCE_ID, ui->pb_applay);
    setTabOrder(ui->pb_applay, ui->pb_cancel);
}
//--------------------------------------------------------------------------------------
