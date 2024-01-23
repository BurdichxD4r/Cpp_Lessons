#include "widget_dce.h"
#include "ui_widget_dce.h"
#include <QHostInfo>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

QString     DCE_INI_PATH;   // путь к каталогу настроек dce
QString     APP_ABS_PATH;   // абс. путь к исполн. прилож
QString     DCE_INI_FL;     // имя файла настоек dce

//----------------------------------------------------------------------------------------------------
Widget_DCE::Widget_DCE(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget_DCE)
{
    ui->setupUi(this);

    APP_ABS_PATH = QCoreApplication::applicationDirPath();
    DCE_INI_PATH     = "/INI/";     // дир. с файлами конфигур.
    DCE_INI_FL       = "dce.ini";   // имя файла конфигур.

    qRegisterMetaType<NetConnectS>();

    connect(ui->pb_net, SIGNAL(clicked()), this, SLOT(slotNetWorking()));  // кнопка  "Старт Сервера"
    connect(ui->rb_json,SIGNAL(clicked()), this, SLOT(slotJson()));
    connect(ui->rb_xml,SIGNAL(clicked()), this, SLOT(slotXml()));
    connect(ui->le_port, SIGNAL(textChanged(QString)), this, SLOT(slotTitleChange(QString)));
    connect(ui->chB_Journal,SIGNAL(clicked(bool)), this, SLOT(slotJournalView(bool)));
    connect(ui->ch_ID, SIGNAL(clicked(bool)), this, SLOT(slotDCE_ID_DST(bool)));
    connect(ui->sb_id, SIGNAL(valueChanged(QString)), this, SLOT(slotDCEIdChange(QString)));
    connect(ui->chB_busy, SIGNAL(clicked(bool)), this, SLOT(slotDCESetBusy(bool)));
    connect(ui->cb_stage, SIGNAL(currentIndexChanged(int)), this , SLOT(slotDCESetStage(int)));
    connect(ui->cb_depth, SIGNAL(currentTextChanged(QString)), this, SLOT(slotDepthChanged(QString)));


       // связь сигнала из ModemPanel c signal для передачи через сервер обмена
     connect(this, &Widget_DCE::sigIQToRadio, ui->widget_IQ, &Widget_Exch_IQ::slotIQToExch);   //
     connect(this, &Widget_DCE::sigBfSht, ui->widget_IQ, &Widget_Exch_IQ::slotBfShift);
     connect(ui->widget_IQ, &Widget_Exch_IQ::sigCodoFromCorr, this, &Widget_DCE::slotGetIQFromRadio);
     connect(ui->widget_IQ, &Widget_Exch_IQ::sigDisconEx, this, &Widget_DCE::slotDisconRadio);
     connect(this, &Widget_DCE::sigIQEnergy, ui->widget_IQ, &Widget_Exch_IQ::slotIQEnergy);
     connect(ui->widget_IQ, &Widget_Exch_IQ::sigGetSlotSize, this, &Widget_DCE::slotGetSlotSize);

   // if(ui->chB_Journal->isChecked()){
    slotJournalView(ui->chB_Journal->isChecked());
  //  }
   // connect(ui->lePort, SIGNAL(textChanged(QString)), this, SLOT(slotSetStatusForm(QString)));

    initWidget();

    connect(this, &Widget_DCE::sigIQFromCorr, ui->widget_modem, &CoreDCE::slotIQFromCorr);
    connect(ui->widget_modem, &CoreDCE::sigIQEnergy, this, &Widget_DCE::sigIQEnergy);
    connect(ui->widget_modem, &CoreDCE::sigBfShift,this, &Widget_DCE::sigBfSht);

    connect(this, &Widget_DCE::sigExchDCE, ui->widget_modem, &CoreDCE::slotTempExchDCE); // сигнал о пришедшей команде ExchangeDCE
    connect(ui->widget_modem, &CoreDCE::sigEnqueueAnsw, this, &Widget_DCE::slotPrepareEnqueuedAnsw);
    connect(ui->widget_modem, &CoreDCE::sigStartAnsw, this, &Widget_DCE::slotPrepareStartAnsw);
    connect(ui->widget_modem, &CoreDCE::sigEndAnsw, this, &Widget_DCE::slotPrepareEndAnsw);
    connect(ui->widget_modem, &CoreDCE::sigExchDCEAnsw, this, &Widget_DCE::slotCmdExchangeDCEAnsw);

    // определение ip адреса машины на которой запущено приложение
    QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();
    m_ip_adr.clear();
  //  for(int i = 0; i <listAddr.count(); ++i ){
  //      m_ip_adr = m_ip_adr+" * " + listAddr.at(i).toString()+" * ";
  //  }
    m_ip_adr = listAddr.at(0).toString();
    ui->lb_status_line->setText(m_ip_adr);

    emit ui->chB_busy->clicked();
    ui->cb_stage->setCurrentIndex(1);

    connect(ui->lw_logs, &QListWidget::itemSelectionChanged, this, & Widget_DCE::slotLogFileListChanged);
}
//---------------------------------------------------------------------------------------------------
Widget_DCE::~Widget_DCE()
{
    delete ui;
}
//-----------------------------------------------------------------------
void Widget_DCE::setTypeMsg(QString type_msg)
{
     // тип сообщения
    if (type_msg=="JSON"){
       ui->rb_json->setChecked(true);
       m_type_msg=JSON;
    }
    else if (type_msg=="XML"){
       ui->rb_xml->setChecked(true);
       m_type_msg=XML;
    }
}
//-----------------------------------------------------------------------
void Widget_DCE::setQueueDepth(int queDepth)
{
    ui->cb_depth->setCurrentText(QString::number(queDepth));
}
//-----------------------------------------------------------------------
// режим auto_start=0; из EnableDCE
void Widget_DCE::servDCEStartOnly(int port_tcp, QString type_msg){
    ui->le_port->setText(QString::number(port_tcp));
    setTypeMsg(type_msg);   // тип сообщения
    slotNetWorking();       // кнопка Старт Сервер
}
//-----------------------------------------------------------------------
// режим auto_start=1; из ini + cmd_line
void Widget_DCE::settAndStartDCE(QString type_msg, QStringList &other_params)
{
    // делаем доступными элементы управления работы с сервером при авто запуске сервера (без команды EnableDCE)
    ui->pb_net->setEnabled(true);
    ui->sb_id->setEnabled(true);
    ui->sb_id_dst->setEnabled(true);

    m_mode_start = 1;       // режим авто запуска модема (с использованием ini)
    setTypeMsg(type_msg);   // тип сообщения
    QJsonObject settObj = readSettFromIni();       // считываются данные ini файла             

    QString radioRole;      // определим радио роль в сети
    if(settObj.contains("radioDirName")){
        QString paramDir = settObj.value("radioDirName").toString();
        QStringList list = paramDir.split(':');
        for (int i = 0; i <list.count(); ++i){// выделение данных роли в сети RX-ПРМ,  TX-ПРД
            if((list.at(i)=="TX")||(list.at(i)=="RX")){
                radioRole = list.at(i);
            }
        }
     }
     // определяем из считанных параметров радио роль объекта
    // подменим в JSON объекте из ini файла некоторые параметры данными из cmd_line
    if (radioRole=="TX"){  // работаем как ПРД
            settObj.insert("outputAddr", other_params.at(2));
            settObj.insert("outputPort", other_params.at(1).toInt());
            settObj.insert("udpPort", other_params.at(3).toInt());
            QJsonArray arr;
            arr.append(m_port);
            settObj.insert("inputPort",arr);
     }
     else if (radioRole=="RX"){  // работаем как ПРM

            QJsonArray arr;
            arr.append(other_params.at(3).toInt());
            settObj.insert("inputPort",arr);
            settObj.insert("outputPort", m_port);
            settObj.insert("udpPort", other_params.at(3).toInt());
     }

      // поле Глубина Очереди обрабатывается отдельно, его нет в команде EnableDCE
    if(settObj.contains("queueDepth")){  // если поле есть в ини файле, то
       int queDepth = settObj.value("queueDepth").toInt();
       queDepth = other_params.at(5).toInt();      // считываем Глубину Очереди из cmd
       setQueueDepth(queDepth);
    }
    else   // если поля нет в ини файле, по умолчанию 0
       setQueueDepth(0);

    if (settObj.contains("DCE_ID")){
        settObj.insert("DCE_ID",other_params.at(0).toInt());
    }
    if (settObj.contains("ppfIndex")){
        settObj.insert("ppfIndex",other_params.at(4).toInt());
    }

    MsgDescrS msg;     // создадим структуру для передачи объекта с настройками дальше
    msg.obj_msg_in = settObj;
    cmdEnableDCEProc(msg);                         // вызываем ф-цию обработки как будто это команда EnableDCE


    startDCE();                                    // запуск сервера

}
//----------------------------------------------------------------------------------------------------
// считывание данных из файла dce.ini
QJsonObject Widget_DCE::readSettFromIni()
{
    QJsonObject settObj;
    QString path_ini = settFileExist();
    if(path_ini!=""){   // если файл с настройками существует
        settObj = createSettingObj(path_ini);
    }
    else{
     // !   QMessageBox::critical(this, "Сообщение", "Файл настроек отсутствует.\n Будут загружены данные по умолчанию.","");  // предупредим
       settObj = defaultSettLoad();           // настройки по умолчанию
       saveSettToIni(settObj);                // и сохраним в файл ини
    }
    return settObj;
}
//-----------------------------------------------------------------------------
// настройки работы DCE по умолчанию
QJsonObject Widget_DCE::defaultSettLoad()
{
    // заполняем основные поля объекта
    QStringList field_name_list;
    QStringList field_val_list;

    QStringList field_name_list1;
    QStringList field_val_list1;

    QJsonObject json_obj;

    // основные ключи
    field_name_list<<"DCE_ID"
                   <<"freqOffset"
                   <<"frequencyAll"
                   <<"freqActive"
                   <<"pcpKey"
                   <<"initPcpTime"
                   <<"modeFreqPcp"
                   <<"rcvrPort"
                   <<"ppfNumber"
                   <<"outputPort"
                   <<"modeMDM"
                   <<"samplingRate"
                   <<"queueDepth";  /// !!!это поле не входит в ком. EnableDCE

    field_val_list<< "1"
                 << "0"
                 << "124"
                 << "2"
                 << "55"
                 << "1234567"
                 << "6"
                 << "9999"
                 << "2"
                 << "4001"
                 << "5"
                 << "8000"
                 << "0"   ;

    for (int i = 0; i < field_name_list.count(); ++i){
         json_obj.insert(field_name_list.at(i), field_val_list.at(i).toInt());
    }

    field_name_list1<<"modeSwitch"<<"modeExch1"<<"bandWidth"<<"rateSwitch"
                    <<"frequencyAdaptation"<<"constellationAdaptation"
                    <<"inputProt"<<"outputProt"<<"outputAddr"
                    <<"ppfSwitch"<<"radioDirName"<<"rcvrAddr"<<"rcvrProt";

    QString modeSw{"FRCH"};
    QString modeEx{"half_duplex"};
    QString adapt{"off"};
    QString adapt1{"off"};

    field_val_list1
                    <<modeSw
                    <<modeEx
                    <<QString::number(3100)
                    <<QString::number(200)
                    <<adapt
                    <<adapt1
                    <<"JSON"
                    <<"RTP"
                    <<"127.0.0.2"
                    <<"auto"
                    <<"456:RX:0:1"
                    <<"0.0.0.1"
                    <<"HTTP";


    for (int i = 0; i < field_name_list1.count(); ++i){
         json_obj.insert(field_name_list1.at(i), field_val_list1.at(i));
    }
    // список портов
    QJsonArray objArr_port;

    QList<int> inputPorts;
    inputPorts.append(57501);
    inputPorts.append(57502);

    for (int i = 0; i < inputPorts.count(); ++i)
        objArr_port.append(inputPorts.at(i));

    // список индексов частот
    QJsonArray objArr_ppfInd;

    QList<int> ppfIndexes;
    ppfIndexes.append(1);
    ppfIndexes.append(2);

    for (int i = 0; i < ppfIndexes.count(); ++i)
        objArr_ppfInd.append(ppfIndexes.at(i));

    // список частот
    QJsonArray objArr_freq;

    QList<int> freqList;
    freqList.append(2000000);
    freqList.append(3000000);

    for (int i = 0; i < freqList.count(); ++i)
        objArr_freq.append(freqList.at(i));

    // список частот НПП
    QJsonArray objArr_freq_prim;
    QList<int> freqPrim;
    freqPrim.append(2);
    freqPrim.append(4);

    for (int i = 0; i < freqPrim.count(); ++i)
        objArr_freq_prim.append(freqPrim.at(i));

    // список частот ЗЧ
    QJsonArray objArr_freq_add;
    QList<int> freqAdd;
    freqAdd.append(6);
    freqAdd.append(8);

    for (int i = 0; i < freqAdd.count(); ++i)
        objArr_freq_add.append(freqAdd.at(i));

   QJsonArray objArr_param;
    // параметры радиосредств
   for (int j = 0; j < 2; ++j){
        QJsonObject objItem1;
        field_name_list.clear();
        QStringList rs;
         rs << "TX:1";
         rs << QString::number(4010);
         rs << QString::number(5780);
         rs << QString::number(5781);
         rs << "127.0.0.1";

         objItem1.insert("radioName", rs.first());
         field_name_list<< "radioTcpPort"<<"radioUdpPort1"<<"radioUdpPort2";
         for (int i = 0; i < field_name_list.count(); ++i)
            objItem1.insert(field_name_list.at(i), rs.at(i+1).toInt());
         objItem1.insert("radioIPv4", rs.last());
         objArr_param.append(objItem1);
   }
    json_obj.insert("inputPort",objArr_port);
    json_obj.insert("ppfIndex",objArr_ppfInd);
    json_obj.insert("frequencyList",objArr_freq);
    json_obj.insert("frequencyPrimList",objArr_freq_prim);
    json_obj.insert("frequencyAddList",objArr_freq_add);
    json_obj.insert("radioParams",objArr_param);

    return json_obj;

}
//-----------------------------------------------------------------------------------
// сохранение json в файл "dce.ini"
void Widget_DCE::saveSettToIni(QJsonObject &jObj)
{
    QString     fl_path;
    QString     fl_dir = APP_ABS_PATH + DCE_INI_PATH;

     if (!checkDirExist(fl_dir))     // если текущей дир. не существ.
            if(!makeDir(fl_dir))      // пытаемся ее создать
               {}
     fl_path = APP_ABS_PATH + DCE_INI_PATH + DCE_INI_FL;
     QFile *fl = new QFile(fl_path);

     if (!fl->exists()){
        if(!fl->open(QIODevice::WriteOnly|QIODevice::Append)){
            }
         else { // если файл создан
                  //оставим вновь созданный файл пустым
               fl->close();
               delete fl;
         }
      }
      QFile myfile(fl_path);
      if (myfile.isOpen())
          myfile.close();

      QJsonDocument jDoc(jObj);
      QFile file(fl_path);
      if(file.open(QIODevice::WriteOnly)){
          file.write(QString(jDoc.toJson()).toStdString().c_str());
          file.close();
      }
}
//-------------------------------------------------------------------
// проверка существования директории с заданным имененм
bool Widget_DCE:: checkDirExist(QString &dir_name_)
{
    QDir dir(dir_name_);
    if (!dir.exists()){
     //   QMessageBox::critical(0,"Ошибка","Директория:  "+
     //                         dir_name_+
     //                         "  НЕ существует. Мы попробуем её создать." );
        return false;
    }
    else
        return true;
}
//---------------------------------------------------------------------
// создание директории с заданным имененм
bool Widget_DCE:: makeDir(QString &dir_name_)
{
    QDir dir(dir_name_);
    if (!dir.mkdir(dir_name_)){
        QMessageBox::critical(0,"Ошибка","Директория:  "+
                              dir_name_+
                              "  НЕ может быть создана.Проверте права доступа." );
        return false;
    }
    else
        return true;
}
//-----------------------------------------------------------------------------
// процедура создания списка настроечных параметров
// создается по данным файла  "dce.ini"
QJsonObject Widget_DCE::createSettingObj(QString fl_ini_name)
{
    QFile settingFile (fl_ini_name);
    QString json_str;
    QJsonParseError parseErr;
    QString error = "";
    QJsonObject json_setting;

    if (!settingFile.open(QIODevice::ReadOnly|QIODevice::Text)) // Проверяем, возможно ли открыть наш файл для чтения
            return json_setting;                                              // если это сделать невозможно, то завершаем функцию
       json_str = settingFile.readAll();                         //считываем все данные с файла в объект
    settingFile.close();

    QJsonDocument doc;

    doc = QJsonDocument::fromJson(json_str.toUtf8(), & parseErr);
    if (parseErr.error != QJsonParseError::NoError){
       error = parseErr.errorString();//"JSON-ошибка";
      qDebug()<< "In file: dce.ini - JSON error! ("+error+")";
      }
    else if(doc.isObject()){
         //QJsonObject root_obj = doc.object();
        json_setting = doc.object();
    }
    return json_setting;
}
//---------------------------------------------------------------------------------------------------
// проверка существования файла с настройками.
// если файла не существует (не найден) возвращается пустой путь, если найден, путь к файлу настроек
QString Widget_DCE::settFileExist()
{
    QString     fl_path = APP_ABS_PATH + DCE_INI_PATH + DCE_INI_FL;
    QFile       file(fl_path);

    if(!file.exists())       // если файла настроек не существует
    {
        fl_path ="";
     // QMessageBox::critical(this, "Сообщение", "Файл настоек не существует, \n поэтому будет создан автоматически.","");
      //defaultSettingsToVal();  // заготавливаем данные по умолчанию
    }
    // если файл настроек имеется из функции возвращаем только путь к существующему файлу, конструктор объекта Settings позаботится о
    // считывании данных из этого файла в объект настроек
    return fl_path;
}

//-----------------------------------------------------------------
void Widget_DCE ::startDCE()
{
    slotNetWorking();   // кнопка Старт Сервер
    NetS net_adr;
    net_adr.port = m_port;
    net_adr.address = m_ip_adr;

    ui->lb_title->setText("ПАК УВС # "+ QString::number(m_uvs_obj.DCE_ID));
    emit sigNetAdr(net_adr);
    emit sigDCEIdChng(); // изменился DCE_ID

}
//-----------------------------------------------------------------
QString Widget_DCE ::getRadioRole()                   // радио роль
{
   return m_uvs_obj.radio_role;
}
//-----------------------------------------------------------------
int Widget_DCE::getID_DCE()         // номер модема DCE_ID
{
    return m_uvs_obj.DCE_ID;//m_dce_id;
}
//-----------------------------------------------------------------
int Widget_DCE::getPort()        // порт прослушки TCP
{
    return m_port;
}
//-----------------------------------------------------------------
int Widget_DCE::getServerStatus()   // статус сервера TCp запущен или остановлен
{
    if (m_prtcl_serv == nullptr)  // сервер остановлен
       return 0;
    else
       return 1;   // сервер запущен
}
//-----------------------------------------------------------------
/*void Widget_DCE::setModemAndUdpParams(NetS portAdr, int port_bind,
                                      int index_zpch, int start_buf,
                                      int que_depth)
{
    ui->widget_modem->setQueueDepth(que_depth);
    ui->cb_depth->setCurrentIndex(que_depth);
    ui->widget_IQ->setParams(portAdr, port_bind, index_zpch, start_buf);
    startRadio();
}
*/
//-----------------------------------------------------------------
// обработка запроса радио канал о размере слота модема
void  Widget_DCE::slotGetSlotSize()
{
    ui->widget_IQ->setSlotSize(ui->widget_modem->getSlotLength());
}
//-----------------------------------------------------------------
void Widget_DCE::setID_DCE(int id_dce)
{
    ui->sb_id->setValue(id_dce);
    m_uvs_obj.DCE_ID = id_dce;
}
//-----------------------------------------------------------------
// задание номера порта из командной стр.
void Widget_DCE::setPort(int port)
{
    ui->le_port->setText(QString::number(port));
    m_port = port;
    //slotNetWorking();   // запускаем сервер УВС при старте приложения
}
//-------------------------------------------------------------------
// установка глубины очереди передачи
void  Widget_DCE::slotDepthChanged(QString  depth)
{
  ui->widget_modem->setQueueDepth(depth.toInt());
}
//------------------------------------------------------------------
void Widget_DCE::slotDCESetBusy(bool busy)
{
    emit sigSetBusy(busy);
}
//------------------------------------------------------------------
void Widget_DCE::slotDCESetStage(int index)
{
    emit sigSetStage(index);
}
//------------------------------------------------------------------
void Widget_DCE::slotJournalView(bool state)
{
    if(state){
      connect(this, &Widget_DCE::sigWriteInJourn, ui->widget_journ, &WidgetMess::slotAddMsg);
      ui->widget_journ->setVisible(true);
    }
    else{
      disconnect(this, &Widget_DCE::sigWriteInJourn, ui->widget_journ, &WidgetMess::slotAddMsg);
      ui->widget_journ->setVisible(false);
    }
}
//-----------------------------------------------------------------
void  Widget_DCE::slotDCE_ID_DST(bool use)
{
    emit sigUseDST(use);
}
//-----------------------------------------------------------------
void   Widget_DCE:: slotSetTitleForm(QString title)         // задание заголовка формы
{
    ui->lb_title->setText(title);
}
//------------------------------------------------------------------
void   Widget_DCE:: slotSetStatusForm(QString status_line)  // задание статус бара формы
{
    QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();
    m_ip_adr.clear();

   // for(int i = 0; i <listAddr.count(); ++i ){
   //      m_ip_adr = m_ip_adr +" * " + listAddr.at(i).toString() +" * ";
   //   }

    m_ip_adr = listAddr.at(0).toString();

    ui->lb_status_line->setText("("+ui->le_port->text()+ ") ПАК УВС  ip[ "+ m_ip_adr+" ]");
    ui->lb_status_line->setText(status_line);
}
//-------------------------------------------------------------------
// изменения на форме DCE_ID
void Widget_DCE::slotDCEIdChange(QString val)
{
    m_uvs_obj.DCE_ID = val.toInt();
    //emit sigDCEIdChng(); // изменился DCE_ID

}
//------------------------------------------------------------------
void Widget_DCE::slotTitleChange(QString str)
{
   // m_port =str.toInt();
   // emit sigPortChng(str);    // изменился port
     Q_UNUSED(str);
   // определение ip адреса машины на которой запущено приложение

    QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();

    ipAdr = "";    //listAddr.first().toString();
    for(int i = 0; i <listAddr.count(); ++i ){
        ipAdr = ipAdr+" * " + listAddr.at(i).toString()+" * ";
    }
   // connect(this, &Widget_DCE::sigCodoFromCorr, ui->widget_modem, &CoreDCE::slotCodoFromCorr);
    //!MainWindDCE::setWindowTitle("("+ui->lePort->text()+ ") ПАК УВС  ip[ "+ ipAdr+" ]");
}
//------------------------------------------------------------------
//   задание типа сообщения json
void Widget_DCE::slotJson()
{
    if(ui->rb_json->isChecked()){
       m_type_msg  = JSON;
    }
    else{
        m_type_msg  = XML;
    }
}
//------------------------------------------------------------------
//   задание типа сообщения xml
void Widget_DCE::slotXml()
{

    if(ui->rb_xml->isChecked()){
       m_type_msg  = XML;
    }
    else{
       m_type_msg  = JSON;
    }
}
//--------------------------------------------------------------------------
//   запуск Сервера / остановка сервера (кнопка)
void Widget_DCE::slotNetWorking()
{
    QString title = ui->pb_net->text();

    if(title == "Старт Сервер"){ // сервер запущен
        ui->pb_net->setText("Стоп Сервер");
        ui->chB_format->setEnabled(false);
        startServ();
    }
    else if(title == "Стоп Сервер"){  // сервер остановлен
        ui->pb_net->setText("Старт Сервер");
        ui->chB_format->setEnabled(true);
        stopServ();
    }
    emit sigDCEChangeStatus();   // изменение статуса работы сервера TCP
}
//---------------------------------------------------------------------------
void Widget_DCE::startRadio()
{
    ui->widget_IQ->setBinding();
}
//---------------------------------------------------------------------------
void Widget_DCE::stopRadio()
{
    ui->widget_IQ->setDisconnect();
}
//---------------------------------------------------------------------------
// "Старт Сервера"
void Widget_DCE::startServ()
{
    // считываем адрес/порт для работы в сети
    NetS            netAdress;
    netAdress.port = ui->le_port->text().toInt();
    netAdress.address = "127.0.0.1"; 

     // для отображения в MainWind

    m_port =ui->le_port->text().toInt();
    //if(m_mode_start){ //если режим 1=auto включение без ком. EnableDCE
    turnOnModem(m_modem_param_obj); // включаем модем
    //}
    ui->lb_title->setText("ПАК УВС # "+ QString::number(m_uvs_obj.DCE_ID)); // обновим заголовок формы

    ui->rb_json->setEnabled(false);
    ui->rb_xml->setEnabled(false);

    m_prtcl_serv = new ProtocolServ(netAdress, m_type_msg, ui->chB_format->isChecked());

    connect (this, &Widget_DCE::sigNetAdr, m_prtcl_serv, &ProtocolServ::slotSetNetAdr);
    connect(this, &Widget_DCE::sigUseDST, m_prtcl_serv, &ProtocolServ::slotUseDST); 
    connect(this, &Widget_DCE::sigCmdToClient, m_prtcl_serv, &ProtocolServ::slotCmdToClient);
    connect(this, &Widget_DCE::sigCmdToClientFromRadio, m_prtcl_serv, &ProtocolServ::slotCmdToClientFromChanel);
    connect (ui->widget_modem, &CoreDCE::sigIQSignalReady, this, &Widget_DCE::slotIQSigToRadio);
    connect(this, &Widget_DCE::sigStopServ, m_prtcl_serv, &ProtocolServ::sigDisconnect);
    connect(this, &Widget_DCE::destroyed, m_prtcl_serv, &ProtocolServ::sigDisconnect);

    connect(m_prtcl_serv, &ProtocolServ::sigAddMsgToJournal, this, &Widget_DCE::slotSendPackFromTransport);
    connect(m_prtcl_serv, &ProtocolServ::sigUpdateClientList, this, &Widget_DCE::slotSendListClientFromConnect);
    connect(m_prtcl_serv, &ProtocolServ::sigChangeWorkStatus, this, &Widget_DCE::slotSendErrFromConnect);
    connect(m_prtcl_serv, &ProtocolServ::sigCmdFromClient, this, &Widget_DCE::slotCmdFromClient); // команда от клиента
    connect(this, &Widget_DCE::sigErrorTurnOn, m_prtcl_serv, &ProtocolServ::slotErrorFromTurnOnModemRadio);

    m_uvs_obj.create_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    m_uvs_obj.enable_time = 0;
    m_uvs_obj.stage = INIT;
    m_uvs_obj.busy = 1;

    emit sigPortChng(ui->le_port->text());    // изменился port в MainWindow
    emit sigDCEIdChng();                      // возможно изменился dce_id в MainWindow

}
//--------------------------------------------------------------------
//  Остановка Сервера
void Widget_DCE::stopServ()
{
   // остановка сервера

    ui->pb_net->setText("Старт Сервер");
    ui->pb_net->setEnabled(true);

    ui->rb_json->setEnabled(true);
    ui->rb_xml->setEnabled(true);
    ui->lb_title->setText("ПАК УВС # ...");
    ui->widget_modem->set_turnOn_State(false);   // выключим модем

    emit sigStopServ();

    disconnect (this, &Widget_DCE::sigNetAdr, m_prtcl_serv, &ProtocolServ::slotSetNetAdr);
    disconnect(this, &Widget_DCE::sigUseDST, m_prtcl_serv, &ProtocolServ::slotUseDST);
    //disconnect (m_prtcl_serv, &ProtocolServ::sigCmdFromClient, ui->widget_modem, &CoreDCE::slotCmdFromClient); // команда от клиента
    //disconnect(ui->widget_modem, &CoreDCE::sigCmdToClient, m_prtcl_serv, &ProtocolServ::slotCmdToClient);
    //disconnect(ui->widget_modem, &CoreDCE::sigCmdToClientFromRadio, m_prtcl_serv, &ProtocolServ::slotCmdToClientFromChanel);
    disconnect (ui->widget_modem, &CoreDCE::sigIQSignalReady, this, &Widget_DCE::slotIQSigToRadio);
    disconnect(this, &Widget_DCE::sigStopServ, m_prtcl_serv, &ProtocolServ::sigDisconnect);
    disconnect(this, &Widget_DCE::destroyed, m_prtcl_serv, &ProtocolServ::sigDisconnect);
    disconnect(m_prtcl_serv, &ProtocolServ::sigAddMsgToJournal, this, &Widget_DCE::slotSendPackFromTransport);
    disconnect(m_prtcl_serv, &ProtocolServ::sigUpdateClientList, this, &Widget_DCE::slotSendListClientFromConnect);
    disconnect(m_prtcl_serv, &ProtocolServ::sigChangeWorkStatus, this, &Widget_DCE::slotSendErrFromConnect);
    disconnect(this, &Widget_DCE::sigErrorTurnOn, m_prtcl_serv, &ProtocolServ::slotErrorFromTurnOnModemRadio);
    disconnect(m_prtcl_serv, &ProtocolServ::sigCmdFromClient, this, &Widget_DCE::slotCmdFromClient); // команда от клиента

    clearClientPanel();   // очистка панели подключенных клиентов
    delete  m_prtcl_serv;
    m_prtcl_serv = nullptr;

    emit sigPortChng(ui->le_port->text());    // изменился port в MainWindow
    emit sigDCEIdChng();                      // возможно изменился dce_id в MainWindow

}
//-------------------------------------------------------------------
void Widget_DCE::slotCmdFromClient(MsgDescrS & msg)
{
    QString name_com_in = msg.obj_msg_in.value("cmd").toString(); // выделим имя пришедшей команды

  //  if(name_com_in !="TermType" || name_com_in !="Ping" ){     // для этих команд ответ отправляется из verificationLevel

    m_mode_start = 0;    // режим  запуска модема по ком. EnableDCE ()

    msgCmdParse(msg);                                 // парсинг пришедшей команды от клиента
}
//-------------------------------------------------------------------
// парсинг пришедшей команды от клиента
void Widget_DCE::msgCmdParse(MsgDescrS  & msg)
 {
     QString name_com_in = msg.obj_msg_in.value("cmd").toString();    // выделим имя пришедшей команды
     m_uvs_obj.status = "start";   //  ??????
     // Команда Ping обрабатывается в модуле verification,
     // Команда ReplayExchangeDCE,  пришедшая от клиента (status:recv) - не требует ответа и поэтому не обрабатывается

    // обработка пришедших команд в зависимости от типа

     if (name_com_in == "Version" ){
         cmdVerionProc(msg);
     }
     else if (name_com_in == "Status" ){
         cmdStatusProc(msg);
     }
     else if (name_com_in == "GetDCEInfo"){
         cmdGetDCEInfoProc(msg);
     }
     else if (name_com_in == "GetRadioDirInfo"){
         cmdGetRadioDirInfoProc(msg);
     }
     else if (name_com_in == "EnableDCE"){
         cmdEnableDCEProc(msg);
     }
     else if (name_com_in == "DisableDCE"){
         cmdDisableDCEProc(msg);
         cmdDisableDCEAnsw(msg);
     }
     else if (name_com_in == "NetworkSYNC"){
         cmdNetworkSYNCProc(msg);
         cmdNetworkSYNCAnsw(msg);
     }
     else if (name_com_in == "NetworkCONNECT"){
         cmdNetworkCONNECTProc(msg);
         cmdNetworkCONNECTAnsw(msg);
     }
     else if (name_com_in == "NetworkDISCON"){
         cmdNetworkDISCONProc(msg);
         cmdNetworkDISCONAnsw(msg);
     }
     else if (name_com_in == "ExchangeDCE"){
         cmdExchangeDCEProc(msg);
     }
     else if (name_com_in == "ReplyExchangeDCE"){
         cmdReplyDCEProc(msg);
     }

 }
//--------------------------------------------------------------
void Widget_DCE::cmdReplyDCEProc(MsgDescrS & msg)
 {
     cmdReplyDCEAnsw(msg);
 }
//-----------------------------------------------------------------------
//    обработка пришедшей от клиента команды заданного типа
//-----------------------------------------------------------------------
void Widget_DCE::cmdVerionProc(MsgDescrS & msg)
{
    cmdVerionAnsw(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdStatusProc(MsgDescrS & msg)
{
    cmdStatusAnsw(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdGetDCEInfoProc(MsgDescrS & msg)
{
    cmdGetDCEInfoAnsw(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdGetRadioDirInfoProc(MsgDescrS & msg)
{
    cmdGetRadioDirInfoAnsw(msg);
}
//-----------------------------------------------------------------------
// обработка ком. EnableDCE
void Widget_DCE::cmdEnableDCEProc(MsgDescrS & msg)
{
    // парсинг из команды выделяем параметры для модема и для радиосредств
    // создаем отдельно json объекты с параметрами и передаем объекты в соотвю
    // модули для настройки, ожидая ответа об успешности включения модема и радиосредств
    cmdEnableDCEParser(msg, m_modem_param_obj, m_radio_param_obj);     // парсер команды
    getCurrentSetting(msg.obj_msg_in); // заносим параметры из команды в структуру m_uvs_obj
    bool res_m = turnOnModem(m_modem_param_obj);  // включение модема с исп. парам. команды
    turnOnRadio(m_radio_param_obj);  // запуск радио средств с парметрами команды
    bool res_r = !(ui->widget_IQ->getNoConnect()); // получаем флаг об отсутствии  соединения
    QString err_str{""};           // строка с содержанием ошибки
    emit sigPortChng(QString::number(m_uvs_obj.DCE_ID));    // изменился port в MainWindow
    emit sigDCEIdChng();

    if(res_m && res_r){  // если настройки прошли успешно
           if(!m_mode_start){ // если режим включения - команда EnableDCE
             cmdEnableDCEAnsw(msg);     // формируем полож. ответ
             emit sigPortChng(QString::number(m_uvs_obj.DCE_ID));    // изменился port в MainWindow
             emit sigDCEIdChng();                   // возможно изменился dce_id в MainWindow
           }
    }
    else { // если ошибки при настройке, формируем сообщение об ошибке
        if(!m_mode_start){ // если режим включения - команда EnableDCE
           if (!res_m)
              err_str = STR_ERR_MODEM +"; \n";
           if(!res_r)
             err_str = err_str + STR_ERR_RADIO;
            emit sigErrorTurnOn(err_str, msg);  // сигнал в ProtocolServ об ошибке
    }
        else{   // если настраивались из ini
            qDebug()<<"Modem does not start! Error setting! ";
        }
    }
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdEnableDCEParser(MsgDescrS & msg, QJsonObject & modem_obj, QJsonObject & radio_obj)
{

    QJsonObject  msg_in = msg.obj_msg_in;
    QString   radio_role{""};   // роль объекта в радио сети

    //----------------modem---------
    if(msg_in.contains("DCE_ID")){
       modem_obj.insert("DCE_ID", msg_in.value("DCE_ID").toInt());
    }
    if(msg_in.contains("bandWidth")){
       modem_obj.insert("bandWidth", msg_in.value("bandWidth").toString());
    }
    if(msg_in.contains("rateSwitch")){
       modem_obj.insert("rateSwitch", msg_in.value("rateSwitch").toString());
    }
    if(msg_in.contains("modeSwitch")){
       modem_obj.insert("modeSwitch", msg_in.value("modeSwitch").toString());
    }
    if(msg_in.contains("modeExch1")){
       modem_obj.insert("modeExch1", msg_in.value("modeExch1").toString());
    }
    if(msg_in.contains("frequencyAll")){
       modem_obj.insert("frequencyAll", msg_in.value("frequencyAll").toInt());
    }
    if(msg_in.contains("frequencyAdaptation")){
       modem_obj.insert("frequencyAdaptation", msg_in.value("frequencyAdaptation").toString());
    }
    if(msg_in.contains("constellationAdaptation")){
       modem_obj.insert("constellationAdaptation", msg_in.value("constellationAdaptation").toString());
    }
    if(msg_in.contains("pcpKey")){
       modem_obj.insert("pcpKey", msg_in.value("pcpKey").toInt());
    }
    if(msg_in.contains("initPcpTime")){
       modem_obj.insert("initPcpTime", msg_in.value("initPcpTime").toInt());
    }
    if(msg_in.contains("modeFreqPcp")){
       modem_obj.insert("modeFreqPcp", msg_in.value("modeFreqPcp").toInt());
    }
    if(msg_in.contains("samplingRate")){
       modem_obj.insert("samplingRate", msg_in.value("samplingRate").toInt());
       radio_obj.insert("samplingRate", msg_in.value("samplingRate").toInt());
    }
    if(msg_in.contains("modeMDM")){
       modem_obj.insert("modeMDM", msg_in.value("modeMDM").toInt());
    }
    if (msg_in.contains("frequencyList")){
       modem_obj.insert("frequencyList",msg_in.value("frequencyList").toArray());
    }
    if (msg_in.contains("frequencyPrimList")){
       modem_obj.insert("frequencyPrimList",msg_in.value("frequencyPrimList").toArray());
    }
    if (msg_in.contains("frequencyAddList")){
       modem_obj.insert("frequencyAddList",msg_in.value("frequencyAddList").toArray());
    }
    if (msg_in.contains("ppfIndex")){
       modem_obj.insert("ppfIndex", msg_in.value("ppfIndex").toArray());
       radio_obj.insert("ppfIndex", msg_in.value("ppfIndex").toArray());
    }
    if(msg_in.contains("ppfNumber")){
       modem_obj.insert("ppfNumber", msg_in.value("ppfNumber").toInt());
    }
   //-------------radio-----
    if(msg_in.contains("radioDirName")){
       radio_role = msg_in.value("radioDirName").toString();
       radio_obj.insert("radioDirName", msg_in.value("radioDirName").toString());
       modem_obj.insert("radioDirName", msg_in.value("radioDirName").toString());
    }
    if (msg_in.contains("udpPort")){
        radio_obj.insert("udpPort", msg_in.value("udpPort").toInt());
    }

    if (msg_in.contains("inputPort")){
      QJsonArray objArr;
      objArr.append(msg_in.value("inputPort").toArray().at(0).toInt());
      radio_obj.insert("inputPort",objArr);
    }
    if(msg_in.contains("inputProt")){
       radio_obj.insert("inputProt", msg_in.value("inputProt").toString());
    }
    if(msg_in.contains("outputAddr")){
       radio_obj.insert("outputAddr", msg_in.value("outputAddr").toString());
    }
    if(msg_in.contains("outputPort")){
       radio_obj.insert("outputPort", msg_in.value("outputPort").toInt());
    }
    if(msg_in.contains("outputProt")){
       radio_obj.insert("outputProt", msg_in.value("outputProt").toString());
    }

    if(msg_in.contains("ppfSwitch")){
       radio_obj.insert("ppfSwitch", msg_in.value("ppfSwitch").toInt());
    }
    if(msg_in.contains("rcvrAddr")){
       radio_obj.insert("rcvrAddr", msg_in.value("rcvrAddr").toString());
    }
    if(msg_in.contains("rcvrPort")){
       radio_obj.insert("rcvrPort", msg_in.value("rcvrPort").toInt());
    }
    if(msg_in.contains("rcvrProt")){
       radio_obj.insert("rcvrProt", msg_in.value("rcvrProt").toString());
    }
    // ключ radioParams -временно искл.  из состава команды
   /*
    * QString radio_role{""};
      QString radio_ipv4{""};
      int     radio_udp1{0};
      int     dce_id{0};
      QString id{""};
      QJsonArray radio_array;
      QJsonObject radio_obj;

      if(msg_obj.contains("radioParams")){
          radio_array = msg_obj["radioParams"].toArray();   // массив объектов

          for(int i =0; i < radio_array.count(); ++i){    // обработка всех элементов массива
              radio_obj = radio_array.at(i).toObject();
              if (radio_obj.contains("radioName")){
                  radio_role = radio_obj.value("radioName").toString();
               }
              if (radio_obj.contains("radioIPv4")){
                  radio_ipv4 = radio_obj.value("radioIPv4").toString();
               }
              if (radio_obj.contains("radioUdpPort1")){
                  radio_udp1 = radio_obj.value("radioUdpPort1").toInt();
               }
          }
      }
      */

}
//-----------------------------------------------------------------------
// установка текущих параметров увс
void  Widget_DCE:: getCurrentSetting(QJsonObject & params_obj)
{
  /*  struct UVSParamS{
        quint32            DCE_ID{0};         // позывной модема
        quint32            DCE_ID_DST{0};     // позывной получателя (оппонента)
        quint64            create_time{0};    // время создания
        quint64            enable_time{0};    // время включения
        PACStatusEnum      stage{INIT};       // состояние модема на текущий момент (INIT = 0, ENABLE,  NET_SYNC, NET_CONNECT, EXCHENGE, NET_DISCON)
        int                msg_id{10};        // номер принятой команды ExchangeDCE
        QString            radio_role{"RX"};  // роль в радио передаче
        int                busy{0};           // занят -1, не занят - 0
        int                s_n{0};            // отношение сигнал/шум
        int                err_RS{0};         // кол-во ошибок Рида-Салам.
        QString            status{""};        // статус состояния (start,enqueued, recv, end, )
        QVector<quint32>   internalTime;     // массив внутр. времени модема
        int                freqRX{0};        // рекомендуемая частота приема
        int                freqTX{0};        // рекомендуемая частота передачи
        int                msgsInQue{0};      // сообщ. в очереди на передачу
        int                bytesInQue{0};    // байт в очер. на передачу
        QString            type{"raw"};      // тип передачи
       int                samplRate{0};     // частота дискритизации
       int                modeMDM{0};       // вариант режима работы

    };
    */
    if(params_obj.contains("DCE_ID")&& params_obj.contains("modeMDM") &&params_obj.contains("samplingRate")){
       m_uvs_obj.DCE_ID = params_obj.value("DCE_ID").toInt();
       m_uvs_obj.modeMDM = params_obj.value("modeMDM").toInt();
       m_uvs_obj.samplRate = params_obj.value("samplingRate").toInt();
    }

    QString radioRole{""};

    if(params_obj.contains("radioDirName")){ // определим радио роль в сети
        QString paramDir = params_obj.value("radioDirName").toString();
        QStringList list = paramDir.split(':');
        for (int i = 0; i <list.count(); ++i){// выделение данных роли в сети RX-ПРМ,  TX-ПРД
            if((list.at(i)=="TX")||(list.at(i)=="RX")){
                radioRole = list.at(i);
            }
        }
        if (radioRole=="TX"){  // работаем как ПРД
           m_uvs_obj.radio_role = "TX";
        }
        else if (radioRole=="RX"){  // работаем как ПРM
           m_uvs_obj.radio_role = "RX";

        //пока не использованные поля команды:
        /*  только на стороне ПРМ (управление раб. частотой приёмника
         * params_obj.value("rcvrAddr").toString();
         * params_obj.value("rcvrPort").toInt()
         * params_obj.value("rcvrProt").toString();
         */
        }
    }
}
//-----------------------------------------------------------------------
// передача в модем параметров работы и запуск модема
bool Widget_DCE:: turnOnModem(QJsonObject & params_obj)
{
    Q_UNUSED(params_obj);

    //if(params_obj.contains("DCE_ID")&& params_obj.contains("modeMDM") &&params_obj.contains("samplingRate")){
       ui->widget_modem->set_turnOn_State(true); // вкл. запуск таймера
       m_uvs_obj.DCE_ID = m_uvs_obj.DCE_ID;
       ui->lb_title->setText("ПАК УВС # "+ QString::number(m_uvs_obj.DCE_ID));
       ui->sb_id->setValue(m_uvs_obj.DCE_ID);   // на 1й вкладке настройки
       ui->widget_modem->setModemNumber(m_uvs_obj.DCE_ID);
       ui->widget_modem->setChannelLevel(true);
       ui->widget_modem->setSamplingRate(m_uvs_obj.modeMDM-1, m_uvs_obj.samplRate);
       ui->widget_modem->setPanelVariant(m_uvs_obj.modeMDM-1);     // установка 5 режима (режим должен меняться по команде EnableDCE)
  //  }
    // другие параметры пока в модем не передаются
    // если требуются другие параметры для передачи они находятся в  params_obj

    return ui->widget_modem->isActive;  // возвращаем запустился ли модем или нет
}
//-----------------------------------------------------------------------
// подключение к радио средствам параметров
void Widget_DCE::turnOnRadio(QJsonObject & params_obj)
{
    QString radioRole = m_uvs_obj.radio_role;  // определяем из пришедших параметров радио роль объекта

    if (radioRole=="TX"){  // работаем как ПРД          
           NetS corr_adr;    // адрес, портк корресп. в сети (радио передатчик)
           corr_adr.address = params_obj.value("outputAddr").toString();
           corr_adr.port = params_obj.value("outputPort").toInt();
           int port_bind = NO_USE_PORT;    // незначащий порт
           int start_buf = 0;              // стартовая буф. отключена по умолчанию
           ui->widget_IQ->setParams(corr_adr, port_bind, params_obj.value("ppfIndex").toArray().at(0).toInt(),start_buf);
           ui->widget_IQ->setTXInterface();
           m_uvs_obj.radio_role = "TX";
    }
    else if (radioRole=="RX"){  // работаем как ПРM         
           NetS corr_adr;    // адрес, порт корресп. в сети (радио передатчик)
           corr_adr.address = NO_USE_ADR;
           corr_adr.port = NO_USE_PORT;
           int port_bind = params_obj.value("inputPort").toArray().at(0).toInt(); //
           int start_buf = 0;            // стартовая буф. отключена по умолчанию
           ui->widget_IQ->setParams(corr_adr, port_bind, params_obj.value("ppfIndex").toArray().at(0).toInt(),start_buf);
           ui->widget_IQ->setRXInterface();
           m_uvs_obj.radio_role = "RX";

        //пока не использованные поля команды:
        /*  только на стороне ПРМ (управление раб. частотой приёмника
         * params_obj.value("rcvrAddr").toString();
         * params_obj.value("rcvrPort").toInt()
         * params_obj.value("rcvrProt").toString();
         */
        }
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdDisableDCEProc(MsgDescrS & msg)
{
    Q_UNUSED(msg);
    ui->widget_modem->set_turnOn_State(false); // вкл. запуск таймера
    m_uvs_obj.DCE_ID = 0;
    ui->lb_title->setText("ПАК УВС # "+ QString::number(m_uvs_obj.DCE_ID));
    ui->sb_id->setValue(m_uvs_obj.DCE_ID);   // на 1й вкладке настройки
    ui->widget_modem->setModemNumber(m_uvs_obj.DCE_ID);
    ui->widget_modem->setChannelLevel(false);

    ui->widget_IQ->setDisconnect();
   // ui->widget_modem->setSamplingRate(m_uvs_obj.modeMDM-1, m_uvs_obj.samplRate);
  //  ui->widget_modem->setPanelVariant(m_uvs_obj.modeMDM-1);
    //emit sigDisableDCE(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdNetworkSYNCProc(MsgDescrS & msg)
{
    emit sigSYNC(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdNetworkCONNECTProc(MsgDescrS & msg)
{
    emit sigCONECT(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdNetworkDISCONProc(MsgDescrS & msg)
{
    emit sigDISCON(msg);
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdExchangeDCEProc(MsgDescrS & msg)
{
    emit sigExchDCE(msg);   // пока связан со slotTempExchDCE(MsgDescrS & msg)
}
//=======================================================================
//   подготовка и отправка клиенту ответ в ProtocolServer
//=======================================================================
void Widget_DCE::cmdVerionAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;
    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//----------------------------------------------------------------------
void Widget_DCE::cmdStatusAnsw(MsgDescrS & msg)

{
    QJsonObject obj_param;
    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID).toUtf8().toInt());
    obj_param.insert("create_time", QString::number(m_uvs_obj.create_time).toUtf8().toLongLong() );
    obj_param.insert("enable_time", QString::number(m_uvs_obj.enable_time).toUtf8().toLongLong() );
    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdGetDCEInfoAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;

    QString stage{""};
    switch (m_uvs_obj.stage){
         case INIT : stage = "init";
               break;
         case ENABLE : stage = "enable";
               break;
         case NET_SYNC :stage = "sync";
               break;
         case NET_CONNECT :stage = "connect";
               break;
         case NET_DISC :stage = "discon";
              break;
         case EXCH :stage = "exchange";
              break;
         default : stage = "init";
              break;
     }

    QString busy{""};
    switch (m_uvs_obj.busy){
         case 0 : busy = "no";
               break;
         case 1 : busy = "yes";
               break;
         default : busy = "yes";
              break;
    }

    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID).toUtf8().toInt());
    obj_param.insert("stage", stage );
    obj_param.insert("busy", busy );
    obj_param.insert("create_time", QString::number(m_uvs_obj.create_time).toUtf8().toLongLong() );
    obj_param.insert("enable_time", QString::number(m_uvs_obj.enable_time).toUtf8().toLongLong() );

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdGetRadioDirInfoAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;
    int sn = 1;
    int errRS = 1;
    m_uvs_obj.DCE_ID_DST = 2;

    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID).toUtf8().toInt());
    obj_param.insert("DCE_ID_OPPN", QString::number(m_uvs_obj.DCE_ID_DST).toUtf8().toInt() );
    obj_param.insert("s_n", QString::number(sn).toUtf8().toInt() );
    obj_param.insert("err_RS", QString::number(errRS).toUtf8().toInt() );

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdEnableDCEAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;
    bool turn_on = true;
    obj_param.insert("turn_on", turn_on );

    m_uvs_obj.enable_time = QDateTime::currentDateTime().currentMSecsSinceEpoch();
    m_uvs_obj.stage = ENABLE;
    m_uvs_obj.DCE_ID = msg.obj_msg_in.value("DCE_ID").toDouble();

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdDisableDCEAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;
    bool turn_on = false;
    obj_param.insert("turn_on", turn_on );

    m_uvs_obj.enable_time = 0;
    m_uvs_obj.stage = INIT;
    m_uvs_obj.DCE_ID = 0xffff;;

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdNetworkSYNCAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;
    QJsonArray objArr;

    for(int i = 0; i < m_uvs_obj.internalTime.count(); ++i)
        objArr.append((int)m_uvs_obj.internalTime.at(i));

    m_uvs_obj.DCE_ID_DST = msg.obj_msg_in.value("DCE_ID_OPPN").toInt();

    obj_param.insert("id", msg.obj_msg_in.value("id").toString());
    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID));
    obj_param.insert("DCE_ID_OPPN", QString::number(m_uvs_obj.DCE_ID_DST) );
    obj_param.insert("status", m_uvs_obj.status);
    obj_param.insert("internalTime", objArr);

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdNetworkCONNECTAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;
    obj_param.insert("id", msg.obj_msg_in.value("id").toString());
    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID));
    obj_param.insert("DCE_ID_OPPN",QString::number(m_uvs_obj.DCE_ID_DST));
    obj_param.insert("freqRX", QString::number(m_uvs_obj.freqRX));
    obj_param.insert("freqTX", QString::number(m_uvs_obj.freqTX));
    obj_param.insert("status", m_uvs_obj.status);

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::cmdNetworkDISCONAnsw(MsgDescrS & msg)
{
    QJsonObject obj_param;

    obj_param.insert("id", msg.obj_msg_in.value("id").toString());
    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID));
    obj_param.insert("DCE_ID_OPPN", QString::number(m_uvs_obj.DCE_ID_DST));
    obj_param.insert("status",  m_uvs_obj.status);

    emit sigCmdToClient(msg, obj_param);  // сигнал в ProtocolServ
}
//-----------------------------------------------------------------------
void Widget_DCE::slotCmdExchangeDCEAnsw(QByteArray data_exch)
{
    ++m_uvs_obj.msg_id;
    m_uvs_obj.type = "Raw";

    QJsonObject obj_param;
    QString str{data_exch.toBase64()};
    QJsonValue data_val(str);

    qDebug()<<"Demodul_mess: "<< str;

    obj_param.insert("data", data_val);
    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID).toInt());
    obj_param.insert("DCE_ID_DST", QString::number(m_uvs_obj.DCE_ID_DST).toInt());
    obj_param.insert("MSG_ID", m_uvs_obj.msg_id);
    obj_param.insert("type", m_uvs_obj.type);

    emit sigCmdToClientFromRadio("ExchangeDCE", obj_param);

 }
//-----------------------------------------------------------------------
void Widget_DCE::cmdReplyDCEAnsw(MsgDescrS & msg)  // status:verify
{
    QJsonObject obj_param;

    obj_param.insert("DCE_ID", QString::number(m_uvs_obj.DCE_ID).toInt());
    obj_param.insert("DCE_ID_DST", QString::number(m_uvs_obj.DCE_ID_DST).toInt());
    obj_param.insert("status", m_uvs_obj.status);
    obj_param.insert("MSG_ID", m_uvs_obj.msg_id);
    obj_param.insert("msgsInQueue",  m_uvs_obj.msgsInQue);
    obj_param.insert("bytesInQueue", m_uvs_obj.bytesInQue);
    obj_param.insert("type", m_uvs_obj.type);

    emit sigCmdToClient(msg, obj_param); // сигнал в ProtocolServ
 }
//---------------------------------------------------------------------------
// длина сообщения в разкодированном виде
qint32 Widget_DCE::getMsgSize(QString &data_field)
{
    quint32 sz{0};
    QByteArray ba, ba_;
    ba.append(data_field.toUtf8());
    ba_.append(QByteArray::fromBase64(ba.data())); // декодируем сообщение
    sz = ba_.count();                               // подсчитываем кол-во байт в нем

    return sz;
}
//-----------------------------------------------------------------------
// подготовка ответа клиенту со status::enqueued
void Widget_DCE::slotPrepareEnqueuedAnsw(MsgDescrS & msg, int msgInQue)
{
    QString data_str{msg.obj_msg_in.value("data").toString()};
    m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue + getMsgSize(data_str);

    m_uvs_obj.status = "enqueued";
    m_uvs_obj.DCE_ID = msg.obj_msg_in.value("DCE_ID").toInt();
    m_uvs_obj.DCE_ID_DST = msg.obj_msg_in.value("DCE_ID_DST").toInt();
    m_uvs_obj.msg_id = msg.obj_msg_in.value("MSG_ID").toInt();
    m_uvs_obj.msgsInQue = msgInQue;//m_queue_msg.count();//1;

 //   QString data_str{msg.obj_msg_in.value("data").toString()};
  //  m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue + getMsgSize(data_str);
    m_uvs_obj.type = msg.obj_msg_in.value("type").toString();
    cmdReplyDCEAnsw(msg);
}
//-------------------------------------------------------------------------
int Widget_DCE::decodingDataLength(QJsonValue val)
{
    int len;
    QByteArray bb, ba;

    bb.append(val.toString().toUtf8());       // зашифрованные данные в байтах помещаем в байтовый массив
    ba.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
    len = ba.length();
    return len;
}
//-------------------------------------------------------------------------
int Widget_DCE::decodingGettingObject(QJsonObject json_in)
{
    QJsonValue objVal;
    int len{0};
    if(json_in.contains("data")){
      objVal = json_in.value("data");
      len = decodingDataLength(objVal);
     }
    return len;
}
//-----------------------------------------------------------------------
// подготовка ответа клиенту со status::start
void Widget_DCE::slotPrepareStartAnsw(MsgDescrS & msg)
{
    QString data_str{msg.obj_msg_in.value("data").toString()};
    m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue - getMsgSize(data_str) ;

    int slot_len = ui->widget_modem->getSlotMsgLen();
    int data_len = decodingGettingObject(msg.obj_msg_in);
    int dif{0};

    (slot_len > data_len) ? (dif = data_len) : (dif = slot_len);
    m_uvs_obj.status = "start";
    m_uvs_obj.DCE_ID = msg.obj_msg_in.value("DCE_ID").toInt();
    m_uvs_obj.DCE_ID_DST = msg.obj_msg_in.value("DCE_ID_DST").toInt();
    m_uvs_obj.msg_id = msg.obj_msg_in.value("MSG_ID").toInt();
   // m_uvs_obj.msgsInQue = m_queue_msg.count();//1;
   // m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue - dif;
    m_uvs_obj.type = msg.obj_msg_in.value("type").toString();
    cmdReplyDCEAnsw(msg);
}
//-----------------------------------------------------------------------
 // подготовка ответа клиенту со status::end
void Widget_DCE::slotPrepareEndAnsw(MsgDescrS & msg, int msgInQue)
{
    int slot_len = ui->widget_modem->getSlotMsgLen();
    int data_len = decodingGettingObject(msg.obj_msg_in);
    int dif{0};

    (slot_len > data_len) ? (dif = data_len) : (dif = slot_len);

    m_uvs_obj.status = "end";
    m_uvs_obj.DCE_ID = msg.obj_msg_in.value("DCE_ID").toInt();
    m_uvs_obj.DCE_ID_DST = msg.obj_msg_in.value("DCE_ID_DST").toInt();
    m_uvs_obj.msg_id = msg.obj_msg_in.value("MSG_ID").toInt();
    m_uvs_obj.msgsInQue  = msgInQue;//m_queue_msg.count();//0;
  //  QString data_str{msg.obj_msg_in.value("data").toString()};
  //  m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue + dif;
 //   m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue - getMsgSize(data_str) ;
    m_uvs_obj.type = msg.obj_msg_in.value("type").toString();
    cmdReplyDCEAnsw(msg);
}
//-------------------------------------------------------------------
 void Widget_DCE::slotIQSigToRadio(SlotFieldS &current_slot/*short_complex * signal, int len, int freq_num*/)                     // передача данных в radio канал
 {
     emit sigIQToRadio(current_slot);
 }
//--------------------------------------------------------------------
void Widget_DCE::slotGetCommandFromProt(MsgDescrS &msg)
{
    // отправляем в мир команду принятую модулем PROTOCOL_DCE от клиента
    emit sigGetCommd(msg);
}
//--------------------------------------------------------------------
void Widget_DCE::slotCommdReReady(MsgDescrS &msg)
{
    // возвращаем дозаполненную команду ответ в протокол
    emit sigCommReToProt(msg);
}

//-------------------------------------------------------------------------------------------------
void Widget_DCE::slotGetNetAdrFromConnect(NetS & net_adr)
{
    emit sigNetAdr(net_adr);
}
//-------------------------------------------------------------------------------------------------
void Widget_DCE::slotSendErrFromConnect( QString & str, QTime tm)
{
    Q_UNUSED(tm);
   // произошла ошибка запуске сервера
   if(str=="error"){
       // соединение не установлено

       ui->pb_net->setText("Старт Сервер");

       disconnect (this, &Widget_DCE::sigNetAdr, m_prtcl_serv, &ProtocolServ::slotSetNetAdr);
       disconnect(this, &Widget_DCE::sigUseDST, m_prtcl_serv, &ProtocolServ::slotUseDST);
      // disconnect (m_prtcl_serv, &ProtocolServ::sigCmdFromClient, ui->widget_modem, &CoreDCE::slotCmdFromClient); // команда от клиента
       //disconnect(ui->widget_modem, &CoreDCE::sigCmdToClient, m_prtcl_serv, &ProtocolServ::slotCmdToClient);
      // disconnect(ui->widget_modem, &CoreDCE::sigCmdToClientFromRadio, m_prtcl_serv, &ProtocolServ::slotCmdToClientFromChanel);
       disconnect (ui->widget_modem, &CoreDCE::sigIQSignalReady, this, &Widget_DCE::slotIQSigToRadio);
       disconnect(this, &Widget_DCE::sigStopServ, m_prtcl_serv, &ProtocolServ::sigDisconnect);
       disconnect(this, &Widget_DCE::destroyed, m_prtcl_serv, &ProtocolServ::sigDisconnect);
       disconnect(m_prtcl_serv, &ProtocolServ::sigAddMsgToJournal, this, &Widget_DCE::slotSendPackFromTransport);
       disconnect(m_prtcl_serv, &ProtocolServ::sigUpdateClientList, this, &Widget_DCE::slotSendListClientFromConnect);
       disconnect(m_prtcl_serv, &ProtocolServ::sigChangeWorkStatus, this, &Widget_DCE::slotSendErrFromConnect);
       disconnect(this, &Widget_DCE::sigErrorTurnOn, m_prtcl_serv, &ProtocolServ::slotErrorFromTurnOnModemRadio);
       disconnect(m_prtcl_serv, &ProtocolServ::sigCmdFromClient, this, &Widget_DCE::slotCmdFromClient); // команда от клиента


       clearClientPanel();
       delete  m_prtcl_serv;
       m_prtcl_serv = nullptr;
   }
   // сервер запустился нормально
   else if(str =="ok"){
       // соединение установлено
       ui->pb_net->setText("Стоп Сервер");
       ui->rb_json->setEnabled(false);
       ui->rb_xml->setEnabled(false);
   }
}
//------------------------------------------------------------------
// добавление на панель подключившегося клиента
void Widget_DCE::addClientSign(QString client, int port, QString adr)
{

    Q_INIT_RESOURCE(pict);
    QString name{":/icon/pics/mas_blue.png"};

   // QFile file (name);
    QPixmap sign;

    if(client == "MAS") {
        sign = QPixmap (":/icon/pics/mas_blue.png");
    }
    else if (client == "AU"){
         sign = QPixmap (":/icon/pics/au_blue.png");
    }

    else if (client == "UVS"){
        sign = QPixmap (":/icon/pics/uvs_blue.png");
    }
    else if (client == "UNKN"){
        sign = QPixmap (name);
    }

     DTE_label * client_label = new DTE_label(QString::number(port), sign, adr,this);
     ui->grB_clients->layout()->addWidget(client_label);
}
//-----------------------------------------------------------------
// очистка панели с подключенными клиентами для обновления списка подключенных
void Widget_DCE::clearClientPanel()
{
    while (QLayoutItem *item = ui->grB_clients->layout()->takeAt(0)){
        delete item->widget();
        delete item;
   }
}
//-----------------------------------------------------------------
void Widget_DCE::slotSendListClientFromConnect(QVector <NetConnectS> client_vec)
{
    clearClientPanel();

    for(int i = 0; i < client_vec.count(); ++i){
      addClientSign(client_vec.at(i).client_type, client_vec.at(i).port, client_vec.at(i).address);
    }
}
//------------------------------------------------------------------
// функция формирования заголовка для отображения в информационном окне приложения
// tx -флаг приема или передачи = 1;   // передача;  = 0;   // прием

QString Widget_DCE::titleAndColorCreate(QColor &cl, Qt::Alignment &al, int tx)
{
    QString title("");
    cl = Qt::black;

    if(!tx){
        title = "";//"Сообщение от ПАК МАС/ ПАК АУ:";
        cl = Qt::blue;
        al = Qt::AlignLeft;
    }
    else if(tx){
       title = "";//"Сообщение ПАК МАС/ ПАК АУ:";
       cl = Qt::red;
       al = Qt::AlignRight;
    }

    return  title;
}
//------------------------------------------------------------------
// разбор сообщения JSON и выделение из сообщения названия команды
QString Widget_DCE::jsonCmd(const QString & str_json)
{
    QString str = str_json;
    int comPos {-1};
    int statusPos {-1};
    QString com {"???"};
    QString status {""};
    QStringList list = str.split(",");

    for (int i = 0 ; i < list.count(); ++i){
        comPos = list.at(i).indexOf("cmd");
        statusPos = list.at(i).indexOf("status\":");
        if (comPos > 0){
            com.clear();
             QString test =list.at(i);
             test.remove("\"");
             test.remove(" ");
             test.remove("}");
             int pos = test.indexOf(":");
             com = test.right(test.size()-pos-1);
        }
        if (statusPos > 0){
            QString test =list.at(i);
            test.remove("\"");
            test.remove(" ");
            test.remove("}");
            int pos = test.indexOf(":");
            status = test.right(test.size()-pos-1);
            com = com +" : "+status;
        }
    }
     return "  "+com;
}
//------------------------------------------------------------------
QString Widget_DCE::xmlCmd(const QString & str_xml)
{
    QString str = str_xml;
    QString type = "<cmd>";
    QString status = "<status>";
    int comPos = str.indexOf("<cmd>") + type.size();
    int statusPos = str.indexOf("<status>") + status.size();
    int comEndPos = str.indexOf("</cmd>", comPos);
    int statusEndPos = str.indexOf("</status>", statusPos);
    QString com{"???"};
    if (comPos > 0){
        com.clear();
        com = str.mid(comPos, comEndPos - comPos);
    }
    if(statusPos > 7)
       com = com +" : "+str.mid(statusPos, statusEndPos - statusPos);
    return "  "+com;
}
//------------------------------------------------------------------
 QString Widget_DCE::getTypeCom(const QString & str)
 {
    QString cmd ="";
     if(m_type_msg == JSON){
         cmd = jsonCmd(str);
     }
     else if (m_type_msg == XML){
         cmd = xmlCmd(str);
     }
     return cmd;
 }
 //------------------------------------------------------------------
 void Widget_DCE::slotSendPackFromTransport(int tx, QByteArray & ba, QDateTime tm)
 {
   // отображаем в окне журнала принятый / отправляемый пакет.
    QColor cl = Qt::black;
    Qt::Alignment al;
    QString mess_title = titleAndColorCreate(cl, al, tx);// цвет текущего сообщения задается внутри ф-ции

    QString type_com = getTypeCom(QString(ba));
    emit sigWriteInJourn(tx, tm, type_com, QString(ba), cl);   // отображаем запись в журнале
 }
 //-------------------------------------------------------------------
 //--  Настройки внешнего вида WidgetDCE ----------------------------
 //-------------------------------------------------------------------
 void Widget_DCE::initWidget()
 {

     // изолируем элементы управления работы с сервером при запуске сервера командой EnableDCE
     ui->pb_net->setEnabled(false);
     ui->sb_id->setEnabled(false);
     ui->sb_id_dst->setEnabled(false);


     m_text_cl = Qt::red;

    ui->lb_title->setText("ПАК УВС # ...");

    QPalette pl;

    pl.setBrush(QPalette::Active,QPalette::WindowText, QColor(m_text_cl));
    pl.setBrush(QPalette::Inactive,QPalette::WindowText, QColor(m_text_cl));
    pl.setBrush(QPalette::Disabled,QPalette::WindowText, QColor(m_text_cl));

    pl.setBrush(QPalette::Disabled,QPalette::Text, QColor(m_text_cl));//Qt::darkGray));
    pl.setBrush(QPalette::Active,QPalette::Text, QColor(m_text_cl));//Qt::darkGray));
    pl.setBrush(QPalette::Inactive,QPalette::Text, QColor(m_text_cl));//Qt::darkGray));

    pl.setBrush(QPalette::Active, QPalette::ButtonText, QColor(m_text_cl));
    pl.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(m_text_cl));
    pl.setBrush(QPalette::Inactive, QPalette::ButtonText, QColor(m_text_cl));

    this->setPalette(pl);
    this->setPalette(pl);
    ui->lb_title->setPalette(pl);
    ui->chB_format->setChecked(false);

    // начало работы
    ui->pb_net->setText("Старт Сервер");    
 }

 //------------------------------------------------------------------
 // функция формирования заголовка для отображения в информационном окне приложения

 QString Widget_DCE::createTitle(QColor &cl, Qt::Alignment &al,int tx)
 {
     QString title("");
     cl = Qt::black;

     if(!tx){
         title = "Сообщение от ПАК УВС:";
          cl= Qt::red;
          al = Qt::AlignRight;
     }
     else if(tx){
        title = "Сообщение ПАК УВС:";
         cl= Qt::blue;
         al = Qt::AlignLeft;
     }
     return  title;
 }
 //-------------------------------------------------------------------
 // установка скорости передачи
/* void    Widget_DCE::setRatePass(int rate)
 {
  //   if(m_prtcl_serv!=nullptr)
   //      m_prtcl_serv->setRatePass(rate);
 } 
 */
//--------------------------------------------------------------------
 void Widget_DCE::slotGetIQFromRadio(QVector<int_complex> * signal)
 {
     ui->widget_modem->slotIQFromCorr(signal);
  //   if(m_prtcl_serv!=nullptr){
   //      m_prtcl_serv->сodoFromExch(obj_in);
   //  }
 }
 //--------------------------------------------------------------------
 void Widget_DCE::slotCodoToRadio(QJsonObject & obj)
 {
     emit sigCodoToRadio(obj);
 }
//----------------------------------------------------------------------
 void Widget_DCE::slotDisconRadio()
 {
    // if(m_prtcl_serv!=nullptr){
    //     m_prtcl_serv->disconnectExch();
  //   }
 }
 //---------------------------------------------------------------------

void Widget_DCE::slotLogFileListChanged()
{
  /*  for (int i = 0; i < ui->lw_logs->count(); ++i){
        ui->lw_logs->item(i)->checkState();
        ui->lw_logs->item(i)->
    }
    */
}

//-----------------------------------------------------------------------
// процедура рекурсивной обработки json объектов()
void Widget_DCE::processObject(QJsonObject & obj_in,   // объект принятый, сформированный из входящей последовательности
                            QJsonObject & obj_samp,  // объект эталонный, сформированный по данным m_com_list т.е.из файла command_list
                            ModeEnum mode, QString &err)           // режим обработки (CHECK - проверяется входящий объект на соответствие с эталонным,
                                                         //                  FILL_SAME - после того как входной объект проверен, по некоторым
                                                       //его данным + эталонный объект ->формируется формирутся объект отвертной команды)
{
   if((!obj_in.isEmpty()) && (!obj_samp.isEmpty()) ){    // если переданные объекты не пустые
      for (int i = 0; i < obj_samp.count(); ++i){
          QString key_samp = obj_samp.keys().at(i);           // очередной ключ в объекте эталоне
          QJsonValue val_in = obj_in.value(key_samp);         // значение того же ключа в входном объекте
          QJsonValue val_samp = obj_samp.value(key_samp);     // значение ключа в эталонном пакете
          bool s, d, a, o, sd, ds = false;                    // флаги для определения типа значения из данного ключа

          double val_in_d {0};
          QString val_in_s{""};

                               // определяем считанные значения из входного объекта, и эталонного объекта
          s = val_in.isString() && val_samp.isString();
          d = val_in.isDouble() && val_samp.isDouble();
          sd = val_in.isDouble() && val_samp.isString();
          ds = val_in.isString() && val_samp.isDouble();
          a = val_in.isArray() && val_samp.isArray();
          o = val_in.isObject() && val_samp.isObject();

         if(obj_in.contains(key_samp) && (a|d|s|o|ds|sd)){ // если есть совпадение названия полей и их типов

             if(d|s){                                // если совпадение по типу строки или числа
                obj_samp.insert(key_samp, val_in);   // пришедшее значение записываем в эталонный объект
             }   // обработка строки ли числа

             else if (sd){
                 val_in_s = val_in.toString();
                 obj_in.insert(key_samp, val_in_s);
             }
             else if (ds){
                 val_in_d = val_in.toDouble();
                 obj_in.insert(key_samp, val_in_d);
             }
             else if (a){  // если совпадение по типу массив
                              // проверяем поля и значения массива
                 QJsonArray ja_in = val_in.toArray();
                 QJsonArray ja_samp = val_samp.toArray();

                 for (int j = 0; j < ja_in.count(); ++j){
                     if(ja_in.at(j).isObject()){       // если значения массива - объекты =>
                       QJsonObject subTr_in = ja_in.at(j).toObject();
                       QJsonObject subTr_samp = ja_samp.at(j).toObject();
                       processObject(subTr_in, subTr_samp, mode, err); // рекурсивно вызываем эту же функцию
                                                                  //передава ей элемены массивов для дальнейщего обхода
                     }
                     else if (ja_in.at(j).isDouble() || ja_in.at(j).isString() ||ja_in.at(j).isBool()){
                             obj_samp.insert(key_samp, val_in);   // пришедшее значение записываем в эталонный объект
                             break;   // выходим из цикла j предполагая что все элемнты массива одного типа , не объекты
                     }
                 }
             }   // обработка массива

             else if (o){   // если совпадение по типу объект
                // проверяем поля и значения объекта
                QJsonObject jo_in = val_in.toObject();
                QJsonObject jo_sampl = val_samp.toObject();
                processObject(jo_in, jo_sampl, mode, err);      // рекурсивно вызываем эту же функцию
            } // обработка объекта
        }  // поля в эталоне и входящем объекте - имеются и совпадают по типу

        else {  // если в пришедшей последовательности нужного поля нет или тип не совпадает
            if(mode == CHECK){  // сообщение о несовпадении входного и эталонного объекта по ключам требуется только для режима проверки
              // QMessageBox::critical(0, "Сообщение","В сообщении JSON ошибка структуры команды: ("+key_samp+")" ,"");
               err = " Верификация. Структура команды: " + key_samp;
             }
           } // выдаем ошибку структуры JSON сообщения
       }   // обход эталонного объекта завершен
     }     // входной и эталонный объекты, переданные в функцию не пустые

   }
//-------------------------------------------------------------------------------------------------------
