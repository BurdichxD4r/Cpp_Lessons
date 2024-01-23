#include "netconnect_dial.h"
#include "ui_netconnect_dial.h"
#include <QCommonStyle>

#include <QMessageBox>
#include <QIODevice>
#include <QApplication>
#include <QAction>
#include <QSettings>
#include <QCommonStyle>
#include <QFile>
#include <QHeaderView>

int MAX_CHANNAL = 16;
int MAX_FACT_USER = 5;
int MAX_FACT_NET  = 5;

extern QStringList REGIM_EXCH;
extern QStringList ADAPTATION;
extern QStringList REGIM;
extern QStringList REGIM_C;
extern QStringList REGIM_S;
extern QStringList PRIORITY;

// необходимо соответствие с ModeSwitchEnum, ModeExchEnum, ModeOffOnEnum из enums_list.h (fhss-types компоненты)

 /*   QStringList REGIM_EXCH = {"HALF_DUPLEX", "DUPLEX", "SIMPLEX"};
    QStringList ADAPTATION = {"OFF", "ON"};
    QStringList REGIM      = {"ФРЧ", "НРЧ", "ППРЧ"};
    QStringList REGIM_C    = {"точка-точка", "точка-многоточка"};
    QStringList REGIM_S    = {"синхронный", "асинхронный"};
    QStringList PRIORITY   = {"M", "B", "P", "C", "O"};
    */

//------------------------------------------------------------------------
NetCONNECT_dial::NetCONNECT_dial(NetCONNECTParamStruct &data_struct, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::NetCONNECT_dial)
{
    ui->setupUi(this);
    m_netCONNECT_param = &data_struct;
    initForm();

    readSettFromIni();          // данные из файла ini считываются и заносятся на форму после прохождения проверки

}
//-------------------------------------------------------------------------
NetCONNECT_dial::~NetCONNECT_dial()
{
    delete ui;
}
//--------------------------------------------------------------------------------------
void NetCONNECT_dial::initForm()
{
    // настройка ComboBox_regimBW , cb_reg

    ui->cb_regim_comb_net->addItems(REGIM_C);//   regim <<"точка-точка" << "точка-многоточка" ;
    ui->cb_regim_switch->addItems(REGIM);//   change_r <<"ФРЧ" << "НРЧ" << "ППРЧ";
    ui->cb_regim_transit->addItems(REGIM_S);//   trans <<"синхронный" << "асинхронный" ;
    ui->cb_priority->addItems(PRIORITY); // priority <<"M" <<"B" << "P" <<"C" <<"O";

    QStringList rate;
    rate <<"auto" <<"1200" << "2400" <<"4800" <<"9600" << "19200" <<"38400" << "64000";
    ui->cb_rate->addItems(rate);

    QStringList bandW;
    bandW <<"3100" << "4500" << "9000" << "40000";
    ui->cb_regim_bw->addItems(bandW);

    QStringList change_rate;
    change_rate <<"50" << "200";
    ui->cb_rate_switch->addItems(change_rate);

    QStringList change_freq;
    change_freq <<"3" << "75";
    ui->cb_regim_change_freq->addItems(change_freq);

    QStringList stage;
    stage <<" 1 " << " 2 " << " 3 " << " 1, 2, 3 ";
    ui->cb_stage->addItems(stage);

    connect (ui->pb_applay, SIGNAL(clicked()), this, SLOT(slotApplay()));     // кнопка Применить
    connect (ui->pb_cancel, SIGNAL(clicked()), this, SLOT(slotCancel()));     // кнопка ОТМЕНИТЬ

    QCommonStyle style;
    ui->pb_default->setIcon(style.standardIcon(QStyle::SP_ArrowBack));
    ui->pb_default->setToolTip("Вернуться к настройкам \n по умолчанию.");
    connect (ui->pb_default, SIGNAL(clicked()), this, SLOT(slotDefaultSettings()));  // кнопка "Значения по умолчанию"

    connect (ui->cb_stage, SIGNAL(activated(int)), this, SLOT(modelRegimSett()));    // внешний вид модели в зависимости от выбранного этапа

    // настройка ComboBox_regim в зависимости от выбранного этапа
    modelRegimSett();

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
    ui->sb_imit->setDisplayIntegerBase(10);
    ui->sb_imit->setMinimum(0);

    ui->sb_DCE_ID->setMinimum(1);
    ui->sb_DCE_ID->setMaximum(429493729);

    ui->sb_DCE_ID_OPPN->setMinimum(1);
    ui->sb_DCE_ID_OPPN->setMaximum(429493729);

    ui->sb_freeParam->setMinimum(1);
    ui->sb_freeParam->setMaximum(MAX_CHANNAL);   // максим. кол-во каналов

    ui->sb_factor_user->setMinimum(1);
    ui->sb_factor_user->setMaximum(MAX_FACT_USER);   // максим. кол-во каналов

    ui->sb_factor_net->setMinimum(1);
    ui->sb_factor_net->setMaximum(MAX_FACT_NET);   // максим. кол-во каналов

    tabOrderSett();
}

//---------------------------------------------------------------------------------
void NetCONNECT_dial::modelRegimSett()
{
    Q_INIT_RESOURCE(mode);
    QString name{":/file_mode/inc/mode1.txt"};

    QString content = fileModelRead(name);
    if(content!=""){   //  если файл с данными для model прочитан и не пустой
       QStandardItemModel *model = new QStandardItemModel();
       createModel(content, *model);   // создание модели
       addModelToWidg(*model, ui->cb_regim);  // отображение модели в виджете
    }
}
//-----------------------------------------------------------------------------------
// добавление элемента () в модель
void NetCONNECT_dial::addColToRow(QString str,    // что добавляем
                                 QColor cl,      // какого цвета
                                 QList<QStandardItem *> &items) // куда добавляем
{
    Q_UNUSED(cl);
    QStandardItem *itm;
    itm = new QStandardItem("");   // первая колонка модели
    //itm->setData(cl, Qt::BackgroundColorRole);
    itm->setData(str, Qt::DisplayRole);
    items.append(itm);
}
//----------------------------------------------------------------------------------
// создание модели данных для mode1 и этапа (разные этапы отображаются разными цветами в моделе)
void  NetCONNECT_dial::createModel(QString content,            // содержание из файла "mode1.txt"
                                  QStandardItemModel & model) // output параметр - созданная модель
{
    QColor cl_stage[4]{"#eeeeee", "#dfdce4", "#ffffff","#d7d7d7"};

    QStringList lst_row = content.split("\n");   // список строк таблицы mode1
    QString str_bw{""};
    QString str_slot{""};
    int k = 0;
                // Устанавливаем заголовки колонок таблицы
    if(lst_row.count() > 0){
       QStringList lst_ = lst_row.at(0).split("\t");   //первую строку таблицы разбираем на названия колонок модели
       QStringList title;
       title << lst_.at(0) +",  "+ lst_.at(1) +",  "+ " F"+",  "+" Модуляция ";
       title << lst_.at(2) << lst_.at(3) << lst_.at(7);
       model.setHorizontalHeaderLabels(title);
     }

    if(lst_row.count() > 1){   // если в файле кроме заголовка содержалось хоть одна строка данных
           // Заполняем модель данными построчно
        for (int i = 1; i < lst_row.count(); ++i){  // начинаем со второй строки, 1-я заголовок
           if(lst_row.at(i)!=""){                   // если строка не пуста
              QStringList lst_col = lst_row.at(i).split("\t"); // разложим строку на колонки

              QColor cl;  // цвет для отображения группы в моделе
              if(str_bw == "" && str_slot== "")
                 cl = cl_stage[k%4];
              else
                 if(str_bw != lst_col.at(4) || str_slot != lst_col.at(7))
                    ++k;
              cl = cl_stage[k%4];
              str_bw = lst_col.at(4);
              str_slot = lst_col.at(7);

               QList<QStandardItem *> items;   // строка модели
                     // первая колонка модели
               QString str = QString("%1%2%3%4").arg(lst_col.at(0),-5,' ').arg(lst_col.at(1),-7, ' ').arg(lst_col.at(4)+" Гц",-9,' ').arg(lst_col.at(5)+" - "+lst_col.at(6), -5, ' ');

               addColToRow(str,cl,items);

               // вторая колонка модели
               if(lst_col.at(2)!= "")
                   addColToRow(lst_col.at(2)+"  мс    ", cl, items);
               else
                   addColToRow(lst_col.at(2), cl, items);
                   // третья колонка модели
               addColToRow(lst_col.at(3)+"  мс    ", cl, items);
                   // 4-я колонка модели
               addColToRow(lst_col.at(7)+"  мс    ", cl, items);

               // анализируем последнюю колонку(8) табл. - это колонка ЭТАП, определяем добавляем ли мы строку в модель ли нет
               if(ui->cb_stage->currentText() == " 1 " ){   // выбираем первый этап
                   if(lst_col.at(8)=="1")
                     model.appendRow(items);
               }
               else if(ui->cb_stage->currentText() ==" 2 " ){ // выбираем второй этап
                   if(lst_col.at(8)=="2")
                     model.appendRow(items);
               }
               else if(ui->cb_stage->currentText() == " 3 " ){// выбираем третий этап
                   if(lst_col.at(8) == "3")
                     model.appendRow(items);
               }
               else if(ui->cb_stage->currentText() ==" 1, 2, 3 " ){ // выбираем все этапы
                     model.appendRow(items);
               }
           }
        }
    }
}
//-----------------------------------------------------------------------------------
// подключение модели (mode1) к комбо боксу
void NetCONNECT_dial::addModelToWidg(QStandardItemModel & model,  // передаем модель
                                    QComboBox* combo_box)        // комбо куда надо загрузить модель
{
    QTableView  *tv = new QTableView();    // это будет наш выпадающий список
    QHeaderView *hh = new QHeaderView(Qt::Horizontal);
    QHeaderView *vh = new QHeaderView(Qt::Vertical);

    tv->setSizeAdjustPolicy(QAbstractItemView::AdjustToContents);

    hh->setSectionResizeMode(QHeaderView::ResizeToContents);
    hh->setStyleSheet("QHeaderView::section { background-color: white}"); // цвет шапки табл. выпадающего списка
    vh->setVisible(false);
    tv->setHorizontalHeader(hh);
    tv->setVerticalHeader(vh);
    tv->setSelectionBehavior(QAbstractItemView::SelectRows);

    tv->setModel(&model);          // подключаем модель к tableView - это наш будущий выпадающий список
    combo_box->setModel(&model);   // подключаем модель к comboBox
    combo_box->setView(tv);        // определяем вид выпадающего списка
}
//----------------------------------------------------------------------------------
// считывание данных для модели из файла
QString NetCONNECT_dial::fileModelRead(QString file_name)
{
    QFile file(file_name); //qApp->applicationDirPath()+"/"+file_name/*mode1.txt"*/);
    QString content{""};

   if (file.exists()){

     if (file.open(QIODevice::ReadOnly|QIODevice::Text)){ // Проверяем, возможно ли открыть наш файл для чтения
           content = file.readAll();                       //считываем все данные с файла
           file.close();
     }
   }
   return content;
}
//--------------------------------------------------------------------------------------
// выполнение настроек по умолчанию
void NetCONNECT_dial:: slotDefaultSettings()
{
   defaultSettingsToVal();   // предполагается что параметры по умолчанию корректные (crc не проверяется)
   valToWgt();               // занесение данных из поля класса на форму
   writeDataToIni();         // запись данных с формы в ини файл
}
//---------------------------------------------------------------------------------
// считывание данных из файла conf. на форму
void NetCONNECT_dial::readSettFromIni()
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
        // !    QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует или поврежден.\n Будут загружены данные по умолчанию.","");  // предупредим
            defaultSettingsToVal();     // перезапишем данными по умолчанию
            int param = findEnableDCE_ID(sett);
            if(param){
              m_netCONNECT_param->dceId = param;  // если секция EnableDCE не найдена или повреждена, значение параметра останется по умолчанию
              m_netCONNECT_param->dceIdCorr = param + 1;
            }
            valToWgt();                 // в виджет
            writeDataToIni();           // и сохраним в файл
       }
    }
    else{
     // !   QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует .\n Будут загружены данные по умолчанию.","");  // предупредим
        defaultSettingsToVal();
        valToWgt();                 // в виджет
        writeDataToIni();           // и сохраним в файл
    }
}
//---------------------------------------------------------------------------------
// запись данных с формы в файл настроек
void NetCONNECT_dial::writeDataToIni()
{
       // создадим объект настроек заново и считаем в него данные из виджетов формы
       QSettings  *sett = new QSettings(getPathToIniFile(), QSettings::IniFormat/*NativeFormat*/);

       sett->remove(SECTION_NAME);
       sett->setValue(SECTION_NAME+"/DCE_ID", ui->sb_DCE_ID->value());
       sett->setValue(SECTION_NAME+"/DCE_ID_OPPN", ui->sb_DCE_ID_OPPN->value());
       sett->setValue(SECTION_NAME+"/ImitINS", ui->sb_imit->value());
       sett->setValue(SECTION_NAME+"/bandWidth", ui->cb_regim_bw->currentIndex());
       sett->setValue(SECTION_NAME+"/modeSwitch", ui->cb_regim_switch->currentIndex());
       sett->setValue(SECTION_NAME+"/modeExch1", ui->cb_regim_transit->currentIndex());
       sett->setValue(SECTION_NAME+"/modeRateSwitch", ui->cb_rate_switch->currentIndex());
       sett->setValue(SECTION_NAME+"/modeChangeFreq", ui->cb_regim_change_freq->currentIndex());
       sett->setValue(SECTION_NAME+"/modeExch2", ui->rb_half_duplex->isChecked()*0 + ui->rb_duplex->isChecked()*1 +ui->rb_simplex->isChecked()*2);
       sett->setValue(SECTION_NAME+"/radioNetType", ui->cb_regim_comb_net->currentIndex());
       sett->setValue(SECTION_NAME+"/priority", ui->cb_priority->currentIndex());
       sett->setValue(SECTION_NAME+"/rate", ui->cb_rate->currentIndex());
       sett->setValue(SECTION_NAME+"/freeParam", ui->sb_freeParam ->value());
       sett->setValue(SECTION_NAME+"/haveInfo", ui->rb_have_info->isChecked()*1 + ui->rb_no_have_info->isChecked()*0);
       sett->setValue(SECTION_NAME+"/modeAdapt", ui->rb_freq_yes->isChecked()*1 + ui->rb_freq_no->isChecked()*0);
       sett->setValue(SECTION_NAME+"/modeAutoDisc", ui->cb_auto_discon->isChecked()*1);
       sett->setValue(SECTION_NAME+"/factorNet", ui->sb_factor_net->value());
       sett->setValue(SECTION_NAME+"/factorUser", ui->sb_factor_user->value());

       sett->beginWriteArray(SECTION_NAME+"/synchroPack");
       int size = m_sync_pack.count();
       for(int i = 0; i < size; ++i){
          sett->setArrayIndex(i);
          QString val = m_sync_pack.at(i)->text();
          sett->setValue("val", val);
       }
       sett->endArray();

       unsigned short crc = calcCRCForSettings(sett, SECTION_NAME, CRC_KEY_NAME);
       sett->setValue(SECTION_NAME+ "/" + CRC_KEY_NAME, crc);
}

//---------------------------------------------------------------------------------
// передача данных из поля класса на форму
void NetCONNECT_dial::valToWgt()
{
    modelRegimSett();

    ui->sb_DCE_ID->setValue(m_netCONNECT_param->dceId);
    ui->sb_DCE_ID_OPPN->setValue(m_netCONNECT_param->dceIdCorr);
    ui->sb_imit->setValue(m_netCONNECT_param->imitIns);
    ui->cb_regim_bw->setCurrentIndex(m_netCONNECT_param->modeBW);   //currentText());
    ui->cb_regim_switch->setCurrentIndex(m_netCONNECT_param->modeSwitch);
    ui->cb_regim_transit->setCurrentIndex(m_netCONNECT_param->modeTrans);
    ui->cb_rate_switch->setCurrentIndex(m_netCONNECT_param->modeRateSwitch);
    ui->cb_regim_change_freq->setCurrentIndex(m_netCONNECT_param->modeChangeFreq);


    switch (m_netCONNECT_param->modeEx2){
      case 2 : ui->rb_simplex->setChecked(true);
            break;
      case 0 : ui->rb_half_duplex->setChecked(true);
            break;
      case 1 : ui->rb_duplex->setChecked(true);
            break;
      default:
            break;
      }
    ui->cb_regim_comb_net->setCurrentIndex(m_netCONNECT_param->radioNetType);
    ui->cb_priority->setCurrentIndex(m_netCONNECT_param->priority);
    ui->cb_rate->setCurrentIndex(m_netCONNECT_param->rate);
    ui->sb_freeParam ->setValue(m_netCONNECT_param->freeParam);

    if(m_netCONNECT_param->modeAdapt == 0)
       ui->rb_freq_no->setChecked(true);
     else ui->rb_freq_yes->setChecked(true);

    if(m_netCONNECT_param->haveInfo == 1)
       ui->rb_have_info->setChecked(true);
     else ui->rb_no_have_info->setChecked(true);

    if(m_netCONNECT_param->autoDiscon == 1)
       ui->cb_auto_discon->setChecked(true);
    else ui->cb_auto_discon->setChecked(false);

    ui->sb_factor_net->setValue(m_netCONNECT_param->factorNet);
    ui->sb_factor_user->setValue(m_netCONNECT_param->factorUser);

    for(int i = 0; i < m_netCONNECT_param->syncPack.count(); ++i){
        if (i < m_netCONNECT_param->syncPack.count())
           m_sync_pack.at(i)->setValue(m_netCONNECT_param->syncPack.at(i));
     }
}
//---------------------------------------------------------------------------------
// передача данных из объекта Настроек в поле класса
void NetCONNECT_dial::settKeyToVal(QSettings *sett)
{
    QStringList group = sett->childGroups();  // получаем список секций в ини-файле
    if (group.size() >0){
      for (int i = 0; i < group.size(); i++ ) {
          sett->beginGroup(group.at(i));
          if (group.at(i) == SECTION_NAME){  // основные настройки команды Enable
            QStringList keys = sett->childKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
               if (keys.at(j)=="DCE_ID")
                  m_netCONNECT_param->dceId = sett->value("DCE_ID").toInt();
               else if (keys.at(j)=="DCE_ID_OPPN")
                  m_netCONNECT_param->dceIdCorr = sett->value("DCE_ID_OPPN").toInt();
               else if (keys.at(j)=="ImitINS")
                  m_netCONNECT_param->imitIns = sett->value("ImitINS").toInt();
               else if (keys.at(j)=="bandWidth")
                  m_netCONNECT_param->modeBW = sett->value("bandWidth").toInt();
               else if (keys.at(j)=="modeSwitch")
                  m_netCONNECT_param->modeSwitch = getModeSwitch(sett->value("modeSwitch").toString());
               else if (keys.at(j)=="modeExch1")
                  m_netCONNECT_param->modeTrans = getModeSynchro(sett->value("modeExch1").toString());
               else if (keys.at(j)=="modeExch2")
                  m_netCONNECT_param->modeEx2 = getModeExch(sett->value("modeExch2").toInt());
               else if (keys.at(j)=="modeChangeFreq")
                  m_netCONNECT_param->modeChangeFreq = sett->value("modeChangeFreq").toInt();
               else if (keys.at(j)=="modeRateSwitch")
                  m_netCONNECT_param->modeRateSwitch = sett->value("modeRateSwitch").toInt();
               else if (keys.at(j)=="radioNetType")
                  m_netCONNECT_param->radioNetType = getModeConnectType(sett->value("radioNetType").toString());
               else if (keys.at(j)=="rate")
                  m_netCONNECT_param->rate = sett->value("rate").toInt();
               else if (keys.at(j)=="freeParam")
                  m_netCONNECT_param->freeParam = sett->value("freeParam").toInt();
               else if (keys.at(j)=="haveInfo")
                  m_netCONNECT_param->haveInfo = getModeIO(sett->value("haveInfo").toInt());
               else if (keys.at(j)=="modeAdapt")
                  m_netCONNECT_param->modeAdapt = getModeAdapt(sett->value("modeAdapt").toInt());
               else if (keys.at(j)=="modeAutoDisc")
                  m_netCONNECT_param->autoDiscon = getModeAdapt(sett->value("modeAutoDisc").toInt());
               else if (keys.at(j)=="factorNet")
                  m_netCONNECT_param->factorNet = sett->value("factorNet").toInt();
               else if (keys.at(j)=="factorUser")
                  m_netCONNECT_param->factorUser = sett->value("factorUser").toInt();
               else if (keys.at(j)=="priority")
                  m_netCONNECT_param->priority = getModePriority(sett->value("priority").toString());
               int size = sett->beginReadArray("synchroPack");
               m_netCONNECT_param->syncPack.clear();
               for(int j = 0; j <size; ++j){
                   sett->setArrayIndex(j);
                   m_netCONNECT_param->syncPack.append(sett->value("val").toInt());
               }
               sett->endArray();
            }   // end for keys
          }    // end if disable-param

      sett->endGroup(); // завершение обхода группы "NetCONNECT"
    }  // end for group
  }    // end if group size > 0

    // уточним значение DCE_ID в секции Enable_DCE
    int param = findEnableDCE_ID(sett);
    if(param)
      m_netCONNECT_param->dceId = param;
}
//---------------------------------------------------------------------------------
// поиск без проверки целостности секции "EnableDCE" в ини файле
unsigned int NetCONNECT_dial::findEnableDCE_ID(QSettings  *sett)
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
void NetCONNECT_dial::defaultSettingsToVal()
{
    m_netCONNECT_param->dceId = 1;
    m_netCONNECT_param->dceIdCorr = m_netCONNECT_param->dceId + 1;
    m_netCONNECT_param->imitIns = 1;
    m_netCONNECT_param->modeBW = 3100;
    m_netCONNECT_param->modeEx2 = ModeExchEnum::HALF_DUPLEX;
    m_netCONNECT_param->modeSwitch = ModeSwitchEnum::FRCH;
    m_netCONNECT_param->modeTrans = ModeSyncExchEnum::SYNC;
    m_netCONNECT_param->modeChangeFreq = 75;
    m_netCONNECT_param->modeRateSwitch = 200;
    m_netCONNECT_param->radioNetType = ModeNetConnectEnum::POINT;
    m_netCONNECT_param->rate = 1200;
    m_netCONNECT_param->freeParam = 12;
    m_netCONNECT_param->haveInfo = ModeYesNoEnum::NO;
    m_netCONNECT_param->modeAdapt = ModeOffOnEnum::OFF;
    m_netCONNECT_param->autoDiscon = ModeOffOnEnum::ON;
    m_netCONNECT_param->factorNet = 3;
    m_netCONNECT_param->factorUser = 2;
    m_netCONNECT_param->priority = ModePriorityEnum::M;

    m_netCONNECT_param->syncPack.clear();
    m_netCONNECT_param->syncPack.append(200);
    m_netCONNECT_param->syncPack.append(35);
    m_netCONNECT_param->syncPack.append(70);

}
//---------------------------------------------------------------------------------------
// кнопка ПРИНЯТЬ
// здесь наш внешний объект должен быть заполнен целиком
void NetCONNECT_dial::slotApplay()
{
    wgtToVal();
                                   // и надеемся, что ничего не упадет!
    writeDataToIni();               // кроме этого данные с формы должны быть сохранены в conf файле с настройками

    accept();                       // закрываем форму с флагом ПРИНЯТО
}
//---------------------------------------------------------------------------------
// передача значений из виджетов формы в поле класса
void NetCONNECT_dial:: wgtToVal()
{
    m_netCONNECT_param->dceId = ui->sb_DCE_ID->value();
    m_netCONNECT_param->dceIdCorr = ui->sb_DCE_ID_OPPN->value();
    m_netCONNECT_param->imitIns = ui->sb_imit->value();
    m_netCONNECT_param->modeBW = ui->cb_regim_bw->currentText().toInt();
    m_netCONNECT_param->modeSwitch = getModeSwitch(ui->cb_regim_switch->currentText());
    m_netCONNECT_param->modeChangeFreq = ui->cb_regim_change_freq->currentIndex();
    m_netCONNECT_param->modeRateSwitch = ui->cb_rate_switch->currentIndex();
    m_netCONNECT_param->modeEx2 = getModeExch(ui->rb_half_duplex->isChecked()*0 +
                                              ui->rb_duplex->isChecked()*1 +
                                              ui->rb_simplex->isChecked()*2);
    m_netCONNECT_param->modeTrans = getModeSynchro(ui->cb_regim_transit->currentText());
    m_netCONNECT_param->radioNetType = getModeConnectType(ui->cb_regim_comb_net->currentText());
    m_netCONNECT_param->rate = ui->cb_rate->currentIndex();
    m_netCONNECT_param->freeParam = ui->sb_freeParam->value();
    m_netCONNECT_param->haveInfo = getModeIO(ui->rb_have_info->isChecked()*1 +
                                    ui->rb_no_have_info->isChecked()*0);
    m_netCONNECT_param->modeAdapt = getModeAdapt(ui->rb_freq_no->isChecked()*0
                                                 + ui->rb_freq_yes->isChecked()*1);
    m_netCONNECT_param->autoDiscon = getModeAdapt(ui->cb_auto_discon->isChecked()*1);
    m_netCONNECT_param->factorNet = ui->sb_factor_net->value();
    m_netCONNECT_param->factorUser = ui->sb_factor_user->value();
    m_netCONNECT_param->priority = getModePriority(ui->cb_priority->currentText());

    m_netCONNECT_param->syncPack.clear();
    for (int i = 0; i < m_sync_pack.count(); ++i)
       m_netCONNECT_param->syncPack.append(m_sync_pack.at(i)->value());
}
//--------------------------------------------------------------------------------------------
//  кнопка ОТМЕНИТЬ
void NetCONNECT_dial::slotCancel()
{
    reject();                      // отменяем все наши действия на форме, выходим с флагом ОТМЕНЕНО
}
//--------------------------------------------------------------------------------------------
// Задание порядка обхода элементов формы
void NetCONNECT_dial::tabOrderSett()
{
    ui->pb_default->setFocus();
    setTabOrder(this, ui->pb_default);
    setTabOrder(ui->pb_default, ui->sb_DCE_ID);
    setTabOrder(ui->sb_DCE_ID, ui->sb_DCE_ID_OPPN);
    setTabOrder(ui->sb_DCE_ID_OPPN, ui->cb_priority);
    setTabOrder(ui->cb_priority, ui->sb_freeParam);
    setTabOrder(ui->sb_freeParam, ui->rb_simplex);
    setTabOrder(ui->rb_simplex,ui->rb_half_duplex);
    setTabOrder(ui->rb_half_duplex, ui->rb_duplex);
    setTabOrder(ui->rb_duplex, ui->cb_regim_bw);
    setTabOrder(ui->cb_regim_bw, ui->cb_regim_change_freq);
    setTabOrder(ui->cb_regim_change_freq, ui->cb_rate_switch);
    setTabOrder(ui->cb_rate_switch,ui->cb_regim_switch);
    setTabOrder(ui->cb_regim_switch, ui->cb_stage);
    setTabOrder(ui->cb_stage, ui->cb_regim);
    setTabOrder(ui->cb_regim,ui->cb_regim_comb_net);
    setTabOrder(ui->cb_regim_comb_net,ui->cb_regim_transit);
    setTabOrder(ui->cb_regim_transit, ui->cb_rate);
    setTabOrder(ui->cb_rate, ui->rb_have_info);
    setTabOrder(ui->rb_have_info, ui->rb_no_have_info);
    setTabOrder(ui->rb_no_have_info,ui->rb_freq_no);
    setTabOrder(ui->rb_freq_no,ui->rb_freq_yes);
    setTabOrder(ui->rb_freq_yes, ui->cb_auto_discon);
    setTabOrder(ui->cb_auto_discon, ui->sb_factor_net);
    setTabOrder(ui->sb_factor_net,ui->sb_factor_user);
    setTabOrder(ui->sb_factor_user, ui->sb_imit);
    setTabOrder(ui->sb_imit, m_sync_pack.at(0));

    for (int i  = 0; i < m_sync_pack.count()-1; ++i){
       setTabOrder(m_sync_pack.at(i), m_sync_pack.at(i+1));
    }
    setTabOrder(m_sync_pack.at(m_sync_pack.count()-1), ui->pb_applay);
    setTabOrder(ui->pb_applay, ui->pb_cancel);
}
//----------------------------------------------------------------------------------------
