#include "exchagedce_dial.h"
#include "ui_exchagedce_dial.h"
#include <QFile>
#include <QMessageBox>
#include <QIODevice>
#include <QApplication>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QUuid>
#include <QJsonObject>
#include <QSettings>
#include <QCommonStyle>
#include <QJsonArray>
#include <QDir>

//----------------------------------------------------------------------------------------------------
ExchageDCE_dial::ExchageDCE_dial(ExchangeParamStruct &data_struct,int & lett_pass, int mode, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ExchageDCE_dial)
{
    ui->setupUi(this);

    m_exchange_param = &data_struct;     // в нашу переменную помещаем адрес внешнего объекта (не выделяем память динамически)
    m_lett_pass = &lett_pass;
    m_mode_exch = mode;


    //  connect(this, &MainWindDTE::sigWriteInJourn, ui->tw_mess, &WidgetMess::slotAddMsg);
   // connect (ui->pb_Applay, &QPushButton::clicked, this, &ExchageDCE_dial::slotApplay);     // кнопка Применить
   // connect (ui->pb_Cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));                   // кнопка ОТМЕНИТЬ
    connect (ui->sbB_length_parity, SIGNAL(valueChanged(int)), this,SLOT(slotDataParityLengthChange(int)) );
    connect (ui->sbB_length_general, SIGNAL(valueChanged(int)), this,SLOT(slotDataGeneralLengthChange(int)) );
    connect (ui->sbB_length_noparity, SIGNAL(valueChanged(int)), this,SLOT(slotDataNoParityLengthChange(int)) );
    connect (ui->sbB_ampl_parity, SIGNAL(valueChanged(int)), this,SLOT(slotDataParityAmplChange(int)) );
    connect (ui->sbB_ampl_noparity, SIGNAL(valueChanged(int)), this,SLOT(slotDataNoParityAmplChange(int)) );
    connect (ui->chB_err_add, SIGNAL(clicked(bool)), this,SLOT(slotErrAdd(bool)) );

    initForm();                 // инициализация формы
    readSettFromIni();          // данные из файла ini считываются и заносятся на форму после прохождения проверки
    tabOrderSett();             // задание порядка обхода

}

//---------------------------------------------------------------------------------
void ExchageDCE_dial::initForm()
{
    // настройка ComboBox_regim в зависимости от выбранного этапа

    connect (ui->pb_Applay, SIGNAL(clicked()), this, SLOT(slotApplay()));     // кнопка Применить
    connect (ui->pb_Cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));     // кнопка ОТМЕНИТЬ
    m_test_ba_parity.clear();
    m_test_ba_noparity.clear();

    int n{0};
    if (m_mode_exch == 0){
        n = 32;   // для теста
        ui->widget_parity->setVisible(true);
        ui->widget_no_parity->setVisible(true);
        ui->widget_normal->setFixedWidth(0);
        ui->widget_normal->setVisible(false);
        m_data_ampl_parity = ui->sbB_ampl_parity->text().toInt();
        m_data_ampl_noparity = ui->sbB_ampl_noparity->text().toInt();
        ui->sbB_length_parity->setValue(0);
        ui->sbB_length_noparity->setValue(0);
        ui->sbB_length_parity->setValue(n);
        ui->sbB_length_noparity->setValue(n);
        ui->cb_type->setCurrentIndex(0); // Test


        //ui->sbB_length->setEnabled(false);
     //   ui->cb_type->setEnabled(false);
    }

    else{
        n = 7;  // передача инфы min знач. по умолч
        ui->widget_parity->setVisible(false);
        ui->widget_no_parity->setVisible(false);
        ui->widget_normal->setVisible(true);
        ui->sbB_length_general->setValue(n);

        ui->cb_type->setCurrentIndex(1); // Raw
        ui->cb_type->setEnabled(true);
    }
}
//-----------------------------------------------------------------------------------
void ExchageDCE_dial::slotErrAdd(bool)
{
    int n = m_data_len_parity;
    ui->sbB_length_parity->setValue(1);
    ui->sbB_length_parity->setValue(n);
   // ui->sbB_length_parity->setValue(n);
}
//-----------------------------------------------------------------------------------
void ExchageDCE_dial::slotDataParityAmplChange(int ampl)
{
    m_data_ampl_parity = ampl;
    m_test_ba_parity.clear();
    ui->te_Data_parity->clear();
    QByteArray ba;
    ba.append(52);
    for(int i = 0; i < m_data_len_parity; ++i){
         m_test_ba_parity.append(m_data_ampl_parity);
    }
  //  m_test_ba_parity.insert(m_data_len_parity/2,52); // для проверки подсчета ошибок
    ui->te_Data_parity->setPlainText(m_test_ba_parity.toBase64());

}
//------------------------------------------------------------------------------------------------------
void ExchageDCE_dial::slotDataNoParityAmplChange(int ampl)
{
    m_data_ampl_noparity = ampl;
    m_test_ba_noparity.clear();
    ui->te_Data_noparity->clear();

    for(int i = 0; i < m_data_len_noparity; ++i){
         m_test_ba_noparity.append(m_data_ampl_noparity);
    }
   // ui->te_Data_noparity->setPlainText(m_test_ba_noparity.toHex());
    ui->te_Data_noparity->setPlainText(m_test_ba_noparity.toBase64());
}
//------------------------------------------------------------------------------------------------------
void ExchageDCE_dial::slotDataParityLengthChange(int len)
{
    m_data_len_parity = len;
    m_test_ba_parity.clear();
    ui->te_Data_parity->clear();
    QByteArray ba;
    ba.append(52);
    for(int i = 0; i < m_data_len_parity; ++i){
         m_test_ba_parity.append(m_data_ampl_parity);
    }
    if(ui->chB_err_add->isChecked()){
       m_test_ba_parity.remove(len/2,1);
       m_test_ba_parity.insert(len/2,52);    // для проверки тестового режима (как бы поврежденный байт)
    }
//    m_test_ba_parity.insert(len/2,52);    // для проверки тестового режима (как бы поврежденный байт)
  //  ui->te_Data_parity->setPlainText(m_test_ba_parity.toHex());
    ui->te_Data_parity->setPlainText(m_test_ba_parity.toBase64());

}
//------------------------------------------------------------------------------------------------------
void ExchageDCE_dial::slotDataNoParityLengthChange(int len)
{
    m_data_len_noparity = len;
    m_test_ba_noparity.clear();
    ui->te_Data_noparity->clear();

    for(int i = 0; i < m_data_len_noparity; ++i){
         m_test_ba_noparity.append(m_data_ampl_noparity);
    }
    //ui->te_Data_noparity->setPlainText(m_test_ba_noparity.toHex());
    ui->te_Data_noparity->setPlainText(m_test_ba_noparity.toBase64());
}
//------------------------------------------------------------------------------------------------------
void ExchageDCE_dial::slotDataGeneralLengthChange(int len)
{
    Q_UNUSED(len);
    ui->te_Data_general->clear();
    m_exch_ba_general.clear();
    m_exch_ba_general.append(0x7F);
    m_exch_ba_general.append(0xFE);
    m_exch_ba_general.append(0x44);
    m_exch_ba_general.append(0xF6);
    m_exch_ba_general.append(0xDB);
    m_exch_ba_general.append(0xFF);
    m_exch_ba_general.append(0xFF);


  //  for(int i = 100; i < (100+len); ++i){
  //      m_exch_ba_general.append(i);
  // }
  //  ui->te_Data_general->setPlainText(m_exch_ba_general.toHex());
    ui->te_Data_general->setPlainText(m_exch_ba_general.toBase64());
}

//------------------------------------------------------------------------------------------------------
ExchageDCE_dial::~ExchageDCE_dial()
{
    delete ui;
}
//---------------------------------------------------------------------------------
void ExchageDCE_dial::defaultSettingsToVal()
{
    m_exchange_param->dce_id = 1;
    m_exchange_param->dce_id_corr = 2;
    m_exchange_param->msg_id = 1;
}
//---------------------------------------------------------------------------------
// поиск без проверки целостности секции "EnableDCE" в ини файле
ExchangeParamStruct ExchageDCE_dial::findNetConnect_ID(QSettings  *sett)
{
    ExchangeParamStruct param;

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
    return param;
}

//---------------------------------------------------------------------------------
// передача данных из объекта Настроек в поле класса
void ExchageDCE_dial::settKeyToVal(QSettings *sett)
{
    QStringList group = sett->childGroups();  // получаем список секций в ини-файле
    if (group.size() >0){
      for (int i = 0; i < group.size(); i++ ) {
          sett->beginGroup(group.at(i));
          if (group.at(i) == SECTION_NAME){  // основные настройки команды Enable
            QStringList keys = sett->childKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
               if (keys.at(j)=="DCE_ID")
                  m_exchange_param->dce_id = findNetConnect_ID(sett).dce_id;//sett->value("DCE_ID").toInt();
               else if (keys.at(j)=="DCE_ID_OPPN")
                  m_exchange_param->dce_id_corr = findNetConnect_ID(sett).dce_id_corr;//sett->value("DCE_ID").toInt();

            }   // end for keys
          }     // end if disable-param

      sett->endGroup(); // завершение обхода группы "NetDisconnect-param"
    }  // end for group
  }    // end if group size > 0

    // уточним значение DCE_ID в секции NetCONNECT
    int param = findNetConnect_ID(sett).dce_id;
    if(param)
      m_exchange_param->dce_id = param;
    param = findNetConnect_ID(sett).dce_id_corr;
    if(param)
      m_exchange_param->dce_id_corr = param;
}
//---------------------------------------------------------------------------------
// передача значений из виджетов формы в поле класса
void ExchageDCE_dial:: wgtToVal()
{
    m_exchange_param->dce_id = ui->sb_DCE_ID->value();
    m_exchange_param->dce_id_corr = ui->sb_DCE_ID_OPPN->value();
    m_exchange_param->type = m_mode_exch;

    if(m_mode_exch == 0) {// тестовый режим
      // m_exchange_param->data_parity = m_test_ba_parity.toBase64().data();
      // m_exchange_param->data_noparity = m_test_ba_noparity.toBase64().data();
        m_exchange_param->data_parity = m_test_ba_parity.data();
        m_exchange_param->data_noparity = m_test_ba_noparity.data();
    }
    else if (m_mode_exch == 1) {// режим передачи данных raw
     //  m_exchange_param->data_general = m_exch_ba_general.toBase64().data();
        m_exchange_param->data_general = m_exch_ba_general.data();
    }
}
//---------------------------------------------------------------------------------
// передача данных из поля класса на форму
void ExchageDCE_dial::valToWgt()
{
    ui->sb_DCE_ID->setValue(m_exchange_param->dce_id);
    ui->sb_DCE_ID_OPPN->setValue(m_exchange_param->dce_id_corr); 
}
//---------------------------------------------------------------------------------
// считывание данных из файла conf. на форму
void ExchageDCE_dial::readSettFromIni()
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
                ;
              // ! QMessageBox::critical(this, "Сообщение", "Ошибка в формате файла настроек","");
        }

       else {    // проверка целостности ини файла не пройдена (или файл отсутствует)
       // !    QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует или поврежден.\n Будут загружены данные по умолчанию.","");  // предупредим
           defaultSettingsToVal();     // перезапишем данными по умолчанию
           valToWgt();                 // в виджет
           writeDataToIni();           // и сохраним в файл
       }
    }
    else
        defaultSettingsToVal();
}

//---------------------------------------------------------------------------------
// запись данных с формы в файл настроек
void ExchageDCE_dial::writeDataToIni()
{
          // создадим объект настроек заново и считаем в него данные из виджетов формы
     QSettings  *sett = new QSettings(getPathToIniFile(), QSettings::IniFormat/*NativeFormat*/);

     sett->remove(SECTION_NAME);
     sett->setValue(SECTION_NAME+"/DCE_ID", ui->sb_DCE_ID->value());
     sett->setValue(SECTION_NAME+"/DCE_ID_OPPN", ui->sb_DCE_ID_OPPN->value());

     unsigned short crc = calcCRCForSettings(sett, SECTION_NAME, CRC_KEY_NAME);
     sett->setValue(SECTION_NAME+"/"+ CRC_KEY_NAME, crc);
}
//--------------------------------------------------------------------------------------------
// кнопка ПРИНЯТЬ
// здесь наш внешний объект должен быть заполнен целиком
void ExchageDCE_dial::slotApplay()
{
    m_exch_ba_general.clear();
    m_exch_ba_general.append(ui->te_Data_general->document()->toPlainText().toUtf8());

    m_test_ba_noparity.clear();
    m_test_ba_noparity.append(ui->te_Data_noparity->document()->toPlainText().toUtf8());

    m_test_ba_parity.clear();
    m_test_ba_parity.append(ui->te_Data_parity->document()->toPlainText().toUtf8());

    wgtToVal();                     // и надеемся, что ничего не упадет!
    writeDataToIni();               // кроме этого данные с формы должны быть сохранены в conf файле с настройками
    accept();                       // закрываем форму с флагом ПРИНЯТО
    //done(3);  // Автоматизация!
}
//--------------------------------------------------------------------------------------------
//  кнопка ОТМЕНИТЬ
void ExchageDCE_dial::slotCancel()
{
    reject();                      // отменяем все наши действия на форме, выходим с флагом ОТМЕНЕНО
}
//--------------------------------------------------------------------------------------------
// Задание порядка обхода элементов формы
void ExchageDCE_dial::tabOrderSett()
{
    setTabOrder(this, ui->sb_DCE_ID);
    setTabOrder(ui->sb_DCE_ID, ui->sb_DCE_ID_OPPN);
    setTabOrder(ui->sb_DCE_ID_OPPN, ui->cb_type);
    setTabOrder(ui->cb_type, ui->sbB_length_parity);
    setTabOrder(ui->sbB_length_parity, ui->sbB_length_noparity);
    setTabOrder(ui->sbB_length_noparity,ui->pb_Applay);
    setTabOrder(ui->pb_Applay, ui->pb_Cancel);
}
//--------------------------------------------------------------------------------------
void ExchageDCE_dial::showEvent(QShowEvent *ev){
    Q_UNUSED(ev);
     //slotApplay();  // убрать после автоматизации
}
//--------------------------------------------------------------------------------------


