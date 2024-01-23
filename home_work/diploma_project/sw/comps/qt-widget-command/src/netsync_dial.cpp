#include "netsync_dial.h"
#include "ui_netsync_dial.h"
#include <QCommonStyle>
#include <QMessageBox>
#include <QIODevice>
#include <QAction>
#include <QSettings>

//--------------------------------------------------------------------------------------
NetSYNC_dial::NetSYNC_dial(NetSYNCParamStruct &data_struct, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetSYNC_dial)
{
    ui->setupUi(this);
    m_netSYNC_param = &data_struct;     // в нашу переменную помещаем адрес внешнего объекта (не выделяем память динамически)

    initForm();

    readSettFromIni();                  // данные из файла ini считываются и заносятся на форму после прохождения проверки
}
//--------------------------------------------------------------------------------------
void NetSYNC_dial::initForm()
{
    connect (ui->pb_applay, SIGNAL(clicked()), this, SLOT(slotApplay()));     // кнопка Применить
    connect (ui->pb_cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));     // кнопка ОТМЕНИТЬ

    QCommonStyle style;
    ui->pb_default->setIcon(style.standardIcon(QStyle::SP_ArrowBack));
    ui->pb_default->setToolTip("Вернуться к настройкам \n по умолчанию.");
    connect (ui->pb_default, SIGNAL(clicked()), this, SLOT(slotDefaultSettings()));  // кнопка "Значения по умолчанию"

    m_sync_pack.append(ui->sb_pack1);
    m_sync_pack.append(ui->sb_pack2);
    m_sync_pack.append(ui->sb_pack3);
    m_sync_pack.append(ui->sb_pack4);
    m_sync_pack.append(ui->sb_pack5);
    m_sync_pack.append(ui->sb_pack6);
    m_sync_pack.append(ui->sb_pack7);
    m_sync_pack.append(ui->sb_pack8);

    for(int i = 0; i < m_sync_pack.count(); ++i)
    {
        m_sync_pack.at(i)->setMaximum(255);
        m_sync_pack.at(i)->setMinimum(0);
    }
    ui->sb_imit->setMaximum(429493729);
   // ui->sb_immit->setDisplayIntegerBase()
    ui->sb_imit->setMinimum(0);

    tabOrderSett();
}
//--------------------------------------------------------------------------------------
// выполнение настроек по умолчанию
void NetSYNC_dial:: slotDefaultSettings()
{
   defaultSettingsToVal();   // предполагается что параметры по умолчанию корректные (crc не проверяется)
   valToWgt();          // занесение данных из поля класса на форму
   writeDataToIni();    // запись данных с формы в ини файл
}
//--------------------------------------------------------------------------------------
NetSYNC_dial::~NetSYNC_dial()
{
    delete ui;
}
//---------------------------------------------------------------------------------
// считывание данных из файла conf. на форму
void NetSYNC_dial::readSettFromIni()
{
    QString path_ini = settFileExist();
    if(path_ini!=""){ // файл настроек существует
        QSettings *sett = new QSettings(path_ini, QSettings::IniFormat); // создаем объект настроек(либо считываем данные из файла либо заготавливаем по умолчанию)

        if(checkDataBeforRead(sett, SECTION_NAME, CRC_KEY_NAME)){     // проверка целостности ини файла пройдена (файл имеется и имеется секция команды)

            if(sett->status()==QSettings::NoError){  //  в формате объекта настроек ошибок не обнаружено
               settKeyToVal(sett);                  //  данные из объекта настоек помещаем в поле класса
               valToWgt();                          //  данные из поля класса помещаем на форму
            }
            else
               QMessageBox::critical(this, "Сообщение", "Ошибка в формате файла настроек","");
        }

       else {    // проверка целостности ини файла не пройдена (файл ини имеется, но нет секции файла или crc не совпадает)
          // ! QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует или поврежден.\n Будут загружены данные по умолчанию.","");  // предупредим
           defaultSettingsToVal();     // перезапишем данными по умолчанию
           int param = findEnableDCE_ID(sett);
           if(param){
             m_netSYNC_param->dce_id = param;  // если секция EnableDCE не найдена или повреждена, значение параметра останется по умолчанию
             m_netSYNC_param->dce_id_dst = param + 1;
           }
           valToWgt();                 // в виджет
           writeDataToIni();           // и сохраним в файл
       }
    }
    else{   // если файл с настройками не существует
        QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует .\n Будут загружены данные по умолчанию.","");  // предупредим
        defaultSettingsToVal();
        valToWgt();                 // в виджет
        writeDataToIni();           // и сохраним в файл
    }
}
//---------------------------------------------------------------------------------
// запись данных с формы в файл настроек
void NetSYNC_dial::writeDataToIni()
{
       // создадим объект настроек заново и считаем в него данные из виджетов формы
       QSettings  *sett = new QSettings(getPathToIniFile(), QSettings::IniFormat/*NativeFormat*/);

       sett->remove( SECTION_NAME);
       sett->setValue( SECTION_NAME+"/DCE_ID", ui->sb_DCE_ID->value());
       sett->setValue( SECTION_NAME+"/DCE_ID_OPPN", ui->sb_DCE_ID_2->value());
       sett->setValue( SECTION_NAME+"/ImitINS", ui->sb_imit->value());

       sett->beginWriteArray(SECTION_NAME+"/synchroPack");
       int size = m_sync_pack.count();
       for(int i = 0; i < size; ++i){
          sett->setArrayIndex(i);
          QString val = m_sync_pack.at(i)->text();
          sett->setValue("val", val);
       }
       sett->endArray();

       unsigned short crc = calcCRCForSettings(sett,SECTION_NAME,CRC_KEY_NAME);
       sett->setValue(SECTION_NAME + "/" + CRC_KEY_NAME, crc);

}

//---------------------------------------------------------------------------------
// передача данных из поля класса на форму
void NetSYNC_dial::valToWgt()
{
    ui->sb_DCE_ID->setValue(m_netSYNC_param->dce_id);
    ui->sb_DCE_ID_2->setValue(m_netSYNC_param->dce_id_dst);
    ui->sb_imit->setValue(m_netSYNC_param->imitIns);
    for(int i = 0; i < m_netSYNC_param->syncPack.count(); ++i){
        if (i < m_netSYNC_param->syncPack.count())
           m_sync_pack.at(i)->setValue(m_netSYNC_param->syncPack.at(i));
     }
}
//---------------------------------------------------------------------------------
// передача данных из объекта Настроек в поле класса
void NetSYNC_dial::settKeyToVal(QSettings *sett)
{

    QStringList group = sett->childGroups();  // получаем список секций в ини-файле
    if (group.size() >0){
      for (int i = 0; i < group.size(); i++ ) {
          sett->beginGroup(group.at(i));
          if (group.at(i) == SECTION_NAME){  // основные настройки команды Enable
            QStringList keys = sett->childKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
               if (keys.at(j)=="DCE_ID")
                  m_netSYNC_param->dce_id = sett->value("DCE_ID").toInt();/*findEnableDCE_ID(sett);*/   //sett->value("DCE_ID").toInt();
               else if (keys.at(j)=="DCE_ID_OPPN")
                  m_netSYNC_param->dce_id_dst = sett->value("DCE_ID_OPPN").toInt();
               else if (keys.at(j)=="ImitINS")
                  m_netSYNC_param->imitIns = sett->value("ImitINS").toInt();

               int size = sett->beginReadArray("synchroPack");
               m_netSYNC_param->syncPack.clear();
               for(int j = 0; j <size; ++j){
                   sett->setArrayIndex(j);
                   m_netSYNC_param->syncPack.append(sett->value("val").toInt());
               }
               sett->endArray();
            }   // end for keys
          }    // end if disable-param

      sett->endGroup(); // завершение обхода группы "NetSYNC"
    }  // end for group
  }    // end if group size > 0

    // уточним значение DCE_ID в секции Enable_DCE
    int param = findEnableDCE_ID(sett);
    if(param)
      m_netSYNC_param->dce_id = param;
}
//---------------------------------------------------------------------------------
// поиск без проверки целостности секции "EnableDCE" в ини файле
unsigned int NetSYNC_dial::findEnableDCE_ID(QSettings  *sett)
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

       sett->endGroup(); // завершение обхода группы
     }  // end for group
    }    // end if group size > 0
  }
    return param;
}
//---------------------------------------------------------------------------------
void NetSYNC_dial::defaultSettingsToVal()
{
    m_netSYNC_param->dce_id = 1;
    m_netSYNC_param->dce_id_dst = m_netSYNC_param->dce_id + 1;
    m_netSYNC_param->imitIns = 1254567;
    m_netSYNC_param->syncPack.clear();
    m_netSYNC_param->syncPack.append(200);
    m_netSYNC_param->syncPack.append(35);
}
//---------------------------------------------------------------------------------------
// кнопка ПРИНЯТЬ
// здесь наш внешний объект должен быть заполнен целиком
void NetSYNC_dial::slotApplay()
{
    wgtToVal();

    writeDataToIni();               // кроме этого данные с формы должны быть сохранены в conf файле с настройками

    accept();                       // закрываем форму с флагом ПРИНЯТО
}
//---------------------------------------------------------------------------------
// передача значений из виджетов формы в поле класса
void NetSYNC_dial:: wgtToVal()
{
   m_netSYNC_param->dce_id = ui->sb_DCE_ID->value();
   m_netSYNC_param->dce_id_dst = ui->sb_DCE_ID_2->value();
   m_netSYNC_param->imitIns = ui->sb_imit->value();
   m_netSYNC_param->syncPack.clear();
   for (int i = 0; i < m_sync_pack.count(); ++i)
       m_netSYNC_param->syncPack.append(m_sync_pack.at(i)->value());
}

//--------------------------------------------------------------------------------------------
//  кнопка ОТМЕНИТЬ
void NetSYNC_dial::slotCancel()
{
    reject();                      // отменяем все наши действия на форме, выходим с флагом ОТМЕНЕНО
}
//--------------------------------------------------------------------------------------------
// Задание порядка обхода элементов формы
void NetSYNC_dial::tabOrderSett()
{
    setTabOrder(this, ui->pb_default);
    setTabOrder(ui->pb_default, ui->sb_DCE_ID);
    setTabOrder(ui->sb_DCE_ID, ui->sb_DCE_ID_2);
    setTabOrder(ui->sb_DCE_ID_2, ui->sb_imit);
    setTabOrder(ui->sb_imit, m_sync_pack.at(0));

    for (int i  = 0; i < m_sync_pack.count()-1; ++i){
       setTabOrder(m_sync_pack.at(i), m_sync_pack.at(i+1));
    }
    setTabOrder(m_sync_pack.at(m_sync_pack.count()-1), ui->pb_applay);
    setTabOrder(ui->pb_applay, ui->pb_cancel);
}
//----------------------------------------------------------------------------------------
