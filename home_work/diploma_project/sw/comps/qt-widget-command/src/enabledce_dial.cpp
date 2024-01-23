// обработка 33 полей

#include "enabledce_dial.h"
#include "ui_enabledce_dial.h"
#include <QTableWidget>
#include <QListView>
#include <QStringListModel>
#include <QStandardItemModel>
#include <QStandardItem>
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
#include <QVector>
#include <QApplication>
#include <QDesktopWidget>
#include <QScreen>

QString     SECTION_NAME = "Enable-param";
QString     CRC_KEY_NAME = "crc_enableDCE";

extern QStringList REGIM;
#define PERCENT_OF_SCREEN (25 / 100)
/*
// необходимо соответствие с ModeSwitchEnum, ModeExchEnum, ModeOffOnEnum из enums_list.h (fhss-types компоненты)
QStringList REGIM      = {"ФРЧ","НРЧ","ППРЧ"};
QStringList REGIM_EXCH = {"HALF_DUPLEX", "DUPLEX", "SIMPLEX"};
QStringList ADAPTATION = {"OFF", "ON"};
*/
//---------------------------------------------------------------------------------
EnableDCE_dial::EnableDCE_dial(EnableParamStruct &data_struct, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::EnableDCE_dial)
{
    m_stage = INIT_FORM;        // переда инициализацией форма находится в стадии INIT (для корректной загрузке параметров группы кол-во частот)
    ui->setupUi(this);  
    m_mas_port = data_struct.param_port;   // сохраним переданный из главной формы порта для дальнейшего анализа
                                             // и использования при формировани команды
    m_enable_param = &data_struct;

    // определим размер окна диалога
    QScreen* screen = QApplication::screens().at(0);
    QSize size = screen->availableSize();
    this->setFixedHeight(4*size.height()/5);

    initForm();                 // инициализация формы
    readSettFromIni();          // данные из файла ini считываются и заносятся на форму после прохождения проверки
    slotRCSett();               // настройка вкладки радиосредства
    for (int i = 0; i < m_vec_widg_rc.count(); ++i){
         connect(m_vec_widg_rc.at(i), SIGNAL(changeRadioName()), this, SLOT(slotRCSett()));
    }
    slotDHS_bw();               // настройка группы режима, Уточнение режима
    tabOrderSett();             // задание порядка обхода
    m_stage = WORK;             // после инициализации(загрузке параметров из ини файла, форма переходит в состояние WORK)
}
//---------------------------------------------------------------------------------
EnableDCE_dial::~EnableDCE_dial()
{
    delete ui;
}
//---------------------------------------------------------------------------------
void EnableDCE_dial:: defaultSettingsToVal()
{
    m_stage = INIT_FORM;      // режим инициализации
    m_enable_param->dce_id = 1;
    m_enable_param->mode_bw = 3100;
    m_enable_param->modeSw = FRCH;
    m_enable_param->tm_switch = 200;
    m_enable_param->modeEx = HALF_DUPLEX;
    m_enable_param->stage = 0;
    m_enable_param->freq_discr = 8000;
    m_enable_param->mode_var = 5;

    m_enable_param->freq_list.clear();
    m_enable_param->use_freq_list = true;
    m_enable_param->freq_list.append(4000000);
    m_enable_param->freq_list.append(5000000);
    m_enable_param->freq_list.append(6000000);
    m_enable_param->freq_list.append(7000000);

    m_enable_param->freq_list_prim.clear();
    for (int i  = 0; i < 8; ++i)
    {
        m_enable_param->freq_list_prim.append(i+1);
    }

    m_enable_param->ppf_ind_list.clear();
    for (int i  = 0; i < 3; ++i)
    {
        m_enable_param->ppf_ind_list.append(i+1);
    }

    m_enable_param->input_port_list.append(57501);
    m_enable_param->input_port_list.append(57502);    

    m_enable_param->ppf_number = 2;
    m_enable_param->ppf_switch ="auto";
    m_enable_param->radio_dir_name = "456:RX:0:1";
    m_enable_param->output_port = 4001;
    m_enable_param->output_adr ="127.0.0.2";
    m_enable_param->rcvr_adr = "0.0.0.0";
    m_enable_param->rcvr_port = 0;

    //m_enable_param->rcvr_prot ="HTTP";
    //m_enable_param->output_prot ="JSON";
    //m_enable_param->input_prot ="RTP";

    m_enable_param->freq_list_add.clear();
    for (int i  = 0; i < 3; ++i)
    {
        m_enable_param->freq_list_add.append(i+1);
    }
    m_enable_param->fr_offset = 1;
    m_enable_param->fr_active = 1;
    m_enable_param->fr_all = m_enable_param->freq_list.count();
    m_enable_param->fr_adapt = OFF;
    m_enable_param->cons_adapt = OFF;

    m_enable_param->pcp_key = 2345;
    m_enable_param->initPcpTime = 5555;
    m_enable_param->modeFreqPcp = 5;

    RadioParams rp;

    rp.name = "111:RX";
    rp.ipv4 = "127.0.0.1";
    rp.tcp_port = 1;
    rp.udp_port1 = 1;
    rp.udp_port2 = 1;
    m_enable_param->radio_param.clear();
    m_enable_param->radio_param.append(rp);
    m_rc_count = 0;      // счетчик  радиосредств для отображения названия вкладки

    m_stage = WORK;      // окончание режима инициализации
}
//---------------------------------------------------------------------------------
// выполнение настроек по умолчанию
void EnableDCE_dial:: slotDefaultSettings()
{
   defaultSettingsToVal();   // предполагается что параметры по умолчанию корректные (crc не проверяется)
   valToWgt();          // занесение данных из поля класса на форму
   writeDataToIni();    // запись данных с формы в ини файл
}
//---------------------------------------------------------------------------------
// передача данных из объекта Настроек в поле класса
void EnableDCE_dial::settKeyToVal(QSettings *sett)
{
    QStringList group = sett->childGroups();  // получаем список секций в ини-файле

    if (group.size() >0){
      for (int i = 0; i < group.size(); i++ ) {
          sett->beginGroup(group.at(i));
          if (group.at(i) == "Enable-param"){  // основные настройки команды Enable
            QStringList keys = sett->childKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
               if (keys.at(j)=="DCE_ID")
                  m_enable_param->dce_id = sett->value("DCE_ID").toInt();
               else if (keys.at(j)=="bandWidth")
                  m_enable_param->mode_bw = sett->value("bandWidth").toInt();
               else if (keys.at(j)=="stage")
                  m_enable_param->stage =  sett->value("stage").toInt();
               else if (keys.at(j)=="mode_var")
                  m_enable_param->mode_var =  sett->value("mode_var").toInt();
               else if (keys.at(j)=="freq_discr")
                  m_enable_param->freq_discr =  sett->value("freq_discr").toInt();
               else if (keys.at(j)=="change_time")
                   m_enable_param->tm_switch = sett->value("change_time").toInt();
               else if (keys.at(j)=="mode")
                   m_enable_param->modeSw = getModeSwitch(sett->value("mode").toString());
               else if (keys.at(j)=="mode2")
                   m_enable_param->modeEx = getModeExch(sett->value("mode2").toInt());
               else if (keys.at(j)=="freq_offset"){
                   int val = sett->value("freq_offset").toInt();
                   m_enable_param->fr_offset = val;
               }
               else if (keys.at(j)=="freq_active"){
                   int val = sett->value("freq_active").toInt();
                   m_enable_param->fr_active = val;
               }
               else if (keys.at(j)=="pcp_key"){
                  m_enable_param->pcp_key = sett->value("pcp_key").toInt();
               }
               else if (keys.at(j)=="init_time_pcp"){
               m_enable_param->initPcpTime = sett->value("init_time_pcp").toInt();
               }
               else if (keys.at(j)=="mode_fr_pcp"){
               m_enable_param->modeFreqPcp = sett->value("mode_fr_pcp").toInt();
               }
               else if (keys.at(j)=="freq_all"){
                   m_enable_param->fr_all = sett->value("freq_all").toInt();
               }
               else if (keys.at(j)=="freq_adaptation"){
                   m_enable_param->fr_adapt = getModeAdapt(sett->value("freq_adaptation").toInt());
               }
               else if (keys.at(j)=="cons_adaptation"){
                   m_enable_param->cons_adapt = getModeAdapt(sett->value("cons_adaptation").toInt());
               }
               else if (keys.at(j) == "use_freq_list"){
                   m_enable_param->use_freq_list = sett->value("use_freq_list").toBool();
               }
               /////новые поля
               else if (keys.at(j) == "ppf_number"){
                   m_enable_param->ppf_number = sett->value("ppf_number").toInt();
               }
               else if (keys.at(j)=="ppf_switch"){
                   m_enable_param->ppf_switch = sett->value("ppf_switch").toString();
               }
               else if (keys.at(j)=="output_adr"){
                m_enable_param->output_adr = sett->value("output_adr").toString();
               }
               else if (keys.at(j)=="rcvr_adr"){
                  m_enable_param->rcvr_adr = sett->value("rcvr_adr").toString();
               }
               else if (keys.at(j) == "rcvr_port"){
                  m_enable_param->rcvr_port = sett->value("rcvr_port").toInt();
               }
               else if (keys.at(j) == "output_port"){
                  m_enable_param->output_port = sett->value("output_port").toInt();
               }
              else if (keys.at(j)=="input_prot"){
                m_enable_param->input_prot = sett->value("input_prot").toString();
              }
              else if (keys.at(j)=="output_prot"){
                m_enable_param->output_prot = sett->value("output_prot").toString();
              }
              else if (keys.at(j)=="rvcr_prot"){
                m_enable_param->rcvr_prot = sett->value("rcvr_prot").toString();
              }
              else if (keys.at(j)=="radio_dir_name"){
                m_enable_param->radio_dir_name = sett->value("radio_dir_name").toString();
              }
           } // end for keys

    // читаем  массив -вход. порты
    int size_port = sett->beginReadArray("input_port_list");
    m_enable_param->input_port_list.clear();
    for(int j = 0; j <size_port; ++j){
     sett->setArrayIndex(j);
     m_enable_param->input_port_list.append(sett->value("num").toInt());
    }
    sett->endArray();

      // читаем  массив -индексы частот
    int size_ind = sett->beginReadArray("ppf_ind_list");
    m_enable_param->ppf_ind_list.clear();
    for(int j = 0; j <size_ind; ++j){
      sett->setArrayIndex(j);
      m_enable_param->ppf_ind_list.append(sett->value("num").toInt());
     }
    sett->endArray();
////////новые поля окончание

            // читаем  массив -список частот
         int size = sett->beginReadArray("freq-list");
         m_enable_param->freq_list.clear();
         for(int j = 0; j <size; ++j){
             sett->setArrayIndex(j);
             m_enable_param->freq_list.append(sett->value("num").toInt());
         }
         sett->endArray();

         // читаем  массив -список частот НПП
        size = sett->beginReadArray("freq-list_prim");
        m_enable_param->freq_list_prim.clear();
        for(int j = 0; j < size; ++j){
          sett->setArrayIndex(j);
          m_enable_param->freq_list_prim.append(sett->value("num").toInt());
        }
        sett->endArray();

        // читаем  массив -список частот ЗПЧ
        size = sett->beginReadArray("freq-list_add");
        m_enable_param->freq_list_add.clear();
        for(int j = 0; j < size; ++j){
           sett->setArrayIndex(j);
           m_enable_param->freq_list_add.append(sett->value("num").toInt());
        }
        sett->endArray();

             // читаем массив - радио параметры
         int size1 = sett->beginReadArray("radio-params");
         m_enable_param->radio_param.clear();
         for (int j = 0; j < size1; ++j){
             sett->setArrayIndex(j);
             RadioParams rp;
             QString r_name;
             r_name = sett->value("radio_name").toString();
             rp.name = r_name;
             rp.ipv4 = sett->value("radio_ipv4").toString();
             rp.tcp_port = sett->value("radio_tcp_port").toInt();
             rp.udp_port1 = sett->value("radio_udp_port1").toInt();
             rp.udp_port2 = sett->value("radio_udp_port2").toInt();
             m_enable_param->radio_param.append(rp);
          }
          sett->endArray();
      }    // end if enable-param
      sett->endGroup(); // завершение обхода очередной группы
    }  // end for group
  }    // end if group size > 0
    // всего 19 полей!!!! + нов поля
}

//---------------------------------------------------------------------------------
// передача значений из виджетов формы в поле класса
void EnableDCE_dial:: wgtToVal()
{
    m_enable_param->dce_id = ui->sb_DCE_ID->value();
    m_enable_param->mode_bw = ui->cb_regim_bw->currentText().toInt();
    m_enable_param->mode_var = ui->cb_reg_var->currentText().toInt();       // номер варианта
    m_enable_param->freq_discr = ui->cb_freq_discr->currentText().toInt();  // частота дискр
    m_enable_param->modeSw = getModeSwitch(ui->cb_reg->currentText());//>currentIndex();

    m_enable_param->tm_switch = ui->cb_chng->currentText().toInt();
    m_enable_param->modeEx = getModeExch(ui->rb_half_duplex->isChecked()*0 +
                             ui->rb_duplex->isChecked()*1 +
                             ui->rb_simplex->isChecked()*2);

    m_enable_param->use_freq_list = ui->chb_freq_list_use->isChecked();

    m_enable_param->freq_list.clear();
    for(int i  = 0; i < ui->tw_freq_list->rowCount(); ++i){
       m_enable_param->freq_list.append(ui->tw_freq_list->item(i,0)->text().toInt());
    }

    m_enable_param->freq_list_prim.clear();
    for(int i  = 0; i < ui->tw_freq_list_prim->rowCount(); ++i){
       m_enable_param->freq_list_prim.append(ui->tw_freq_list_prim->item(i,0)->text().toInt());
    }

    m_enable_param->freq_list_add.clear();
    for(int i  = 0; i < ui->tw_freq_list_add->rowCount(); ++i){
       m_enable_param->freq_list_add.append(ui->tw_freq_list_add->item(i,0)->text().toInt());
    }
    m_enable_param->fr_offset = ui->sb_freq_offset->value();
    m_enable_param->fr_active = ui->sb_freq_active->value();
    m_enable_param->fr_all = ui->sb_freq_all->value();
    m_enable_param->pcp_key = ui->sb_pspKey->value();
    m_enable_param->initPcpTime = ui->sb_initTime_psp->value();
    m_enable_param->modeFreqPcp = ui->sb_mode_fr_psp->value();
   // m_enable_param->stage =  оставляем прежним
    m_enable_param->fr_adapt = getModeAdapt(ui->rb_freq_no->isChecked()*0 + ui->rb_freq_yes->isChecked()*1);
    m_enable_param->cons_adapt = getModeAdapt(ui->rb_sign_no->isChecked()*0 + ui->rb_sign_yes->isChecked()*1);
    m_enable_param->ppf_number =ui->sb_ppf_number->value();

    QString radio_role{"TX"};
    if(ui->rb_radio_dir_rx->isChecked())
        radio_role ="RX";

    m_enable_param->radio_dir_name = QString::number(ui->sb_radio_dir_name_1->value())+":" + radio_role +":"+
                                     QString::number(ui->sb_radio_dir_name_3->value())+":" +
                                     QString::number(ui->sb_radio_dir_name_4->value());
    m_enable_param->ppf_switch = ui->cb_ppf_switch->currentText();
    m_enable_param->input_prot= ui->cb_in_prot->currentText();
    m_enable_param->output_prot= ui->cb_out_prot->currentText();
    m_enable_param->rcvr_prot= ui->cb_rcvr_prot->currentText();
    m_enable_param->output_port=ui->sb_out_port->value();
    m_enable_param->output_adr=ui->le_out_adr->text();
    m_enable_param->rcvr_adr=ui->le_rcvr_adr->text();

    m_enable_param->input_port_list.clear();
    for(int i  = 0; i < ui->tw_input_port->rowCount(); ++i){
       m_enable_param->input_port_list.append(ui->tw_input_port->item(i,0)->text().toInt());
    }

    m_enable_param->ppf_ind_list.clear();
    for(int i  = 0; i < ui->tw_ppf_ind->rowCount(); ++i){
       m_enable_param->ppf_ind_list.append(ui->tw_ppf_ind->item(i,0)->text().toInt());
    }

    m_enable_param->radio_param.clear();
    for(int i = 0; i < m_vec_widg_rc.count(); ++i){
        RadioParams rp;
        if(m_vec_widg_rc.at(i)->m_rb_rx->isChecked()){
           rp.name = m_vec_widg_rc.at(i)->m_le_name->text() + ":RX";
        }
        else
           rp.name = m_vec_widg_rc.at(i)->m_le_name->text() +":TX";
        rp.ipv4 = m_vec_widg_rc.at(i)->m_le_ip->text();
        rp.tcp_port = m_vec_widg_rc.at(i)->m_sp_tcp->value();
        rp.udp_port1 = m_vec_widg_rc.at(i)->m_sp_udp_1->value();
        rp.udp_port2 = m_vec_widg_rc.at(i)->m_sp_udp_2->value();

        m_enable_param->radio_param.append(rp);
    }
}
//---------------------------------------------------------------------------------
// передача данных из поля класса на форму
void EnableDCE_dial::valToWgt()
{
    ui->sb_DCE_ID->setValue(m_enable_param->dce_id);
    ui->cb_regim_bw->setCurrentText(QString::number(m_enable_param->mode_bw));
    ui->cb_reg_var->setCurrentText(QString::number(m_enable_param->mode_var));
    ui->cb_freq_discr->setCurrentText(QString::number(m_enable_param->freq_discr));
    ui->cb_chng->setCurrentText(QString::number(m_enable_param->tm_switch));
    ui->cb_reg->setCurrentIndex(m_enable_param->modeSw);

    switch (m_enable_param->modeEx){
      case SIMPLEX : ui->rb_simplex->setChecked(true);
            break;
      case HALF_DUPLEX : ui->rb_half_duplex->setChecked(true);
            break;
      case DUPLEX : ui->rb_duplex->setChecked(true);
            break;
      default:
            ui->rb_half_duplex->setChecked(true);
           break;
      }
    ui->chb_freq_list_use->setChecked(m_enable_param->use_freq_list);
    slotUseFreqList(m_enable_param->use_freq_list);
    ui->sb_freq_offset->setValue(m_enable_param->fr_offset);
    ui->sb_freq_active->setValue(m_enable_param->fr_active);
    ui->sb_freq_all->setValue(m_enable_param->fr_all);
    ui->sb_pspKey->setValue(m_enable_param->pcp_key);
    ui->sb_initTime_psp->setValue(m_enable_param->initPcpTime);
    ui->sb_mode_fr_psp->setValue(m_enable_param->modeFreqPcp);

    if(m_enable_param->fr_adapt == OFF)
       ui->rb_freq_no->setChecked(true);
     else ui->rb_freq_yes->setChecked(true);

    if(m_enable_param->cons_adapt == OFF)
        ui->rb_sign_no->setChecked(true);
    else
        ui->rb_sign_yes->setChecked(true);

    ui->tw_freq_list->setRowCount(0);
    for (int i = 0; i < m_enable_param->freq_list.count(); ++i){
        ui->tw_freq_list->slotAddFreq();
        QTableWidgetItem *twi = new QTableWidgetItem();
        twi->setText(QString::number(m_enable_param->freq_list.at(i)));
        twi->setSelected(true);
        ui->tw_freq_list->setItem(i,0,twi);
    }

    ui->tw_freq_list_prim->setRowCount(0);
    for (int i = 0; i < m_enable_param->freq_list_prim.count(); ++i){
        ui->tw_freq_list_prim->slotAddFreq();
        QTableWidgetItem *twi = new QTableWidgetItem();
        twi->setText(QString::number(m_enable_param->freq_list_prim.at(i)));
        twi->setSelected(true);
        ui->tw_freq_list_prim->setItem(i,0,twi);
    }

    ui->tw_freq_list_add->setRowCount(0);
    for (int i = 0; i < m_enable_param->freq_list_add.count(); ++i){
        ui->tw_freq_list_add->slotAddFreq();
        QTableWidgetItem *twi = new QTableWidgetItem();
        twi->setText(QString::number(m_enable_param->freq_list_add.at(i)));
        twi->setSelected(true);
        ui->tw_freq_list_add->setItem(i,0,twi);
    }

   ui->sb_ppf_number->setValue(m_enable_param->ppf_number);
   ui->cb_ppf_switch->setCurrentText(m_enable_param->ppf_switch);
   ui->cb_in_prot->setCurrentText(m_enable_param->input_prot);
   ui->cb_out_prot->setCurrentText(m_enable_param->output_prot);
   ui->cb_rcvr_prot->setCurrentText(m_enable_param->rcvr_prot);
   ui->sb_out_port->setValue(m_enable_param->output_port);
   ui->le_out_adr->setText(m_enable_param->output_adr);
   ui->le_rcvr_adr->setText(m_enable_param->rcvr_adr);

   QStringList radioDirList = m_enable_param->radio_dir_name.split(':');
   ui->sb_radio_dir_name_1->setValue(radioDirList.at(0).toInt());
   ui->sb_radio_dir_name_3->setValue(radioDirList.at(2).toInt());
   ui->sb_radio_dir_name_4->setValue(radioDirList.at(3).toInt());

   if (radioDirList.at(1)=="TX"){
       ui->rb_radio_dir_tx->setChecked(true);

   }
   else if (radioDirList.at(1)=="RX"){
       ui->rb_radio_dir_rx->setChecked(true);       
   }

   ui->tw_input_port->setRowCount(0);
   for (int i = 0; i < m_enable_param->input_port_list.count(); ++i){
       ui->tw_input_port->slotAddFreq(); /////!!!!!!!
       QTableWidgetItem *twi = new QTableWidgetItem();
       twi->setText(QString::number(m_enable_param->input_port_list.at(i)));
       twi->setSelected(true);
       ui->tw_input_port->setItem(i,0,twi);
   }

   ui->tw_ppf_ind->setRowCount(0);
   for (int i = 0; i < m_enable_param->ppf_ind_list.count(); ++i){
       ui->tw_ppf_ind->slotAddFreq(); /////!!!!!!!
       QTableWidgetItem *twi = new QTableWidgetItem();
       twi->setText(QString::number(m_enable_param->ppf_ind_list.at(i)));
       twi->setSelected(true);
       ui->tw_ppf_ind->setItem(i,0,twi);
   }

    m_vec_widg_rc.clear();
    ui->tab_rc->clear();
    for (int i = 0; i < m_enable_param->radio_param.count(); ++i){
        QString r_name;
        slotAddRC();
        RadioParams rp = m_enable_param->radio_param.at(i);
        r_name = rp.name;
        if(r_name.contains("RX",Qt::CaseInsensitive)){
            QString name_part = rp.name.section(':',0,0);
           m_vec_widg_rc.at(i)->m_le_name->setText(name_part);
           m_vec_widg_rc.at(i)->m_rb_rx->setChecked(true);
        }
        else if(r_name.contains("TX",Qt::CaseInsensitive)){
            QString name_part = rp.name.section(':',0,0);
           m_vec_widg_rc.at(i)->m_le_name->setText(name_part);
           m_vec_widg_rc.at(i)->m_rb_tx->setChecked(true);
        }
        m_vec_widg_rc.at(i)->m_le_ip->setText(rp.ipv4);
        m_vec_widg_rc.at(i)->m_sp_tcp->setValue(rp.tcp_port);
        m_vec_widg_rc.at(i)->m_sp_udp_1->setValue(rp.udp_port1);
        m_vec_widg_rc.at(i)->m_sp_udp_2->setValue(rp.udp_port2);
    }

    if(ui->chb_freq_list_use->isChecked())
       ui->sb_freq_all->setValue(ui->tw_freq_list->getFreqCount());

}

//------------------------------------------------------------------------------------------------------
// считывание данных из файла conf. на форму
void EnableDCE_dial::readSettFromIni()
{
    QString path_ini = settFileExist();
    if(path_ini!=""){
        QSettings *sett = new QSettings(path_ini, QSettings::IniFormat); // создаем объект настроек(либо считываем данные из файла либо заготавливаем по умолчанию)

        if(checkDataBeforRead(sett, SECTION_NAME, CRC_KEY_NAME)){                // проверка целостности ини файла пройдена

            if(sett->status()==QSettings::NoError){  //  в формате объекта настроек ошибок не обнаружено
               settKeyToVal(sett);                  //  данные из объекта настоек помещаем в поле класса
               valToWgt();                          //  данные из поля класса помещаем на форму
            }
            else
               QMessageBox::critical(this, "Сообщение", "Ошибка в формате файла настроек","");
        }

       else {    // проверка целостности ини файла не пройдена (или файл отсутствует)
        // !   QMessageBox::critical(this, "Сообщение", "Файл настроек поврежден.\n Будут загружены данные по умолчанию.","");  // предупредим
           defaultSettingsToVal();     // перезапишем данными по умолчанию
           valToWgt();                 // в виджет
           writeDataToIni();           // и сохраним в файл
       }
    }
    else{
     // !   QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует.\n Будут загружены данные по умолчанию.","");  // предупредим
        defaultSettingsToVal();
        valToWgt();                 // в виджет
        writeDataToIni();           // и сохраним в файл
    }

    if(ui->rb_radio_dir_rx->isChecked()){
      ui->lb_radio_role->setText("(ПРМ)");
      ui->lb_input->setText("порт РС");
      ui->lb_output->setText("порт ПАК МАС");
      ui->sb_out_port->setValue(m_mas_port);
    }
    else{
      ui->lb_radio_role->setText("(ПРД)");
      ui->lb_input->setText("порт ПАК МАС");
      ui->tw_input_port->item(0,0)->setText(QString::number(m_mas_port));
      ui->lb_output->setText("порт РС");
    }

}
//---------------------------------------------------------------------------------
// запись данных с формы в файл настроек
void EnableDCE_dial::writeDataToIni()
{
       // создадим объект настроек заново и считаем в него данные из виджетов формы
       QSettings  *sett = new QSettings(getPathToIniFile(), QSettings::IniFormat/*NativeFormat*/);

       sett->remove("Enable-param");
       sett->setValue("Enable-param/DCE_ID", ui->sb_DCE_ID->value());
       sett->setValue("Enable-param/bandWidth", ui->cb_regim_bw->currentText());
       sett->setValue("Enable-param/mode", ui->cb_reg->currentIndex());
       sett->setValue("Enable-param/mode_var", ui->cb_reg_var->currentText());
       sett->setValue("Enable-param/freq_discr", ui->cb_freq_discr->currentText());
       sett->setValue("Enable-param/change_time", (ui->cb_chng->currentText()));
       sett->setValue("Enable-param/mode2", (ui->rb_simplex->isChecked()*2 +
                                             ui->rb_half_duplex->isChecked()*0 +
                                             ui->rb_duplex->isChecked()*1));
       sett->setValue("Enable-param/freq_offset", ui->sb_freq_offset->value());
       sett->setValue("Enable-param/freq_all", ui->sb_freq_all->value());
       sett->setValue("Enable-param/freq_active", ui->sb_freq_active->value());
       sett->setValue("Enable-param/pcp_key", ui->sb_pspKey->value());
       sett->setValue("Enable-param/init_time_pcp", ui->sb_initTime_psp->value());
       sett->setValue("Enable-param/mode_fr_pcp", ui->sb_mode_fr_psp->value());
       sett->setValue("Enable-param/use_freq_list", (ui->chb_freq_list_use->isChecked()));
       sett->setValue("Enable-param/freq_adaptation", (ui->rb_freq_no->isChecked()*0 +
                                                       ui->rb_freq_yes->isChecked()*1));
       sett->setValue("Enable-param/cons_adaptation", (ui->rb_sign_no->isChecked()*0 +
                                                       ui->rb_sign_yes->isChecked()*1));

       sett->setValue("Enable-param/ppf_number",ui->sb_ppf_number->value());
       sett->setValue("Enable-param/ppf_switch",ui->cb_ppf_switch->currentText());
       sett->setValue("Enable-param/input_prot",ui->cb_in_prot->currentText());
       sett->setValue("Enable-param/output_prot",ui->cb_out_prot->currentText());
       sett->setValue("Enable-param/rcvr_prot",ui->cb_rcvr_prot->currentText());
       sett->setValue("Enable-param/output_port",ui->sb_out_port->value());
       sett->setValue("Enable-param/rcvr_port",ui->sb_rcvr_port->value());
       sett->setValue("Enable-param/output_adr",ui->le_out_adr->text());
       sett->setValue("Enable-param/rcvr_adr",ui->le_rcvr_adr->text());
       sett->setValue("Enable-param/freq_discr",ui->cb_freq_discr->currentText());

       QString radio_role{"TX"};
       if(ui->rb_radio_dir_rx->isChecked())
           radio_role ="RX";

       QString radioDir = QString::number(ui->sb_radio_dir_name_1->value())+":" + radio_role +":"+
               QString::number(ui->sb_radio_dir_name_3->value())+":" +
               QString::number(ui->sb_radio_dir_name_4->value());

       sett->setValue("Enable-param/radio_dir_name", radioDir);

       sett->beginWriteArray("Enable-param/input_port_list");
       int size_port = ui->tw_input_port->rowCount();
       for(int i = 0; i < size_port; ++i){
          sett->setArrayIndex(i);
          QString val = ui->tw_input_port->item(i,0)->text();
          sett->setValue("num", val);
       }
       sett->endArray();

       sett->beginWriteArray("Enable-param/ppf_ind_list");
       int size_ppf = ui->tw_ppf_ind->rowCount();
       for(int i = 0; i < size_ppf; ++i){
          sett->setArrayIndex(i);
          QString val = ui->tw_ppf_ind->item(i,0)->text();
          sett->setValue("num", val);
       }
       sett->endArray();

       sett->beginWriteArray("Enable-param/freq-list");
       int size = ui->tw_freq_list->rowCount();
       for(int i = 0; i < size; ++i){
          sett->setArrayIndex(i);
          QString val = ui->tw_freq_list->item(i,0)->text();
          sett->setValue("num", val);
       }
       sett->endArray();

       sett->beginWriteArray("Enable-param/freq-list_prim");
       int size1 = ui->tw_freq_list_prim->rowCount();
       for(int i = 0; i < size1; ++i){
          sett->setArrayIndex(i);
          QString val = ui->tw_freq_list_prim->item(i,0)->text();
          sett->setValue("num", val);
       }
       sett->endArray();

       sett->beginWriteArray("Enable-param/freq-list_add");
       int size2 = ui->tw_freq_list_add->rowCount();
       for(int i = 0; i < size2; ++i){
          sett->setArrayIndex(i);
          QString val = ui->tw_freq_list_add->item(i,0)->text();
          sett->setValue("num", val);
       }
       sett->endArray();

       sett->beginWriteArray("Enable-param/radio-params");
       for(int i = 0; i < m_vec_widg_rc.count(); ++i){
          sett->setArrayIndex(i);
          if(m_vec_widg_rc.at(i)->m_rb_tx->isChecked())
             { sett->setValue("radio_name", m_vec_widg_rc.at(i)->m_le_name->text() + ":TX"); }
          else if(m_vec_widg_rc.at(i)->m_rb_rx->isChecked())
             { sett->setValue("radio_name",m_vec_widg_rc.at(i)->m_le_name->text() + ":RX");}
           sett->setValue("radio_ipv4", m_vec_widg_rc.at(i)->m_le_ip->text());
           sett->setValue("radio_tcp_port", m_vec_widg_rc.at(i)->m_sp_tcp->value());
           sett->setValue("radio_udp_port1", m_vec_widg_rc.at(i)->m_sp_udp_1->value());
           sett->setValue("radio_udp_port2", m_vec_widg_rc.at(i)->m_sp_udp_2->value());
       }
       sett->endArray();

       unsigned short crc = calcCRCForSettings(sett, SECTION_NAME, CRC_KEY_NAME);
       sett->setValue("Enable-param/crc_enableDCE", crc);

}
//---------------------------------------------------------------------------------
void EnableDCE_dial::initForm()
{

    // настройка ComboBox_regim в зависимости от выбранного этапа
    //modelRegimSett();

    // настройка ComboBox_regimBW , cb_reg
    QStringList bandW;
    bandW <<"3100" << "4500" << "9000" << "40000";

    QStringList change_r;
    change_r <<"50" << "200";

    QStringList freq_d;
    freq_d <<"8000" << "9600" << "12800" <<"19200";

    QStringList mode_v;
   /* { 1,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_15,sub_29,sbits,ibits_240,54,32,duplex_and_half,3,8,1},
    { 2,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),2,packs_15,sub_29,sbits,ibits_480,108,62,duplex_and_half,3,8,1},
    { 3,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_15,sub_29,sbits,ibits_720,162,92,duplex_and_half,3,8,1},
    { 4,tr("ФРЧ"), 3,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_15,sub_29,sbits,ibits_960,217,123,duplex_and_half,3,8,1},
    { 5,tr("НРЧ"), 3,slot200,fBand_3100,fd_80,tr("ОФМ"),1,packs_15,sub_29,sbits,ibits_240,54,32,duplex_and_half,3,8,2},
    { 6,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),2,packs_15,sub_29,sbits,ibits_480,108,62,duplex_and_half,3,8,2},
    { 7,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_15,sub_29,sbits,ibits_720,162,92,duplex_and_half,3,8,2},
    { 8,tr("НРЧ"), 3,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_15,sub_29,sbits,ibits_960,217,123,duplex_and_half,3,8,2},
    { 9,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_9,sub_29,sbits,ibits_120,32,18,duplex_and_half,1,8,3},
    {10,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),2,packs_9,sub_29,sbits,ibits_240,65,33,duplex_and_half,1,8,3},
    {11,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_9,sub_29,sbits,ibits_360,97,47,duplex_and_half,1,8,3},
    {12,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),3,packs_9,sub_29,sbits,ibits_480,97,63,duplex_and_half,1,8,3},
    {13,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_9,sub_29,sbits,ibits_480,130,62,duplex_and_half,2,8,3},
    {14,tr("НРЧ"),75,slot200,fBand_3100,fd_96,tr("АФМ"),4,packs_9,sub_29,sbits,ibits_720,130,92,duplex_and_half,2,8,3},
    {15,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("ОФМ"),1,packs_9,sub_44,sbits,ibits_240,49,33,duplex_and_half,1,8,4},
    {16,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("ОФМ"),2,packs_9,sub_44,sbits,ibits_480,99,63,duplex_and_half,1,8,4},
    {17,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("ОФМ"),3,packs_9,sub_44,sbits,ibits_720,148,92,duplex_and_half,1,8,4},
    {18,tr("НРЧ"),75,slot200,fBand_4500,fd_96,tr("АФМ"),4,packs_9,sub_44,sbits,ibits_960,198,122,duplex_and_half,2,8,4},
    {19,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("ОФМ"),1,packs_3,sub_44,sbits,ibits_060,16,10,duplex_and_half,2,8,5},
    {20,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("ОФМ"),2,packs_3,sub_44,sbits,ibits_120,33,17,duplex_and_half,2,8,5},
    {21,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("ОФМ"),3,packs_3,sub_44,sbits,ibits_240,49,33,duplex_and_half,2,8,5},
    {22,tr("ППРЧ"),3,slot50,fBand_4500,fd_96,tr("АФМ"),4,packs_3,sub_44,sbits,ibits_480,66,62,duplex_and_half,2,8,5},
    {23,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),1,packs_3,sub_89,sbits,ibits_060,33,11,duplex_and_half,2,8,6},
    {24,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),1,packs_3,sub_89,sbits,ibits_120,33,17,duplex_and_half,2,8,6},
    {25,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),2,packs_3,sub_89,sbits,ibits_240,66,32,duplex_and_half,2,8,6},
    {26,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("ОФМ"),3,packs_3,sub_89,sbits,ibits_480,100,62,duplex_and_half,2,8,6},
    {27,tr("ППРЧ"),3,slot50,fBand_9000,fd_192,tr("АФМ"),4,packs_3,sub_89,sbits,ibits_960,133,123,duplex_and_half,2,8,6},
    {28,tr("ППРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_9,sub_29,sbits28,ibits_0,32,22,serv_package,1,8,7},
    {29,tr("ППРЧ"),75,slot200,fBand_3100,fd_96,tr("ОФМ"),1,packs_9,sub_29,sbits29,ibits_120,32,22,simplex,1,8,7}
   */
    for (int i =0; i < 29; ++i){
         mode_v <<QString::number(i+1);
    }
   /* mode_v <<" 1, ФРЧ, 3, 200, 3100, ОФМ " << " 2, ФРЧ, 3, 200, 3100, ОФМ " << " 3, ФРЧ, 3, 200, 3100, ОФМ "
           <<" 4, ФРЧ, 3, 200, 3100, АФМ "<< " 5, НРЧ, 3, 200, 3100, ОФМ " << " 6, НРЧ, 3, 200, 3100, ОФМ "
           <<" 7, НРЧ, 3, 200, 3100, ОФМ "<< " 8, НРЧ, 3, 200, 3100, AФМ " << " 9, НРЧ, 75, 200, 3100, ОФМ "
           <<" 10, НРЧ, 75, 200, 3100, ОФМ "<< " 11, НРЧ, 75, 200, 3100, ОФМ " << " 12, НРЧ, 75, 200, 3100, ОФМ "
           <<" 13, НРЧ, 75, 200, 3100, АФМ "<< " 14, НРЧ, 75, 200, 3100, AФМ " << " 15, НРЧ, 75, 200, 4500, ОФМ "
           <<" 16, НРЧ, 75, 200, 4500, ОФМ "<< " 17, НРЧ, 75, 200, 4500, ОФМ " << " 18, НРЧ, 75, 200, 4500, AФМ "
           <<" 19, ППРЧ, 3, 50, 4500, ОФМ "<< " 20, ППРЧ, 3, 50, 4500, ОФМ " << " 21, ППРЧ, 3, 50, 4500, ОФМ "
           <<" 22, ППРЧ, 3, 50, 4500, АФМ "<< " 23, ППРЧ, 3, 50, 9000, ОФМ " << " 24, ППРЧ, 3, 50, 9000, ОФМ "
           <<" 25, ППРЧ, 3, 50, 9000, ОФМ "<< " 26, ППРЧ, 3, 50, 9000, ОФМ " << " 27, ППРЧ, 3, 50, 9000, АФМ "
           <<" 28, ППРЧ, 75, 200, 3100, ОФМ "<< " 29, ППРЧ, 75, 200, 3100, ОФМ " ;
     */
    QStringList protName;   // наименование протоколов взаимодействия
    protName<<"RTP" << "JSON" << "HTTP";

    QStringList ppf_sw;
    ppf_sw<<"manual"<<"auto";

    ui->cb_ppf_switch->addItems(ppf_sw);
    ui->cb_rcvr_prot->addItems(protName);
    ui->cb_out_prot->addItems(protName);
    ui->cb_in_prot->addItems(protName);
    ui->cb_reg->addItems(REGIM);
    ui->cb_regim_bw->addItems(bandW);
    ui->cb_chng->addItems(change_r); 
    ui->cb_reg_var->addItems(mode_v);
    ui->cb_freq_discr->addItems(freq_d);

    connect(ui->tab_rc, SIGNAL(tabBarClicked(int)), this, SLOT(slotRCSett()));
    connect(ui->tab_rc, SIGNAL(currentChanged(int)), this, SLOT(slotRCSett()));

        // настройка группы "Режим","Уточнение режима"
  //  connect(ui->cb_regim, SIGNAL(activated(int)), this, SLOT(slotDHS(int)));

     // настройка TableWidget_freqList, SpinBox_freq_all, SpinBox_freq_active, SpinBox_freq_offset,
   // useFreqList(ui->chb_freq_list_use->isChecked());



    connect(ui->chb_freq_list_use, SIGNAL(stateChanged(int)), this, SLOT(slotUseFreqList()));
    connect(ui->chb_freq_list_use, SIGNAL(clicked(bool)), this, SLOT(slotUseFreqList(bool)));
    connect(ui->sb_freq_active, SIGNAL(valueChanged(int)), this, SLOT(slotCheckFreqsCount(int)));
    connect(ui->sb_freq_offset, SIGNAL(valueChanged(int)), this, SLOT(slotCheckFreqsCount(int)));

    connect (ui->tb_add_rc, SIGNAL(clicked()), this, SLOT (slotAddRC()));    // добавл РС
    connect (ui->tb_del_rc, SIGNAL(clicked()), this, SLOT (slotDelRC()));    // удалить РС

    connect (ui->pbApplay, SIGNAL(clicked()), this, SLOT(slotApplay()));     // кнопка Применить
    connect (ui->pbCancel, SIGNAL(clicked()), this, SLOT(slotCancel()));     // кнопка ОТМЕНИТЬ

    connect (ui->tb_add_port, SIGNAL(clicked()), ui->tw_input_port, SLOT(slotAddFreq()));  // доб. в список порт
    connect (ui->tb_del_port, SIGNAL(clicked()), ui->tw_input_port, SLOT(slotDelFreq()));  // удалить из списка порт

    connect (ui->tb_add_ppf_ind, SIGNAL(clicked()), ui->tw_ppf_ind, SLOT(slotAddFreq()));  // доб. в список индекса частот
    connect (ui->tb_del_ppf_ind, SIGNAL(clicked()), ui->tw_ppf_ind, SLOT(slotDelFreq()));  // удалить из списка индекс частоты

    connect (ui->tb_add_freq, SIGNAL(clicked()), ui->tw_freq_list, SLOT(slotAddFreq()));  // доб. в список частот
    connect (ui->tb_del_freq, SIGNAL(clicked()), ui->tw_freq_list, SLOT(slotDelFreq()));  // удалить из списка частот

    connect (ui->tb_add_freq_prim, SIGNAL(clicked()), ui->tw_freq_list_prim, SLOT(slotAddFreq()));  // доб. в список частот НПП
    connect (ui->tb_del_freq_prim, SIGNAL(clicked()), ui->tw_freq_list_prim, SLOT(slotDelFreq()));  // удалить из списка частот НПП

    connect (ui->tb_add_freq_add, SIGNAL(clicked()), ui->tw_freq_list_add, SLOT(slotAddFreq()));  // доб. в список частот ЗЧ
    connect (ui->tb_del_freq_add, SIGNAL(clicked()), ui->tw_freq_list_add, SLOT(slotDelFreq()));  // удалить из списка частот ЗЧ


    delete ui->tab_rc->currentWidget();

    ui->tw_freq_list->setTabKeyNavigation(false);
    ui->tw_freq_list_prim->setTabKeyNavigation(false);
    ui->tw_freq_list_add->setTabKeyNavigation(false);

    connect(ui->tw_freq_list, SIGNAL(freqsChange()), this , SLOT(slotChangeFreqAll()));
    connect(ui->sb_freq_all, SIGNAL(valueChanged(int)), this, SLOT(slotNumAllChange(int)));
    connect(ui->sb_freq_active, SIGNAL(valueChanged(int)), this, SLOT(slotNumActiveChange(int)));
    connect(ui->sb_freq_offset, SIGNAL(valueChanged(int)), this, SLOT(slotNumOffSetChange(int)));

      // начальные граничные значениея группы "кол-ва частот"
    ui->sb_freq_all->setMinimum(1);
    ui->sb_freq_all->setMaximum(128);

    ui->sb_freq_active->setMinimum(1);
    ui->sb_freq_active->setMaximum(128);

    ui->sb_freq_offset->setMinimum(0);
    ui->sb_freq_offset->setMaximum(128);

    connect(ui->cb_reg, SIGNAL(activated(int)), this, SLOT(slotDHS_bw()));
    connect(ui->cb_regim_bw, SIGNAL(activated(int)), this, SLOT(slotDHS_bw()));
    connect(ui->cb_chng, SIGNAL(activated(int)), this, SLOT(slotDHS_bw()));
  //  connect(ui->rb_RX, SIGNAL(clicked()), this, SLOT(slotRX_check()));//>cb_chng, SIGNAL(activated(int)), this, SLOT(slotDHS_bw()));
  //  connect(ui->rb_TX, SIGNAL(clicked()), this, SLOT(slotTX_check()));

    QCommonStyle style;
    ui->pbDefault->setIcon(style.standardIcon(QStyle::SP_ArrowBack));
    ui->pbDefault->setToolTip("Вернуться к настройкам \n по умолчанию.");
    connect (ui->pbDefault, SIGNAL(clicked()), this, SLOT(slotDefaultSettings()));  // кнопка "Значения по умолчанию"

    ui->cb_regim_bw->setEnabled(false);
    ui->cb_reg->setEnabled(false);
    ui->cb_chng->setEnabled(false);

}
//-----------------------------------------------------------------------------------
void EnableDCE_dial::slotRX_check()
{
    if (ui->rb_RX->isChecked()){
        ui->lb_radio_role->setText("ПРМ");
    }
}
//-----------------------------------------------------------------------------------
void EnableDCE_dial::slotTX_check()
{
    if (ui->rb_TX->isChecked()){
        ui->lb_radio_role->setText("ПРД");
    }
  }
//-----------------------------------------------------------------------------------
// Уточнение режима для Режима _ mode1(один из 29 режимов) (технологический)
void EnableDCE_dial::slotDHS(int index)
{
  int mode1 = index + 1;

    if(mode1 < 28 && mode1 > 0){
        ui->rb_duplex->setEnabled(true);
        ui->rb_half_duplex->setEnabled(true);
        ui->rb_simplex->setEnabled(false);
        ui->rb_simplex->setText("симплекс");
        ui->rb_duplex->setText("дуплекс");
        ui->rb_half_duplex->setText("полудуплекс");
     }
    else if (mode1 == 28){
        ui->rb_duplex->setEnabled(false);
        ui->rb_simplex->setEnabled(false);
        ui->rb_half_duplex->setEnabled(false);
        ui->rb_simplex->setText("служебные кадры");
        ui->rb_simplex->setChecked(true);
    }
    else if (mode1 == 29){
        ui->rb_duplex->setEnabled(false);
        ui->rb_half_duplex->setEnabled(false);
        ui->rb_simplex->setText("симплекс");
        ui->rb_simplex->setChecked(true);
    }  
}
//-----------------------------------------------------------------------------------
// изменения вида панели "Уточнение режима" для варианта "Полоса пропускания - Режим - Время смены радиочастот"
void EnableDCE_dial::slotDHS_bw()
{  
    int reg = ui->cb_reg->currentIndex();       // режим
    int bandW = ui->cb_regim_bw->currentIndex(); // полоса пропускания
    int chng_tm = ui->cb_chng->currentIndex(); // время смены радиочастот

    if(reg == 1 && bandW == 0 && chng_tm == 1 ){  // НРЧ, 3100 Гц, 75 мс
        ui->rb_duplex->setEnabled(true);
        ui->rb_half_duplex->setEnabled(true);
        ui->rb_simplex->setEnabled(true);
        ui->rb_duplex->setText("дуплекс");
        ui->rb_half_duplex->setText("полудуплекс");
        ui->rb_simplex->setText("симплекс");
     }
    else {                                         // все остальные комбинации параметров
        ui->rb_duplex->setEnabled(true);
        ui->rb_half_duplex->setEnabled(true);
        ui->rb_simplex->setEnabled(false);
        if(ui->rb_simplex->isChecked())
           ui->rb_duplex->setChecked(true);
        ui->rb_duplex->setText("дуплекс");
        ui->rb_half_duplex->setText("полудуплекс");
        ui->rb_simplex->setText("симплекс");
    }   
}
//-----------------------------------------------------------------------------------
//     Работа со списком частот и количественными характеристиками частотного ряда
//-----------------------------------------------------------------------------------
// использование или нет списка частот (по умолчанию - используем)
void EnableDCE_dial::slotUseFreqList(bool state)
{
    useFreqList(state);
}
//----------------------------------------------------------------------------------
// изменение количества частот в таблице список частот(добавили или удалили)
void EnableDCE_dial::slotChangeFreqAll()
{
    if(ui->chb_freq_list_use->isChecked()) // параметр зависм от кол-ва частот в табл. если выбран чек бокс - Использовать список частот
      ui->sb_freq_all->setValue(ui->tw_freq_list->getFreqCount());
}
//-----------------------------------------------------------------------------------
// изменения в виджете "Всего частот". Этот виджет доступен только для чтения, отробажает кол-во частот в таблице частот
void EnableDCE_dial::slotNumAllChange(int val) // val - значение "кол-во частот всего"
{   
    if(m_stage == WORK){   // переменная определяет 2 состояния (инициализация формы= INIT и работа формы = WORK)
       int active = ui->sb_freq_active->value();   // кол-во активных частот
       int offset = ui->sb_freq_offset->value();   // частотный сдиг

       if((active + offset) > val){ // если границы допустимого перешли
                                    // необходимо корректировать значения (одно или оба),
                                   //а так же граничные значения обоих величин
           if(active >= val){      // кол-во активных частот стало больше общего кол-ва чатот (или равно)
               ui->sb_freq_active->setValue(val);
               ui->sb_freq_active->setMinimum(1);

               ui->sb_freq_offset->setValue(0);
               ui->sb_freq_offset->setMinimum(0);
            }
            else if (active < val){       // значение которое не выходит за границы менять не будем
               ui->sb_freq_active->setMinimum(1);
               ui->sb_freq_offset->setValue(val - active);
               ui->sb_freq_offset->setMinimum(0);
            }
        }
    }
}
//-----------------------------------------------------------------------------------
// изменения в виджете "Активные частоты"
void EnableDCE_dial::slotNumActiveChange(int val)  // val - число активных частот
{    
    if(m_stage == WORK){ // форма находится  в режиме работы, (после режима загрузки данных)
      int max = ui->sb_freq_all->value(); // общее кол-во частот
      int offset =  ui->sb_freq_offset->value();  // частотный сдвиг

      if((val + offset) > max){
          if(val <=max)
             ui->sb_freq_offset->setValue(max-val);
          else{
             ui->sb_freq_offset->setValue(0);
             ui->sb_freq_active->setValue(max);
          }
       }
    }
}
//-----------------------------------------------------------------------------------
// изменения в виджете "Частотный сдвиг"
void EnableDCE_dial::slotNumOffSetChange(int val) // val - частотный сдвиг
{   
    if(m_stage == WORK){ // форма находится  в режиме работы, (после режима загрузки данных)
       int max = ui->sb_freq_all->value();       // общее число частот
       int active = ui->sb_freq_active->value(); // число активных частот

       if((val + active)> max){
           if(val < max)
              ui->sb_freq_active->setValue(max-val);
           else{
              ui->sb_freq_active->setValue(1);  // кол-во активных частот не может быть менее 1
              ui->sb_freq_offset->setValue(max-1);
           }
        }
    }
}
//-----------------------------------------------------------------------------------
// используется или нет список частот, определяется поведение виджетов связанных с частотными характеристиками
void EnableDCE_dial::useFreqList(bool state)
{
    if(state){  // если выбрано "Использовать список частот"
       ui->tw_freq_list->setEnabled(true);      // имеем доступ к редактированию Списка частот
       ui->sb_freq_all->setReadOnly(true);      // при этом не можем изменять значение параметра "Кол-во частот"
       ui->tb_add_freq->setEnabled(true);
       ui->tb_del_freq->setEnabled(true);
       ui->sb_freq_all->setValue(ui->tw_freq_list->rowCount());
    }
   else{       // список частот не используется
        ui->tw_freq_list->setEnabled(false);  // не имеем доступа к редактированию списка частот
        ui->sb_freq_all->setReadOnly(false);  // можем изменять значение данного параметра
        ui->tb_add_freq->setEnabled(false);
        ui->tb_del_freq->setEnabled(false);
    }   
}
//--------------------------------------------------------------------------------------------
//           Работа с радиосредствами на форме
//--------------------------------------------------------------------------------------------
// добавление панели радиосредство на форме
void EnableDCE_dial::slotAddRC()
{   
   ++m_rc_count;     // увеличиваем счетчик подсчета добавленных радиосредсв( для заголовка надписи "РС1")
    widgetRC *wg = new widgetRC();    // создаем виджет "радиосредство"
    ui->tab_rc->addTab(wg,"РС"+ QString::number(m_rc_count)); // создаем новую вкладку

    m_vec_widg_rc.append(wg);         // добавляем виджет в вектор
    ui->tab_rc->setCurrentWidget(m_vec_widg_rc.last()); // добавляем виджет на форму

    slotRCSett();                    // помещаем данные радиосредства по умолчанию в новый виджет
    int n = ui->tab_rc->currentIndex();  // определяем текущую вкладку , вновь созданного радиосредства
    // привязываем  ряд слотов к новому виджету
    connect(m_vec_widg_rc.at(n), SIGNAL(changeRadioName()), this, SLOT(slotRCSett()));
    //connect(m_vec_widg_rc.at(n), SIGNAL(changeRadioName()), this, SLOT(slotDHS_bw()));

    tabOrderSett();            // переопределяем порядок обхода элементов на форме, т.к. появились новые элементы
}
//-------------------------------------------------------------------------------------------
// слот настройка радиосредств (внешний вид и доступность вкладки радио средства)
void EnableDCE_dial::slotRCSett()
{
    int index = ui->tab_rc->currentIndex();
    if(m_vec_widg_rc.count() > index && index >-1){ // если открыта валидная вкладка радиосредства

       if(m_vec_widg_rc.at(index)->m_rb_rx->isChecked()){        // если на текущей панеле радиосредства rx -выбран
          m_vec_widg_rc.at(index)->m_sp_tcp->setEnabled(true);
          m_vec_widg_rc.at(index)->m_sp_udp_1->setEnabled(true);
          m_vec_widg_rc.at(index)->m_sp_udp_2->setEnabled(true);

        }
        else if (m_vec_widg_rc.at(index)->m_rb_tx){   // выбран tx
           m_vec_widg_rc.at(index)->m_sp_tcp->setEnabled(false);
           m_vec_widg_rc.at(index)->m_sp_tcp->setValue(1);
           m_vec_widg_rc.at(index)->m_sp_udp_1->setEnabled(true);
           m_vec_widg_rc.at(index)->m_sp_udp_2->setEnabled(false);
           m_vec_widg_rc.at(index)->m_sp_udp_2->setValue(1);

        }
    }
}
//--------------------------------------------------------------------------------------------
//  удаление элемента радиосредство на форме
void EnableDCE_dial::slotDelRC()
{
    if(m_vec_widg_rc.count()>1){
       m_vec_widg_rc.remove(ui->tab_rc->currentIndex()); // удаляем данные виджета из вектора
       ui->tab_rc->removeTab(ui->tab_rc->currentIndex()); // удаляем виджет с формы
    }
    else
        QMessageBox::critical(this, "Сообщение", "Невозможно удалить последнее радиосредство из списка.\n" ,"");
}
//--------------------------------------------------------------------------------------------
//      Завершение работы с радиосредствами
//--------------------------------------------------------------------------------------------
// кнопка ПРИНЯТЬ
// здесь наш внешний объект должен быть заполнен целиком
void EnableDCE_dial::slotApplay()
{
    wgtToVal();

    writeDataToIni();               // кроме этого данные с формы должны быть сохранены в conf файле с настройками

    accept();                       // закрываем форму с флагом ПРИНЯТО
}
//--------------------------------------------------------------------------------------------
//  кнопка ОТМЕНИТЬ
void EnableDCE_dial::slotCancel()
{
    reject();                      // отменяем все наши действия на форме, выходим с флагом ОТМЕНЕНО
}
//--------------------------------------------------------------------------------------------
// Задание порядка обхода элементов формы
void EnableDCE_dial::tabOrderSett()
{
    ui->pbDefault->setFocus();
    setTabOrder(this, ui->pbDefault);
    setTabOrder(ui->pbDefault, ui->sb_DCE_ID);
    setTabOrder(ui->sb_DCE_ID, ui->chb_freq_list_use );

    setTabOrder(ui->chb_freq_list_use, ui->rb_duplex);
    setTabOrder(ui->rb_duplex, ui->tb_add_freq);

    setTabOrder(ui->tb_add_freq, ui->tw_freq_list);
    setTabOrder(ui->tw_freq_list, ui->tb_del_freq);
    setTabOrder(ui->tb_del_freq, ui->tb_add_freq_prim);

    setTabOrder(ui->tb_add_freq_prim, ui->tw_freq_list_prim);
    setTabOrder(ui->tw_freq_list_prim, ui->tb_del_freq_prim);
    setTabOrder(ui->tb_del_freq_prim, ui->tb_add_freq_add);

    setTabOrder(ui->tb_add_freq_add, ui->tw_freq_list_add);
    setTabOrder(ui->tw_freq_list_add, ui->tb_del_freq_add);
    setTabOrder(ui->tb_del_freq_add, ui->sb_freq_all);

    setTabOrder(ui->sb_freq_all, ui->sb_freq_active);
    setTabOrder(ui->sb_freq_active, ui->sb_freq_offset);
    setTabOrder(ui->sb_freq_offset, ui->tb_add_rc);
    setTabOrder(ui->tb_add_rc, ui->tb_del_rc);
    setTabOrder(ui->tb_del_rc, ui->tab_rc);

    for(int i = 0; i < m_vec_widg_rc.count(); ++i){ // вектор вкладки радио средств
       setTabOrder(ui->tab_rc, m_vec_widg_rc.at(i)->m_rb_rx);
       setTabOrder(m_vec_widg_rc.at(i)->m_rb_rx, m_vec_widg_rc.at(i)->m_le_ip);
       setTabOrder(m_vec_widg_rc.at(i)->m_le_ip, m_vec_widg_rc.at(i)->m_sp_tcp);
       setTabOrder(m_vec_widg_rc.at(i)->m_sp_tcp, m_vec_widg_rc.at(i)->m_sp_udp_1);
       setTabOrder(m_vec_widg_rc.at(i)->m_sp_udp_1, m_vec_widg_rc.at(i)->m_sp_udp_2);
    }
}
//------------------------------------------------------------------------------------------------

void EnableDCE_dial::on_rb_radio_dir_tx_clicked(bool checked)
{
    if (checked){
        ui->lb_radio_role->setText("(ПРД)");
        ui->lb_input->setText("порт ПАК МАС");
        ui->lb_output->setText("порт РС");
        ui->tw_input_port->item(0,0)->setText(QString::number(m_mas_port));
    }
}
//------------------------------------------------------------------------------------------------

void EnableDCE_dial::on_rb_radio_dir_rx_clicked(bool checked)
{
    if (checked){
        ui->lb_radio_role->setText("(ПРМ)");
        ui->lb_input->setText("порт РС");
        ui->lb_output->setText("порт ПАК МАС");
        ui->sb_out_port->setValue(m_mas_port);
    }
}
//--------------------------------------------------------------------------------------------------
