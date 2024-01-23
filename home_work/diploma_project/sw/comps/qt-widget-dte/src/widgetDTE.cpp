#include "widgetDTE.h"
#include "ui_widgetDTE.h"
#include <QHostInfo>
#include <QUuid>
#include <QMessageBox>
#include <QJsonObject>
#include "widgetClient.h"
#include "widgetAU.h"
#include "widgetMAS.h"

//-------------------------------------------------------------------------------------------------
WidgetDTE::WidgetDTE(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetDTE)
{
    ui->setupUi(this);
    qRegisterMetaType<NetConnectS>();   
    qRegisterMetaType<ExchangeModeEnum>();

   // m_wgt_cl = new WidgetClient(this);
  //  ui->verticalLayout_5->addWidget(m_wgt_cl);
    //m_wgt_cl->setFixedHeight(0);
   // m_wgt_cl->setVisible(false);

    m_msg_timer= new QTimer(this);
    m_msg_timer->setTimerType(Qt::PreciseTimer);
    connect(m_msg_timer, SIGNAL(timeout()),this, SLOT(slotTimerMsgAlarm()));

    QVBoxLayout * vbox = new QVBoxLayout(ui->groupBox_6);

  //  vbox->addWidget(&m_wgt_au);

    ui->groupBox_6->setLayout(vbox);
   // ui->verticalLayout_5->addWidget(m_wgt_au);
   // m_wgt_au->setFixedHeight(0);
  //  m_wgt_au.setVisible(true);

 //   m_wgt_mas = new WidgetMAS(this);
 //   ui->verticalLayout_5->addWidget(m_wgt_mas);
   // m_wgt_mas->setFixedHeight(0);
  //  m_wgt_mas->setVisible(false);

    m_autimer = new QTimer();          // таймер для задания времени ожидания ответа на упр. команду ПАК АУ -> ПАК УВС
    //  m_info_pack_timer = new QTimer();   // таймер режима INFO_EXCH обмена инфо. сообщениями
    initMainWind();

    ui->widget_journ->initWidget();
    connect(ui->pb_stop_start, SIGNAL(clicked()),this, SLOT(slotStopTest()));
    //connect(m_info_pack_timer, SIGNAL(timeout()),this, SLOT(slotTimerInfoPackAlarm()));
    //connect(m_autimer, SIGNAL(timeout()),this, SLOT(slotTimerAuAlarm()));       // обработка таймера ожидания ответа на комнду упр. ПАК УВС
    connect (ui->pb_net,SIGNAL(clicked()), this, SLOT(slotNetWorking()));       // обработка "Установления соединения с ПАК УВС"
    connect (ui->pb_ping, SIGNAL(clicked()), this, SLOT(slotPing()));           // кнопка "PING"
    connect (ui->pb_version, SIGNAL(clicked()), this, SLOT(slotVersion()));     // кнопка "VERSION"
    connect (ui->pb_status, SIGNAL(clicked()), this, SLOT(slotStatus()));       // кнопка "STATUS"
    connect (ui->pb_enable, SIGNAL(clicked()), this, SLOT(slotEnable()));       // кнопка "ENABLE_DCE"
    connect (ui->pb_disable, SIGNAL(clicked()), this, SLOT(slotDisable()));     // кнопка "DISABLE_DCE"
    connect (ui->pb_net_sync, SIGNAL(clicked()), this, SLOT(slotNetSync()));    // кнопка "NETWORK_SYNC"
    connect (ui->pb_net_connect, SIGNAL(clicked()), this, SLOT(slotNetConnect()));  // кнопка "NETWORK_CONNECT"
    connect (ui->pb_net_disconnect, SIGNAL(clicked()), this, SLOT(slotNetDisconnect()));  // кнопка "NETWORK_DISCONNECT"
    connect (ui->pb_dceInfo, SIGNAL(clicked()), this, SLOT(slotDceInfo()));    // кнопка "GetDCEINfo"
    connect (ui->pb_dirInfo, SIGNAL(clicked()), this, SLOT(slotDirInfo()));     // кнопка "GetDirINfo"
    connect (ui->pb_Test, SIGNAL(clicked()), this, SLOT(slotTestCmd()));        // кнопка "ТестCmd"


  //  connect(ui->pb_disconnect, SIGNAL(clicked()), this, SLOT(disconnectSlot()));
    connect(this, &WidgetDTE::sigWriteInJourn, ui->widget_journ, &WidgetMess::slotAddMsg);
    connect (ui->rb_mas,SIGNAL(clicked()), this, SLOT(slotMas()));
    connect (ui->rb_au,SIGNAL(clicked()), this, SLOT(slotAu()));
    connect (ui->rb_json,SIGNAL(clicked()), this, SLOT(slotJson()));
    connect (ui->rb_xml,SIGNAL(clicked()), this, SLOT(slotXml()));
    connect (ui->lePort, SIGNAL(textChanged(QString)), this, SLOT(slotTitleChange(QString)));
    connect (ui->rb_info,SIGNAL(clicked()), this, SLOT(slotInfo()));
    connect (ui->rb_test,SIGNAL(clicked()), this, SLOT(slotTest()));
    connect(ui->chB_journal,SIGNAL(clicked(bool)), this, SLOT(slotJournalView(bool)));
    connect(ui->chB_varyable, SIGNAL(clicked(bool)), this, SLOT(slotVaryData(bool)));

   // if(ui->chB_journal->isChecked()){
      slotJournalView(ui->chB_journal->isChecked());
  //  }

      ui->pb_enable->setVisible(true);
      ui->pb_disable->setVisible(true);
      ui->pb_net_sync->setVisible(true);
      ui->pb_net_connect->setVisible(true);
      ui->pb_net_disconnect->setVisible(true);
      ui->groupBox_11->setVisible(false);
      //ui->rb_tcp->setEnabled(false);
      //ui->rb_udp->setEnabled(false);

      ui->pb_stop_start->setText("Инфообмен\n ||");
      ui->pb_stop_start->setEnabled(false);
      ui->cb_qDepth->setCurrentText("1");

      emit ui->chB_varyable->clicked(true);
    //  ui->chB_varyable->setChecked(true);     // в режиме теста передаваемая послед. будет различная

}
//----------------------------------------------------------------------------------------------------------
void WidgetDTE::slotVaryData(bool fl_vary)        // вариативность последовательности для передачи в ком. ExchangeDCE
{
    m_varyable = fl_vary;
}
//-----------------------------------------------------------------
void WidgetDTE::slotCloseApp()
{
  //  slotStopTest();
  //  slotDiscon();
}
//----------------------------------------------------------------------------------------------------------
void WidgetDTE::autoStartMAS()
{
    ui->rb_mas->setChecked(true);
    emit ui->rb_mas->clicked(true);
    emit ui->pb_net->clicked(true);
}
//----------------------------------------------------------------------------------------------------------
WidgetDTE::~WidgetDTE()
{     
    delete ui;
}
//-------------------------------------------------------------------------------------------------------------
void WidgetDTE::setPort(int port)
{
   ui->lePort->setText(QString::number(port));
  // autoStartMAS();    // автозапуск работы иммитатора в режиме ПАК МАС
}
//-------------------------------------------------------------------------------------------------------------
void WidgetDTE::setIP(QString ip)
{
   ui->leAdress->setText(ip);
}
//------------------------------------------------------------------
void WidgetDTE::slotJournalView(bool state)
{
    if(state){
      connect(this, &WidgetDTE::sigWriteInJourn, ui->widget_journ, &WidgetMess::slotAddMsg);
      ui->widget_journ->setVisible(true);
    }
    else{
      disconnect(this, &WidgetDTE::sigWriteInJourn, ui->widget_journ, &WidgetMess::slotAddMsg);
      ui->widget_journ->setVisible(false);
    }
}
//---------------------------------------------------------------------------
void WidgetDTE::slotInfo()
{
    int m;
    if (ui->rb_info->isChecked()){
        m_exch_mode = INFO_EXCH;
        m = 1;
        ui->le_err->setEnabled(false);
    }
    else{
        m_exch_mode = TEST_EXCH;
        m = 0;
        ui->le_err->setEnabled(true);
    }
    emit sigExchMode(m);//m_exch_mode);   // сигнал в ProtocolClient о смене режиа ПАК МАС

    if(ui->pb_stop_start->text()=="Инфообмен\n ||" && ui->pb_stop_start->isEnabled())
       slotStopTest();
    slotExchange();

   // slotStopTest();
   // slotStopTest();

}
//------------------------------------------------------------------
void WidgetDTE::slotTest()
{
    int m;
    if (ui->rb_test->isChecked()){
        m_exch_mode = TEST_EXCH;
        m = 0;
        ui->le_err->setEnabled(true);
    }
    else{
        m = 1;
        m_exch_mode = INFO_EXCH;
        ui->le_err->setEnabled(false);
    }
    emit sigExchMode(m);//m_exch_mode);// сигнал в ProtocolClient о смене режиа ПАК МАС
    if(ui->pb_stop_start->text()=="Инфообмен\n ||" && ui->pb_stop_start->isEnabled())
       slotStopTest();
    slotExchange();

   slotStopTest();
   slotStopTest();
}
//------------------------------------------------------------------------
// обработка выбора технологии JSON для обмена сообщениями с ПАК УВС
void WidgetDTE::slotJson()
{
    if(ui->rb_json->isChecked()){
       m_type_msg  = JSON;
    }
    else{
        m_type_msg  = XML;
    }
}
//------------------------------------------------------------------
void WidgetDTE::slotTitleChange(QString str)
{
    Q_UNUSED(str);
    // определение ip адреса машины на которой запущено приложение
    QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();
    ipAdr = "";    //listAddr.first().toString();

    for(int i = 0; i <listAddr.count(); ++i ){
        ipAdr = ipAdr+" * " + listAddr.at(i).toString()+" * ";
    }

   // WidgetDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК УВС  ip[ "+ ipAdr+" ]");
}
//------------------------------------------------------------------
// обработка выбора технологии XML для обмена сообщениями с ПАК УВС
void WidgetDTE::slotXml()
{
    if(ui->rb_xml->isChecked()){
       m_type_msg  = XML;
    }
    else{
        m_type_msg  = JSON;
    }
}
//-------------------------------------------------------------------
// обработка отключения клиента от сервера (ПАК УВС)
void WidgetDTE::slotDiscon()
{
    totalDisconnect(); // соединение разрывается
}
//------------------------------------------------------------------
// обработка выбора в роли клиента ПАК МАС
void WidgetDTE::slotMas()
{
    if(ui->rb_mas->isChecked()){
       m_client_type  = MAS;
       ui->lb_title->setText("ПАК МАС");
       QPixmap icon_m {":/icon/pics/m_blue.png"};
       QApplication::setWindowIcon( icon_m );
       setWindowTitle("  ПАК МАС  ");
      // MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК МАС");
       ui->pb_dceInfo->setVisible(false);
       ui->pb_dirInfo->setVisible(false);
       ui->pb_status->setVisible(false);
       ui->pb_enable->setVisible(false);
       ui->pb_disable->setVisible(false);
       ui->pb_net_sync->setVisible(false);
       ui->pb_net_connect->setVisible(false);
       ui->pb_net_disconnect->setVisible(false);
       ui->groupBox_10->setVisible(true);
       ui->groupBox_9->setVisible(false);       
       ui->groupBox_7->setVisible(false);
       ui->groupBox_11->setVisible(true);
      // ui->rb_tcp->setEnabled(true);
      // ui->rb_udp->setEnabled(true);
      // panelStateVisible(false);

      // WidgetMAS *wgt_mas = new WidgetMAS(this);
    //   if (m_wgt_cl!=nullptr)
    //       delete m_wgt_cl;

     //  m_wgt_cl = new WidgetMAS(this);//new WidgetClient(this);
    //   m_wgt_cl->setFixedHeight(500);
    //   m_wgt_cl->setFixedWidth(700);
       //m_wgt_cl = wgt_mas;       
      // ui->verticalLayout_5->addWidget(m_wgt_cl);
       //m_wgt_mas->setVisible(true);
      // m_wgt_au->setVisible(false);

    }
    else{
        m_client_type  = AU;
        ui->lb_title->setText("ПАК АУ АСРЦ");
        QPixmap icon_a {":/icon/pics/a_blue.png"};
        QApplication::setWindowIcon( icon_a );
        setWindowTitle("  ПАК АУ АСРЦ  ");
        ui->pb_dceInfo->setVisible(true);
        ui->pb_dirInfo->setVisible(true);
        ui->pb_status->setVisible(true);
        ui->pb_enable->setVisible(true);
        ui->pb_disable->setVisible(true);
        ui->pb_net_sync->setVisible(true);
        ui->pb_net_connect->setVisible(true);
        ui->pb_net_disconnect->setVisible(true);
        ui->groupBox_10->setVisible(true);
        ui->groupBox_9->setVisible(true);        
        ui->groupBox_7->setVisible(true);
        ui->groupBox_11->setVisible(false);
        //ui->rb_tcp->setEnabled(false);
        //ui->rb_udp->setEnabled(false);
       // panelStateVisible(true);
       // if (m_wgt_cl!=nullptr)
       //     delete m_wgt_cl;
       // WidgetAU *wgt_au = new WidgetAU(this);
       // m_wgt_cl = new WidgetClient(this);
       // m_wgt_cl = wgt_au;
       // ui->verticalLayout_5->addWidget(m_wgt_cl);
      //  m_wgt_mas->setVisible(false);
       // m_wgt_au->setVisible(true);
    }
}
//------------------------------------------------------------------
// обработка выбора в роли клиента ПАК АУ
void WidgetDTE::slotAu()
{
    if(ui->rb_au->isChecked()){
       m_client_type  = AU;
       ui->lb_title->setText("ПАК АУ АСРЦ");
       QPixmap icon_a {":/icon/pics/a_blue.png"};
       QApplication::setWindowIcon( icon_a );
       setWindowTitle("  ПАК АУ АСРЦ  ");
       ui->pb_dceInfo->setVisible(true);
       ui->pb_dirInfo->setVisible(true);
       ui->pb_status->setVisible(true);
       ui->pb_enable->setVisible(true);
       ui->pb_disable->setVisible(true);
       ui->pb_net_sync->setVisible(true);
       ui->pb_net_connect->setVisible(true);
       ui->pb_net_disconnect->setVisible(true);
       ui->groupBox_10->setVisible(true);
       ui->groupBox_9->setVisible(true);       
       ui->groupBox_7->setVisible(true);
       ui->groupBox_11->setVisible(false);
       //ui->rb_tcp->setEnabled(false);
       //ui->rb_udp->setEnabled(false);
      // panelStateVisible(true);
       //WidgetAU *wgt_au = new WidgetAU(this);
      // m_wgt_cl = new WidgetClient(this);
      // m_wgt_cl = wgt_au;
      // ui->verticalLayout_5->addWidget(m_wgt_cl);
     //  if (m_wgt_cl!=nullptr)
     //      delete m_wgt_cl;
      // m_wgt_mas->setVisible(false);
     //  m_wgt_au->setVisible(true);
    }
    else{
        m_client_type  = MAS;
        ui->lb_title->setText("ПАК МАС");
        QPixmap icon_m {":/icon/pics/m_blue.png"};
        QApplication::setWindowIcon( icon_m );
        setWindowTitle("  ПАК МАС  ");
      //  MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК МАС");
        ui->pb_dceInfo->setVisible(false);
        ui->pb_dirInfo->setVisible(false);
        ui->pb_status->setVisible(false);
        ui->pb_enable->setVisible(false);
        ui->pb_disable->setVisible(false);
        ui->pb_net_sync->setVisible(false);
        ui->pb_net_connect->setVisible(false);
        ui->pb_net_disconnect->setVisible(false);
        ui->groupBox_10->setVisible(true);
        ui->groupBox_9->setVisible(false);        
        ui->groupBox_7->setVisible(false);
        ui->groupBox_11->setVisible(true);
       // ui->rb_tcp->setEnabled(true);
       // ui->rb_udp->setEnabled(true);
      //  panelStateVisible(false);
       // WidgetMAS *wgt_mas = new WidgetMAS(this);
       // m_wgt_cl = new WidgetClient(this);
       // m_wgt_cl = wgt_mas;
       // ui->verticalLayout_5->addWidget(m_wgt_cl);
      //  if (m_wgt_cl!=nullptr)
      //      delete m_wgt_cl;
  //      m_wgt_mas->setVisible(true);
     //   m_wgt_au->setVisible(false);
    }
}
//-------------------------------------------------------------------
// кнопка "Нового соединение" с ПАК УВС
void WidgetDTE::slotNewConnect()
{
    // считываем адрес для работы в сети
    NetS            net_adress;
    net_adress.port = ui->lePort->text().toInt();
    net_adress.address = ui->leAdress->text();

    m_prtcl_client = new ProtocolClient(net_adress, m_client_type, m_type_msg, ui->chb_format->isChecked());

        // обработка события прихода пакета из TransportDTE в MW_DTE(через verificationDTE, protocolClient)
    connect(m_prtcl_client, &ProtocolClient::sigAddMsgToJournal, this, &WidgetDTE::slotPackFromTransport);
       // обработка события прихода состояния соединения из CONNECTORDTE в MW_DTE(через ProtocolDTE)
    connect(m_prtcl_client, &ProtocolClient::sigChangeWorkStatus, this, &WidgetDTE::slotErrorFromTransport);
       // обработка события включения индикаторов состояния в MW_DTE(сигнал высылается из ProtocolDTE
       // после анализа пришедшего сообщения)
    connect(m_prtcl_client, &ProtocolClient::sigPanelOnDTE, this, &WidgetDTE::slotPanelOn);
      // сигнал из MW_DTE для необходимости сформированного (или доформированного ) сообщения
      // в ProtocolDTE и последующей передачей  в Connector_DTE для отправке в ПАК УВС
    connect(this, &WidgetDTE::sigSendCommand, m_prtcl_client, &ProtocolClient::slotSendCommandDTE);
      // сигнал из MW_DTE о разрыве связи с сервером. для передачи через PROTOCOLDTE в ConnectorDTE
    connect(this, &WidgetDTE::sigDiscon, m_prtcl_client, &ProtocolClient::slotDiscon);
    connect(this, SIGNAL(sigExchMode(int)), m_prtcl_client, SLOT(slotExchMode(int)));

    connect(m_prtcl_client, &ProtocolClient::sigPassBit, this, &WidgetDTE::slotPassBitCount);
    connect(m_prtcl_client, &ProtocolClient::sigGetBit, this, &WidgetDTE::slotGetBitCount);
    connect(m_prtcl_client, &ProtocolClient::sigBytesInQue, this, &WidgetDTE::slotBytesInQueCount);
    connect(m_prtcl_client, &ProtocolClient::sigMsgInQue, this, &WidgetDTE::slotMsgInQueCount);
    connect(m_prtcl_client, &ProtocolClient::sigErrBit, this, &WidgetDTE::slotErrBitCount);
    connect(ui->sb_parity_len, SIGNAL(valueChanged(int)), m_prtcl_client, SLOT(slotParityLenGet(int)));
    connect(ui->sb_noparity_len, SIGNAL(valueChanged(int)), m_prtcl_client, SLOT(slotNoParityLenGet(int)));
    connect(ui->sb_parity_ampl, SIGNAL(valueChanged(int)), m_prtcl_client, SLOT(slotParityAmplGet(int)));
    connect(ui->sb_noparity_ampl, SIGNAL(valueChanged(int)), m_prtcl_client, SLOT(slotNoParityAmplGet(int)));

    ui->pb_net->setEnabled(false);
    //ui->rb_tcp->setEnabled(false);
    //ui->rb_udp->setEnabled(false);

}
//------------------------------------------------------------------
void WidgetDTE::slotMsgInQueCount(int count)
{
    m_msg_queueu = count;
}
//------------------------------------------------------------------
void WidgetDTE::slotBytesInQueCount(int count)
{
    m_byte_queueu = count;
}
//------------------------------------------------------------------
void WidgetDTE::slotErrBitCount(int count)
{
    m_bit_err = count;
    ui->le_err->setText(QString::number(m_bit_err));
}
//------------------------------------------------------------------
void WidgetDTE::slotGetBitCount(int count)
{
    m_byte_get = count;
    ui->le_get->setText(QString::number(m_byte_get));
}
//------------------------------------------------------------------
void WidgetDTE::slotPassBitCount(int count)
{
    m_byte_pass = m_byte_pass + count;
    ui->le_pass->setText(QString::number(m_byte_pass));
}
//------------------------------------------------------------------
// разрыв соединения
void WidgetDTE::totalDisconnect()
{
   // this->statusBar()->showMessage("");

    QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
    for (; it1!= m_map_stage.end(); ++it1){
            it1.value()->setOff();// включаем все кнопки
        }
    m_au_obj.status = INIT;  // начальные статус объекта АУ

      ui->pb_version->setEnabled(false);
      ui->pb_Test->setEnabled(false);
      ui->pb_ping->setEnabled(false);
      ui->pb_status->setEnabled(false);
      ui->pb_enable->setEnabled(false);
      ui->pb_disable->setEnabled(false);
      ui->pb_net_sync->setEnabled(false);
      ui->pb_net_connect->setEnabled(false);
      ui->pb_net_disconnect->setEnabled(false);
      ui->pb_stop_start->setEnabled(false);
      ui->pb_dceInfo->setEnabled(false);
      ui->pb_dirInfo->setEnabled(false);
      ui->rb_info->setEnabled(false);
      ui->rb_test->setEnabled(false);

      ui->rb_au->setEnabled(true);
      ui->rb_mas->setEnabled(true);
      ui->rb_json->setEnabled(true);
      ui->rb_xml->setEnabled(true);

      if(m_client_type == MAS){
         setWindowTitle(" ПАК МАС");
         ui->lb_title->setText("ПАК MAC # ...");
       //  ui->rb_tcp->setEnabled(true);
       //  ui->rb_udp->setEnabled(true);

      }
      else{
         setWindowTitle(" ПАК АУ АСРЦ");
         ui->lb_title->setText("ПАК АУ АСРЦ # ...");
       //  ui->rb_tcp->setEnabled(false);
        // ui->rb_udp->setEnabled(false);
      }

      emit sigDiscon();

      disconnect(m_prtcl_client, &ProtocolClient::sigAddMsgToJournal, this, &WidgetDTE::slotPackFromTransport);
      disconnect(m_prtcl_client, &ProtocolClient::sigChangeWorkStatus, this, &WidgetDTE::slotErrorFromTransport);
      disconnect(m_prtcl_client, &ProtocolClient::sigPanelOnDTE, this, &WidgetDTE::slotPanelOn);
      disconnect(this, &WidgetDTE::sigSendCommand, m_prtcl_client, &ProtocolClient::slotSendCommandDTE);
      disconnect(this, &WidgetDTE::sigDiscon, m_prtcl_client, &ProtocolClient::slotDiscon);
      disconnect(this, &WidgetDTE::sigExchMode, m_prtcl_client, &ProtocolClient::slotExchMode);

      disconnect(m_prtcl_client, &ProtocolClient::sigPassBit, this, &WidgetDTE::slotPassBitCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigGetBit, this, &WidgetDTE::slotGetBitCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigBytesInQue, this, &WidgetDTE::slotBytesInQueCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigMsgInQue, this, &WidgetDTE::slotMsgInQueCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigErrBit, this, &WidgetDTE::slotErrBitCount);

      disconnect(ui->sb_parity_len, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotParityLenGet(int)));
      disconnect(ui->sb_noparity_len, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotNoParityLenGet(int)));
      disconnect(ui->sb_parity_ampl, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotParityAmplGet(int)));
      disconnect(ui->sb_noparity_ampl, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotNoParityAmplGet(int)));

      delete  m_prtcl_client;     
}

//-------------------------------------------------------------------
// пришедшее сообщение об ошибке из модуля connectorDTE
void WidgetDTE::slotErrorFromTransport(QString & str, QTime tm)
{
    Q_UNUSED(tm);

    ui->pb_net->setEnabled(true);
    // произошла ошибка при соединении с сервером - связь не установлена

   if(str=="error"){
       // соединение не установлено
      ui->lb_title->setText("ПАК АУ АСРЦ # ...");

      QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
      for (; it1!= m_map_stage.end(); ++it1){
              it1.value()->setOff();// включаем все кнопки
       }
      ui->pb_net->setEnabled(true);
      ui->pb_net->setText("Установить  соединение");
      ui->pb_version->setEnabled(false);
      ui->pb_ping->setEnabled(false);
      ui->pb_Test->setEnabled(false);
      ui->pb_status->setEnabled(false);
      ui->pb_enable->setEnabled(false);
      ui->pb_disable->setEnabled(false);
      ui->pb_net_sync->setEnabled(false);
      ui->pb_net_connect->setEnabled(false);
      ui->pb_net_disconnect->setEnabled(false);
      ui->pb_dceInfo->setEnabled(false);
      ui->pb_dirInfo->setEnabled(false);
      ui->rb_info->setEnabled(false);
      ui->rb_test->setEnabled(false);

      disconnect(m_prtcl_client, &ProtocolClient::sigAddMsgToJournal, this, &WidgetDTE::slotPackFromTransport);
      disconnect(m_prtcl_client, &ProtocolClient::sigChangeWorkStatus, this, &WidgetDTE::slotErrorFromTransport);
      disconnect(m_prtcl_client, &ProtocolClient::sigPanelOnDTE, this, &WidgetDTE::slotPanelOn);
      disconnect(this, &WidgetDTE::sigSendCommand, m_prtcl_client, &ProtocolClient::slotSendCommandDTE);
      disconnect(this, &WidgetDTE::sigDiscon, m_prtcl_client, &ProtocolClient::slotDiscon);
      disconnect(this, &WidgetDTE::sigExchMode, m_prtcl_client, &ProtocolClient::slotExchMode);

      disconnect(m_prtcl_client, &ProtocolClient::sigPassBit, this, &WidgetDTE::slotPassBitCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigGetBit, this, &WidgetDTE::slotGetBitCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigBytesInQue, this, &WidgetDTE::slotBytesInQueCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigMsgInQue, this, &WidgetDTE::slotMsgInQueCount);
      disconnect(m_prtcl_client, &ProtocolClient::sigErrBit, this, &WidgetDTE::slotErrBitCount);
      disconnect(ui->sb_parity_len, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotParityLenGet(int)));
      disconnect(ui->sb_noparity_len, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotNoParityLenGet(int)));
      disconnect(ui->sb_parity_ampl, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotParityAmplGet(int)));
      disconnect(ui->sb_noparity_ampl, SIGNAL(changeValue(int)), m_prtcl_client, SLOT(slotNoParityAmplGet(int)));

      delete  m_prtcl_client;
   }
   else if(str =="ok"){
       // соединение установлено
       ui->pb_dceInfo->setEnabled(true);
       ui->pb_dirInfo->setEnabled(true);
       ui->pb_version->setEnabled(true);
       ui->pb_ping->setEnabled(true);
       ui->pb_status->setEnabled(true);
       ui->pb_Test->setEnabled(true);
       ui->pb_enable->setEnabled(true);

       ui->pb_net->setText("Разорвать  соединение");
       ui->pb_net->setEnabled(true);

       ui->rb_au->setEnabled(false);   // роль клиента больше менять нельзя
       ui->rb_mas->setEnabled(false);
       ui->rb_json->setEnabled(false);
       ui->rb_xml->setEnabled(false);
       ui->rb_info->setEnabled(true);
       ui->rb_test->setEnabled(true);

       // определение ip адреса машины на которой запущено приложение

      // const QString portNum = QString::number(m_protocolDTE->getPortNumber());
       QString ipAdr ="";
       QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
       QList<QHostAddress> listAddr= info.addresses();
       ipAdr = "";    //listAddr.first().toString();
       for(int i = 0; i <listAddr.count(); ++i ){
           ipAdr = ipAdr+" * " + listAddr.at(i).toString()+" * ";
       }
   //    if(m_client_type == AU)
   //       MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК АУ АСРЦ  ip[ "+ ipAdr+" ]  port[ " + portNum+" ]");
   //    else if(m_client_type == MAS)
   //        MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК МАС  ip[ "+ ipAdr+" ]  port[ " + portNum+" ]");
   }
}
//------------------------------------------------------------------
// функция формирования заголовка для отображения в информационном окне приложения
// вызывается из "slotMessFromConnector"
QString WidgetDTE::createTitle(QColor &cl, Qt::Alignment &al,int tx)
{
    QString title("");
    cl = Qt::black;

    if(!tx){
        title = "Сообщение от ПАК УВС:";
         cl= Qt::red;
         al = Qt::AlignRight;
    }
    else if(tx){
       title = "Сообщение ПАК AY:";
        cl= Qt::blue;
        al = Qt::AlignLeft;
    }
    return  title;
}
//------------------------------------------------------------------
//  выделение наименования команды json
QString WidgetDTE::jsonCmd(const QString & str_json)
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
             QString test =list.at(i);
             com.clear();
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
//--------------------------------------------------------------
//  выделение наименования команды xml
QString WidgetDTE::xmlCmd(const QString & str_xml)
{
    QString str = str_xml;
    QString type = "<cmd>";
    QString status = "<status>";
    int comPos = str.indexOf("<cmd>") + type.size();
    int statusPos = str.indexOf("<status>") + status.size();
    int comEndPos = str.indexOf("</cmd>", comPos);
    int statusEndPos = str.indexOf("</status>", statusPos);
    QString com{"???"};
    if(comPos  > 0){
        com = str.mid(comPos, comEndPos - comPos);
    }
    if(statusPos > 7)
       com = com +" : "+str.mid(statusPos, statusEndPos - statusPos);
    return "  "+com;
}
//------------------------------------------------------------------
//   выделение команды из строки для отображения в таблице
 QString WidgetDTE::getTypeCom(const QString & str)
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
  //  пришедшее сообщение о пришедшем пакете из модуля транспорт
 void WidgetDTE::slotPackFromTransport(int tx, QByteArray & ba, QDateTime tm)
 {
   // отображаем в окне журнала принятый пакет.
     QColor cl = Qt::black;
     Qt::Alignment al;
     QString mess_title = createTitle(cl, al, tx);
     QString type_com = "";
     type_com = getTypeCom(QString(ba));
     emit sigWriteInJourn(tx, tm, type_com, QString(ba), cl);   // отображаем запись в журнале

 }
 //--------------------------------------------------------------------------
 // обработка таймера запущего при запуске команды и не получения ответа от ПАК УВС в назначенный промежуток времени
 void WidgetDTE::slotTimerAuAlarm()
 {
    QMessageBox::critical(0, "Сообщение", "Время ожидания ответа от ПАК АУ закончилось." ,"");
   //  m_autimer->stop();
     unlockEnableButtons();

     if ( m_au_obj.status == ENABLE) { // значит не состоялась синхронизация
         QMap<QString, StageMarkerButton *> ::iterator it =  m_map_stage.begin();
         for (; it!= m_map_stage.end(); ++it){
             if(it.key() == "Отсутствие сетевой \n синхронизации" ){
                 it.value()->setOn(STATE_BUTTON_WIDTH);// включаем кнопку "Отсутствие сетевой \n синхронизации"
             }
         }
     }

     else if ( m_au_obj.status == NET_SYNC) { // значит не получилось установить канал связи
                                              //(либо проблема с абонентом либо нет свободного канала),
                                              // либо авария аппаратуры
         QMap<QString, StageMarkerButton *> ::iterator it =  m_map_stage.begin();
         for (; it!= m_map_stage.end(); ++it){
             if(it.key() == "Авария аппаратуры" ){
                 it.value()->setOn(STATE_BUTTON_WIDTH);// включаем кнопку "Авария аппаратуры"
             }
         }
     }
 }

 //-------------------------------------------------------------------
 //--  Настройки внешнего вида MainWindow ----------------------------
 //-------------------------------------------------------------------
 void WidgetDTE::initMainWind()
 {
   m_text_cl = Qt::blue;

   ui->lb_title->setText("ПАК АУ АСРЦ # ...");
      // начало работы

   ui->pb_net->setText("Установить  соединение");
   ui->pb_version->setEnabled(false);
   ui->pb_status->setEnabled(false);
   ui->pb_ping->setEnabled(false);
   ui->pb_enable->setEnabled(false);
   ui->pb_disable->setEnabled(false);
   ui->pb_net_sync->setEnabled(false);
   ui->pb_net_connect->setEnabled(false);
   ui->pb_net_disconnect->setEnabled(false);
  // ui->pb_exchange->setEnabled(false);
   ui->pb_stop_start->setEnabled(false);
   ui->pb_dceInfo->setEnabled(false);
   ui->pb_dirInfo->setEnabled(false);
   ui->pb_Test->setEnabled(false);
   ui->rb_info->setEnabled(false);
   ui->rb_test->setEnabled(false);

   ui->chb_format->setChecked(false);

   QPalette pl;
   pl.setBrush(QPalette::Active,QPalette::WindowText, QColor(m_text_cl));
   pl.setBrush(QPalette::Disabled,QPalette::WindowText, QColor(m_text_cl));
   pl.setBrush(QPalette::Inactive,QPalette::WindowText, QColor(m_text_cl));

   pl.setBrush(QPalette::Active, QPalette::ButtonText, QColor(m_text_cl));
   pl.setBrush(QPalette::Disabled, QPalette::ButtonText, QColor(m_text_cl));
   pl.setBrush(QPalette::Inactive, QPalette::ButtonText, QColor(m_text_cl));

   this->setPalette(pl);
   ui->lb_title->setPalette(pl);

   //============================ Панель индикации состояний ПАК АУ ===========================
   if( m_client_type == AU){
      panelStateCreate();
   }
   //===================================== Установка режима работы ПАК МАС=====================
   else if (m_client_type == MAS){

      if(ui->rb_info->isChecked()){
         m_exch_mode = INFO_EXCH;
         ui->le_err->setEnabled(false);
       }
      else{
          m_exch_mode = TEST_EXCH;
          ui->le_err->setEnabled(true);
      }
   }
 }
 //-------------------------------------------------------------------
 void WidgetDTE::panelStateVisible(bool visible)
 {
    if (visible){
        QList<StageMarkerButton *> bt_list= m_map_stage.values();
        for (int i  = 0; i < m_map_stage.count(); ++i){
            bt_list.at(i)->setFixedHeight(STATE_BUTTON_WIDTH);
         }
    }
    else{
        QList<StageMarkerButton *> bt_list= m_map_stage.values();
        for (int i  = 0; i < m_map_stage.count(); ++i){
            bt_list.at(i)->setFixedHeight(0);
         }
    }
 }
 //-------------------------------------------------------------------
 void WidgetDTE::panelStateCreate()
 {
     m_map_stage.clear();
     StageMarkerButton *tb_1= new StageMarkerButton(CORRECT,"Ведется сеанс связи", this);
     m_map_stage.insert("Ведется сеанс связи",tb_1);

     StageMarkerButton *tb_2= new StageMarkerButton(INFORM,"ПАК УВС вкл.", this); //+
     m_map_stage.insert("ПАК УВС вкл.",tb_2);

     StageMarkerButton *tb_3= new StageMarkerButton(DAMAGE,"Авария аппаратуры", this);
     m_map_stage.insert("Авария аппаратуры",tb_3);

     StageMarkerButton *tb_4= new StageMarkerButton(INFORM,"Предлагаемый канал связи № ... ", this);  //+
     m_map_stage.insert("Предлагаемый канал связи № ... ",tb_4);

          // индикатор удален по просьбе Хвостунова Ю.С.
     //StageMarkerButton *tb_5= new StageMarkerButton(WARING,"Прекращение сеанса связи", this);
     //m_map_stage.insert("Прекращение сеанса связи",tb_5);

     StageMarkerButton *tb_5= new StageMarkerButton(WARING,"Ожидание \n сеанса связи", this);  //+
     m_map_stage.insert("Ожидание \n сеанса связи",tb_5);
     StageMarkerButton *tb_6= new StageMarkerButton(INFORM,"Сеть занята", this);  // +
     m_map_stage.insert("Сеть занята",tb_6);

     StageMarkerButton *tb_7= new StageMarkerButton(INFORM,"Абонент занят или выключен", this); //+
     m_map_stage.insert("Абонент занят или выключен",tb_7);

     StageMarkerButton *tb_8= new StageMarkerButton(CORRECT,"Обмен информацией успешно завершён", this);
     m_map_stage.insert("Обмен информацией успешно завершен",tb_8);

     StageMarkerButton *tb_9= new StageMarkerButton(INFORM,"Синхронизация завершена", this);
     m_map_stage.insert("Синхронизация завершена",tb_9);

     StageMarkerButton *tb_10= new StageMarkerButton(WARING,"Отсутствие сетевой \n синхронизации", this);
     m_map_stage.insert("Отсутствие сетевой \n синхронизации",tb_10);

        //  добавление индикаторов в верхнуюю часть гравной формы ПАК АУ

     QList<StageMarkerButton *> bt_list= m_map_stage.values();
     for (int i  = 0; i < m_map_stage.count(); ++i){          
          bt_list.at(i)->setFixedWidth(0);//STATE_BUTTON_HIGHT);
          ui->horizontalLayout_2->addWidget(bt_list.at(i));
      }
 }
 //-------------------------------------------------------------------
 // блокируем все кнопки экранной формы на время отправления сообщения
 // серверу и ожидания ответа от него
 // если ответ на команду придет с ошибкой кнопки останутся заблокированными

 void WidgetDTE::lockAllButtons()
 {
     ui->pb_dceInfo->setEnabled(false);
    // ui->pb_dirInfo->setEnabled(false);
     ui->pb_version->setEnabled(false);
     ui->pb_ping->setEnabled(false);
     ui->pb_status->setEnabled(false);
     ui->pb_enable->setEnabled(false);
     ui->pb_disable->setEnabled(false);
     ui->pb_net_sync->setEnabled(false);
     ui->pb_net_connect->setEnabled(false);
     ui->pb_net_disconnect->setEnabled(false);
   // ui->rb_info->setEnabled(false);
  //   ui->rb_test->setEnabled(false);
     //ui->pb_exchange->setEnabled(false);
    // ui->pb_stop_test->setEnabled(false);
     ui->pb_disable->setEnabled(false);
    // this->statusBar()->showMessage("  Ждите завершения выполнения команды.  ");
 }
 //-------------------------------------------------------------------
 // разблокируем экранные кнопки доступные для текущего состояния
 // в связи с пришедшей и обработанной командой

 void WidgetDTE::unlockEnableButtons()
 {
     lockAllButtons();

  //   this->statusBar()->showMessage("  Команда выполнена.  ");
     ui->pb_version->setEnabled(true);
     ui->pb_ping->setEnabled(true);
     ui->pb_status->setEnabled(true);
     ui->pb_dceInfo->setEnabled(true);
     ui->pb_enable->setEnabled(true);

    // ui->pb_exchange->setEnabled(true);
 //     ui->pb_stop_test->setEnabled(true);

     switch(m_au_obj.status){

     case  INIT :   ui->pb_enable->setEnabled(true);
                    ui->pb_status->setEnabled(true);

                    break;

     case  ENABLE: ui->pb_net_sync->setEnabled(true);
                   ui->pb_disable->setEnabled(true);
                   ui->pb_status->setEnabled(true);
                   break;

     case  NET_SYNC: ui->pb_net_connect->setEnabled(true);                     
                     ui->pb_disable->setEnabled(true);
                     ui->pb_status->setEnabled(true);
                     break;

     case  NET_CONNECT: ui->pb_net_disconnect->setEnabled(true);
                        ui->pb_dirInfo->setEnabled(true);
                        ui->pb_disable->setEnabled(true);
                        ui->pb_status->setEnabled(true);
                        break;
     case  EXCH:
              //  ui->pb_exchange->setEnabled(true);
               // ui->pb_stop_start->setEnabled(true);
                break;


     case  NET_DISC: ui->pb_disable->setEnabled(true);
                     ui->pb_status->setEnabled(true);
                     break;

     default:break;;
     }
 }
 //-------------------------------------------------------------------
 void WidgetDTE::slotNetWorking()
 {
     QString title = ui->pb_net->text();

     if(title == "Установить  соединение")
     {
         ui->pb_net->setText("Разорвать  соединение");
         slotNewConnect();
         ui->chb_format->setEnabled(false);
        // ui->pb_exchange->setEnabled(true);
       //  ui->pb_stop_start->setEnabled(true);
         ui->rb_info->setEnabled(true);
         ui->rb_test->setEnabled(true);
         if (m_client_type == AU)
             slotTermType();     // ком. termType отправляется при уста. только если клиент является ПАК АУ
     }
     else if (title == "Разорвать  соединение")
     {
         ui->pb_net->setText("Установить  соединение");
         ui->chb_format->setEnabled(true);
      //   ui->pb_exchange->setEnabled(false);
         ui->pb_stop_start->setEnabled(false);
         ui->rb_info->setEnabled(false);
         ui->rb_test->setEnabled(false);
         slotDiscon();
        // m_info_pack_timer->stop();   // останавливаем таймер отправки ExchangeDCE если он запущен
        // m_autimer->stop();           // останавливаем таймер ожидания ответного сообщения
     }

     QString ipAdr ="";
     QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
     QList<QHostAddress> listAddr= info.addresses();
     for(int i = 0; i <listAddr.count(); ++i ){
         ipAdr = ipAdr+" * " + listAddr.at(i).toString()+" * ";
     }

  //   if(m_client_type == MAS){
  //       MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК МАС ip[ " + ipAdr+" ]");
  //   }
  //   else
  //      MainWindDTE::setWindowTitle("("+ui->lePort->text()+ ") ПАК АУ АСРЦ ip[ " + ipAdr+" ]");
 }

 //-------------------------------------------------------------------

 //-----------------------------------------------------------------
 // слот обрабатывает сигнал из proctocolDTE сообщающий что поступило сообщение
 // и необходимо произвести некоторые действия на главной форме в связи с этим

 void WidgetDTE::slotPanelOn(StageMarkerButEnum stage, QString &free_param)
 {
    switch (stage) {
    case StageMarkerButEnum::ENBL:
    {QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
        for (; it1!= m_map_stage.end(); ++it1){
                it1.value()->setOff();// включаем все кнопки
            }
        QMap<QString, StageMarkerButton *> ::iterator it =  m_map_stage.begin();
        for (; it!= m_map_stage.end(); ++it){
            if(it.key() == "ПАК УВС вкл." ){
                m_au_obj.status = ENABLE;
                it.value()->setOn(STATE_BUTTON_WIDTH);// включаем кнопку "Дежурный приём"
                m_autimer->stop(); // останавливаем таймер
                unlockEnableButtons();
            }
         }
    }break;

    case StageMarkerButEnum::SYNCH:
    {
        QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
        for (; it1!= m_map_stage.end(); ++it1){
                it1.value()->setOff();// включаем все кнопки
            }
        QMap<QString, StageMarkerButton *> ::iterator it =  m_map_stage.begin();
        for (; it!= m_map_stage.end(); ++it){
            if(it.key() == "Синхронизация завершена" ){
                m_au_obj.status = NET_SYNC;
                it.value()->setOn(STATE_BUTTON_WIDTH);// включаем кнопку "Синхронизация завершена"
                m_autimer->stop(); // останавливаем таймер
                 unlockEnableButtons();
            }
         }
    } break;

    case StageMarkerButEnum::CONNECT_END:
    {
        QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
        for (; it1!= m_map_stage.end(); ++it1){
                it1.value()->setOff();// включаем все кнопки
            }

        QMap<QString, StageMarkerButton *> ::iterator it =  m_map_stage.begin();
        m_au_obj.status = NET_CONNECT;
        for (; it!= m_map_stage.end(); ++it){

            if(it.key() == "Ведется сеанс связи" ){
                it.value()->setOn(STATE_BUTTON_WIDTH);// включаем кнопку "Ведется сеанс связи"
                //break;
            }
            else if(it.key().contains("Предлагаемый канал связи № ")){
                it.value()->setOn(STATE_BUTTON_WIDTH);// выключаем кнопку
                it.value()->setTitle("Предлагаемый канал связи :  \n"+ free_param);// выключаем кнопку
                m_autimer->stop(); // останавливаем таймер
                unlockEnableButtons();
            }
      }
    } break;

    case StageMarkerButEnum::DISCON:
    {
        m_au_obj.status = NET_DISC;

        QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
        for (; it1!= m_map_stage.end(); ++it1){
                it1.value()->setOff();// включаем все кнопки
            }
         m_autimer->stop(); // останавливаем таймер
         unlockEnableButtons();
        QMap<QString, StageMarkerButton *> ::iterator it =  m_map_stage.begin();
        for (; it!= m_map_stage.end(); ++it){
            if(it.key() == "ПАК УВС вкл." ){
                it.value()->setOn(STATE_BUTTON_WIDTH);// включаем кнопку "Дежурный приём"
                 unlockEnableButtons();
            }
         }
    } break;

    case StageMarkerButEnum::DISBL:
    {
        QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
        for (; it1!= m_map_stage.end(); ++it1){
                it1.value()->setOff();// включаем все кнопки
            }
        m_au_obj.status = INIT;
        m_autimer->stop(); // останавливаем таймер
        unlockEnableButtons();
    } break;

    default:
    {
        m_autimer->stop(); // останавливаем таймер
        unlockEnableButtons();
    }break;
    }
 }
 //-----------------------------------------------------------------
 // команда TermType - представлене клиента серверу
 void WidgetDTE::slotTermType()
 {
     bool res = false;

     if(m_prtcl_client!=nullptr){  // если объект протокола создан
         res = m_prtcl_client->termTypeCmdSend();
     }

     if(res){
         lockAllButtons();             // отключаем от использования все кнопки
         m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
     }
     else
         QMessageBox::critical(0, "Сообщение","Команда TermType не отправлена." ,"");
 }
 //-----------------------------------------------------------------
 // нажатие клиента на кнопку PING
 // формируем здесь JSON объект для отправки на сервер
 void WidgetDTE::slotPing()
 {
     bool res = false;

     if(m_prtcl_client!=nullptr){  // если объект протокола создан
         res = m_prtcl_client->pingCmdSend();
     }

     if(res){
         lockAllButtons();             // отключаем от использования все кнопки
         m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
     }
     else
         QMessageBox::critical(0, "Сообщение","Команда Ping не отправлена." ,"");
 }
 //-----------------------------------------------------------------
 // нажатие клиента на кнопку VERSION
 void WidgetDTE::slotVersion()
 {
     bool res = false;

     if(m_prtcl_client!=nullptr){  // если объект протокола создан
         res = m_prtcl_client->versionCmdSend();
     }
     if(res){
         lockAllButtons();             // отключаем от использования все кнопки
         m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
     }
     else
         QMessageBox::critical(0, "Сообщение","Команда Version не отправлена." ,"");

   }
 //-----------------------------------------------------------------------
 //              Режим ПАК АУ
 //-----------------------------------------------------------------------
 void WidgetDTE::slotDceInfo()
 {
     bool res = false;

     if(m_prtcl_client!=nullptr){  // если объект протокола создан
         res = m_prtcl_client->dceInfoCmdSend();
     }

     if(res){
         lockAllButtons();             // отключаем от использования все кнопки
         m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
     }
     else
         QMessageBox::critical(0, "Сообщение","Команда GetDceInfo не отправлена." ,"");
 }
 //-----------------------------------------------------------------------
 void WidgetDTE::slotDirInfo()
{
    bool res = false;

    if(m_prtcl_client!=nullptr){  // если объект протокола создан
        res = m_prtcl_client->dirInfoCmdSend();
    }

    if(res){
        lockAllButtons();             // отключаем от использования все кнопки
        m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
    }
    else
        QMessageBox::critical(0, "Сообщение","Команда GetRadioDirInfo не отправлена." ,"");
}
 //-----------------------------------------------------------------------
 // нажатие клиента на кнопку Status
 void WidgetDTE::slotStatus()
 {
     bool res = false;

     if(m_prtcl_client!=nullptr){  // если объект протокола создан
         res = m_prtcl_client->statusCmdSend();
     }

     if(res){
         lockAllButtons();             // отключаем от использования все кнопки
         m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
     }
     else
         QMessageBox::critical(0, "Сообщение","Команда Status не отправлена." ,"");
 }
 //-----------------------------------------------------------------------
 // нажатие клиента на кнопку Test_cmd
 void WidgetDTE::slotTestCmd()
 {
     bool res = false;

     if(m_prtcl_client!=nullptr){  // если объект протокола создан
         res = m_prtcl_client->testCmdSend();
     }

     if(res){
         lockAllButtons();             // отключаем от использования все кнопки
         m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
     }
     else
         QMessageBox::critical(0, "Сообщение","Команда Tест не отправлена." ,"");
 }
 //-----------------------------------------------------------------------
 // нажатие клиента на кнопку enable
 void WidgetDTE::slotEnable()
{
    EnableParamStruct data_enable;
    quint32  res_id{ 0 };  // номер dce_id отправленное на сервер
    data_enable.param_port = ui->lePort->text().toInt(); // для передачи в форму команды порта ПАК МАС
    if(m_prtcl_client!=nullptr){
       m_dialog_com = new EnableDCE_dial(data_enable, this);
       if(m_dialog_com->exec() == QDialog::Accepted){
         if(1)/*m_au_obj.status == INIT )*/{
             res_id = m_prtcl_client->enableCmdSend(&data_enable);
          }
          if(res_id){
             ui->lb_title->setText("ПАК АУ АСРЦ # " + QString::number(res_id)); // выводим номер на MW
             m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
             lockAllButtons();
          }
         else
             QMessageBox::critical(0, "Сообщение","Команда EnableDCE не отправлена." ,"");
     }
     delete m_dialog_com;
     m_dialog_com = nullptr;
    }
 }
 //-----------------------------------------------------------------
 void WidgetDTE::slotNetSync()
 {
     NetSYNCParamStruct data_synchro;

     if(m_prtcl_client!=nullptr){
        m_dialog_com = new NetSYNC_dial(data_synchro, this);
        if(m_dialog_com->exec() == QDialog::Accepted){
          if(1){//(m_au_obj.status == ENABLE || m_au_obj.status == NET_SYNC
           // || m_au_obj.status == NET_CONNECT || m_au_obj.status == EXCH){
              if( m_prtcl_client->syncCmdSend(&data_synchro)){
                 m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
                 lockAllButtons();
              }
              else
                  QMessageBox::critical(0, "Сообщение","Команда NetworkSYNC не отправлена." ,"");
          }
      }
      delete m_dialog_com;
      m_dialog_com = nullptr;
     }
 }
 //------------------------------------------------------------------
 void WidgetDTE::slotNetConnect()
 {
     NetCONNECTParamStruct data_connect;

     if(m_prtcl_client != nullptr){
       m_dialog_com = new NetCONNECT_dial(data_connect, this);
       if(m_dialog_com->exec() == QDialog::Accepted){
         if(1){//m_au_obj.status == NET_SYNC || m_au_obj.status == NET_CONNECT){ // установка соединеня возможна только после включения
             if( m_prtcl_client->connectCmdSend(&data_connect)){
                 m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
                 lockAllButtons();
              }
             else
              QMessageBox::critical(0, "Сообщение","Команда NetworkCONNECT не отправлена." ,"");
          }
       }
       delete m_dialog_com;
       m_dialog_com = nullptr;
       }
 }
 //------------------------------------------------------------------
 // нажатие клиента на кнопку enable
 void WidgetDTE::slotDisable()
 {
     DisableParamStruct data_disable;
     if(m_prtcl_client!=nullptr){
         m_dialog_com = new DisableDCE_dial(data_disable, this);
         if(m_dialog_com->exec() == QDialog::Accepted){
           if(1){//(m_au_obj.status == ENABLE || m_au_obj.status == NET_SYNC
            // || m_au_obj.status == NET_CONNECT || m_au_obj.status == EXCH){
               if( m_prtcl_client->disableCmdSend(&data_disable)){
                  m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания
                  lockAllButtons();
               }
               else
                   QMessageBox::critical(0, "Сообщение","Команда NetworkSYNC не отправлена." ,"");
           }
       }
       delete m_dialog_com;
       m_dialog_com = nullptr;
      }

     QMap<QString, StageMarkerButton *> ::iterator it1 =  m_map_stage.begin();
     for (; it1!= m_map_stage.end(); ++it1){
             it1.value()->setOff();// включаем все кнопки
         }
 } 
 //------------------------------------------------------------------
 void WidgetDTE::slotNetDisconnect()
 {
     NetDISCONParamStruct data_discon;

     if(m_prtcl_client != nullptr){
       m_dialog_com = new NetDisconnect_dial(data_discon, this);
       if(m_dialog_com->exec() == QDialog::Accepted){
         if(1){// m_au_obj.status == NET_CONNECT){ //
             if( m_prtcl_client->disconCmdSend(&data_discon)){
                 m_autimer->start(time_wait);  // устанавливаем таймер на время ожидания ответа
                 lockAllButtons();
              }
             else
              QMessageBox::critical(0, "Сообщение","Команда NetworkDISCON не отправлена." ,"");
          }
       }
       delete m_dialog_com;
       m_dialog_com = nullptr;
       }
 }
 //------------------------------------------------------------------
 //         Режим ПАК МАС
 //------------------------------------------------------------------
 // обработка нажатия на кнопку ExchangeDCE
 void WidgetDTE::slotExchange()
 {
     ExchangeParamStruct data_exchange;

     ui->le_get->setText("0");
     ui->le_pass->setText("0");
     time_info_pack = ui->sp_exch_time->value();

   //  ui->rb_test->setEnabled(false);
   //  ui->rb_info->setEnabled(false);

     if (m_exch_mode == TEST_EXCH){
         ui->le_err->setText("0");
     }
       //     ui->pb_exchange->setEnabled(false);

     ui->pb_stop_start->setText("Инфообмен\n ||");

     if(m_prtcl_client != nullptr){  // если класс протокола создан
        m_dialog_com = new  ExchageDCE_dial(data_exchange, m_byte_pass, int(m_exch_mode), this);
       // m_dialog_com->show();  ///!! убрать после автоматизации        

        if(m_dialog_com->exec() == QDialog::Accepted){// замена
        //  if(1){//m_au_obj.status == NET_CONNECT){ // обмен информацией возможен только после установления соединения
               ++m_msg_queueu;   // отправили сообщение даже если ответа пока не дождались и не можетм оценить величину очереди
               m_exch_struct = data_exchange;
               if( m_prtcl_client->exchCmdSend(&data_exchange)){
                  lockAllButtons();

                  m_msg_timer->start(time_info_pack);                 
                  ui->sb_noparity_len->setEnabled(false);
                  ui->sb_parity_len->setEnabled(false);
                  ui->sb_noparity_ampl->setEnabled(false);
                  ui->sb_parity_ampl->setEnabled(false);
                  ui->rb_test->setEnabled(true);
                  ui->rb_info->setEnabled(true);
                  ui->pb_stop_start->setEnabled(true);
                }
               else{
                  QMessageBox::critical(0, "Сообщение","Команда ExchangeDCE не отправлена." ,"");
          //        ui->pb_exchange->setEnabled(true);  // если команда не запущена, кнопка вызова диалог. окна - доступна
               }
          }

        else {
            ui->sb_noparity_len->setEnabled(true);
            ui->sb_parity_len->setEnabled(true);
            ui->sb_noparity_ampl->setEnabled(true);
            ui->sb_parity_ampl->setEnabled(true);
            ui->pb_stop_start->setEnabled(false );
       //     ui->pb_exchange->setEnabled(true);
        }

        delete m_dialog_com;
        m_dialog_com = nullptr;
     }
 }
 //--------------------------------------------------------------------------
 // подготовка вариативной последовательности для передачи
 ExchangeParamStruct WidgetDTE::makeVaryableData(const ExchangeParamStruct & data_struct){

     ExchangeParamStruct exch_param;

     if(m_vary_count == 255)
        m_vary_count = 0;

     ++m_vary_count;    // счетчик, какой раз изменяем последовательность для передачи(относительно первоначальной послед.)

     exch_param.type = data_struct.type;
     exch_param.dce_id = data_struct.dce_id;
     exch_param.dce_id_corr = data_struct.dce_id_corr;
     exch_param.msg_id = data_struct.msg_id;
     exch_param.data_general.append(data_struct.data_general);

     QByteArray ba_data, ba_temp;

     // перезаписываем четную последов.
     ba_temp.append(QByteArray::fromBase64(data_struct.data_parity.data())); // декод. четн. послед.
     for(int i = 0; i < ba_temp.count(); ++i){
          ba_data.append(ba_temp.at(0) + m_vary_count);
     }
     exch_param.data_parity.append(ba_data.toBase64()); // в структуру помещаем кодир. послед.

     // перезаписываем нечет послед.
     ba_data.clear();
     ba_temp.clear();
     ba_temp.append(QByteArray::fromBase64(data_struct.data_noparity.data())); // декод. нечетн. послед.
     for(int i = 0; i < ba_temp.count(); ++i){
          ba_data.append(ba_temp.at(0) + m_vary_count);
     }
     exch_param.data_noparity.append(ba_data.toBase64());

     return exch_param;
 }
 //--------------------------------------------------------------------------
 void WidgetDTE::slotTimerMsgAlarm()
 {

     if(!m_timer_stop){
        if(m_msg_queueu <= ui->cb_qDepth->currentText().toInt()){
           if(m_prtcl_client != nullptr){  // если класс прот-ла создан
              if(1){//m_au_obj.status == NET_CONNECT){ // обмен информацией возможен только после установления соединения
                   ++m_msg_queueu;   // отправили сообщение даже если ответа пока не дождались и не можетм оценить величину очереди
                   ++m_exch_struct.msg_id;

                  ExchangeParamStruct data_exchange;    // структура для передачи в команде ExchangeDCE

                  if(m_varyable)  // если задан режим в вариативной последовательностью
                     data_exchange = makeVaryableData(m_exch_struct); // постоянную последованельность превращаем в вариативную

                   if( m_prtcl_client->exchCmdSend(&data_exchange)){//&m_exch_struct)){
                      lockAllButtons();
                      ui->pb_stop_start->setEnabled(true);
                      ui->rb_test->setEnabled(true);
                      ui->rb_info->setEnabled(true);
                      ui->pb_stop_start->setEnabled(true);
                    }
                   else{
                      QMessageBox::critical(0, "Сообщение","Команда ExchangeDCE не отправлена." ,"");
                      ui->pb_stop_start->setEnabled(false);
                      //     ui->pb_exchange->setEnabled(true);  // если команда не запущена, кнопка вызова диалог. окна - доступна
                   }
               }
           }          
        }
     }
 } 
 //------------------------------------------------------------------
 void WidgetDTE::slotStopTest()
 {
     QString title = ui->pb_stop_start->text();
     time_info_pack = ui->sp_exch_time->value();
     m_msg_timer->stop();

     if (title == "Инфообмен\n |>"){
         m_timer_stop = false;
         m_msg_timer->start(time_info_pack);
         ui->pb_stop_start->setText("Инфообмен\n ||");
         ui->pb_version->setEnabled(false);
         ui->pb_ping->setEnabled(false);
         ui->pb_status->setEnabled(false);
         ui->pb_net->setEnabled(false);
         }
     else if (title =="Инфообмен\n ||"){
          m_timer_stop = true;
          ui->pb_stop_start->setText("Инфообмен\n |>");
          ui->pb_version->setEnabled(true);
          ui->pb_ping->setEnabled(true);
          ui->pb_status->setEnabled(true);
          ui->pb_net->setEnabled(true);
     }
 }
//--------------------------------------------------------------------

