#include "widget-exch-IQ.h"
#include "ui_widget-exch-IQ.h"
#include <QHostInfo>
#include <QUuid>
#include <QJsonObject>
#include <QJsonArray>
#include <QMessageBox>

//-----------------------------------------------------------------------------------
Widget_Exch_IQ::Widget_Exch_IQ(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget_Exch_IQ)
{
    ui->setupUi(this);  

    connect(ui->pb_net, SIGNAL(clicked()), this, SLOT(slotNetWorkingExch()));     // кнопка  "Начать прослушку"    
    connect(ui->sp_ZPCH, SIGNAL(valueChanged(int)), this,SLOT(slotZpchChanged(int)));

    // определение ip адреса машины на которой запущено приложение
    QString ipAdr ="";
    QHostInfo info = QHostInfo::fromName( QHostInfo::localHostName() );
    QList<QHostAddress> listAddr= info.addresses();
    ipAdr = listAddr.first().toString();

  // setPort(2324);

    ui->ch_start_buff->setEnabled(false);
    ui->sp_ZPCH->setEnabled(false);
}

//--------------------------------------------------------------------------------------
Widget_Exch_IQ::~Widget_Exch_IQ()
{
    setDisconnect();
    delete ui;
}
//-------------------------------------------------------------------------------------
// настройка интерфейса виджета в случае роли RX (ПРМ) ПАК УВС
void Widget_Exch_IQ::setRXInterface()
{
    ui->le_port_bind->setVisible(true);
    ui->le_port_cor->setVisible(false);
    ui->le_adr_cor->setVisible(false);

    ui->label_2->setVisible(false);
    ui->label_8->setVisible(false);
    ui->label_7->setVisible(true);
}
//-------------------------------------------------------------------------------------
// настройка интерфейса виджета в случае роли TX (ПРД) ПАК УВС
void Widget_Exch_IQ::setTXInterface()
{
    ui->le_port_bind->setVisible(false);
    ui->le_port_cor->setVisible(true);
    ui->le_adr_cor->setVisible(true);

    ui->label_2->setVisible(true);
    ui->label_8->setVisible(true);
    ui->label_7->setVisible(false);
}
//-------------------------------------------------------------------------------------
// задание параметров работы UDP соединения (номера порта,.... из командной стр.
void Widget_Exch_IQ::setParams(NetS adr, int port_bind, int index_zpch, int start_buf)
{
    ui->le_port_bind->setText(QString::number(port_bind));
    ui->le_port_cor->setText(QString::number(adr.port));
    ui->le_adr_cor->setText(adr.address);
    ui->sp_ZPCH->setValue(index_zpch);
    ui->ch_start_buff->setChecked(start_buf);
    startRadioWork();   // включить работу радио сети
    emit sigZpchCh(index_zpch);
   // emit ui->ch_start_buff->click();
}
//------------------------------------------------------------------------------------
void Widget_Exch_IQ::stopRadioWork()
{
    QString title = ui->pb_net->text();
    if(title == " Подключиться ") // если радио сеть выключена
    {
       // slotNetWorkingExch();  //  подключение радиоканала
    }
    else if (title == " Отключиться ") // если на момент обновления параметров радио сеть была запущена
    {
        slotNetWorkingExch();  //  отключаем радио сеть
    }
}
//------------------------------------------------------------------------------------
void Widget_Exch_IQ::startRadioWork()
{
    QString title = ui->pb_net->text();
    if(title == " Подключиться ") // если радио сеть выключена
    {
        slotNetWorkingExch();  //  подключение радиоканала
    }
    else if (title == " Отключиться ") // если на момент обновления параметров радио сеть была запущена
    {
        slotNetWorkingExch();  //  отключаем радио сеть
        slotNetWorkingExch();  //  включаем радио сеть с новыми параметрами
    }
}
//------------------------------------------------------------------------------------
void   Widget_Exch_IQ::slotZpchChanged(int val)
{
    emit sigZpchCh(val);
}
//------------------------------------------------------------------------------------
void    Widget_Exch_IQ::setSlotSize(int sz)
{
    m_slot_size = sz;
    // размер слота модема установлен по запросу sigGetSlotSize();
    // подключаем протокол радиоканала

    if(m_protocolRadio !=nullptr){
        delete  m_protocolRadio;
        m_protocolRadio = nullptr;
     }
     m_protocolRadio = new ProtocolRadio(m_net_corr, m_slot_size);

     connect(this, &Widget_Exch_IQ::sigZpchCh, m_protocolRadio,&ProtocolRadio::sigZpchChanged);
     connect (this, &Widget_Exch_IQ::sigDisconEx, m_protocolRadio, &ProtocolRadio::slotDiscon);
     connect (this, &Widget_Exch_IQ::sigBindPort, m_protocolRadio, &ProtocolRadio::sigBindPort);
     connect (m_protocolRadio, &ProtocolRadio::sigIQFromCorr, this, &Widget_Exch_IQ::slotIQFromExch);
     connect (m_protocolRadio, &ProtocolRadio::sigIQEnergy, this, &Widget_Exch_IQ::slotIQEnergy);
     connect (m_protocolRadio, &ProtocolRadio::sigErrLost, this, &Widget_Exch_IQ::slotErrLost);
     connect (m_protocolRadio, &ProtocolRadio::sigIQFromRadio, this, &Widget_Exch_IQ::slotDataToJournal);
     connect (m_protocolRadio, &ProtocolRadio::sigIQToRadio, this, &Widget_Exch_IQ::slotDataToJournal);
     connect (ui->ch_start_buff, SIGNAL(clicked(bool)), m_protocolRadio, SLOT(slotUseBuffSlot(bool))); // использование начальной буф.


    // ui->ch_start_buff->setChecked(true);
    // emit ui->ch_start_buff->clicked(true);

     emit ui->sp_ZPCH->valueChanged(0);
     ui->sp_ZPCH->setEnabled(true);     

      //ui->pb_net->setEnabled(false);
      ui->le_port_bind->setReadOnly(true);
      ui->le_port_cor->setReadOnly(true);
      ui->le_adr_cor->setReadOnly(true);

      emit sigBindPort(ui->le_port_bind->text().toInt());

}
//------------------------------------------------------------------------------------
int Widget_Exch_IQ:: getSlotSize()
{
    return m_slot_size;
}
//---------------------------------------------------------------------------------------
bool Widget_Exch_IQ::getNoConnect()
{
    return m_no_connect;
}
//------------------------------------------------------------------------------------
void    Widget_Exch_IQ::setBinding()                                          // соединение
{
  //  slotNetWorkingExch();
}
//------------------------------------------------------------------------------------
void    Widget_Exch_IQ::setDisconnect()                                       // разрыв соединения
{
    ui->le_port_bind->setText("");
    ui->le_port_cor->setText("");
    ui->le_adr_cor->setText("");
    ui->sp_ZPCH->setValue(0);
   // ui->ch_start_buff->setChecked(start_buf);

    if(m_protocolRadio !=nullptr){
       stopRadioWork();
    }
   // if(m_protocolRadio !=nullptr){
  //     slotDiscon();
   // }
}
//------------------------------------------------------------------------------------
// обработка кнопки Начать прослушку радио канала
void Widget_Exch_IQ::slotNetWorkingExch()
{
    QString title = ui->pb_net->text();

    if(title == " Подключиться ")
    {
        ui->pb_net->setText(" Отключиться ");
        slotNewBinding();      
    }
    else if (title == " Отключиться ")
    {
        ui->pb_net->setText(" Подключиться ");
        slotDiscon();
    }
}

//---------------------------------------------------------------------------------
void Widget_Exch_IQ::slotIQEnergy(qreal val)
{
    ui->le_energy->setText(QString::number(val,'g',2));
}
//---------------------------------------------------------------------------------
void Widget_Exch_IQ::slotErrLost(QString & err_journ)
{
    QString clear{""};
    ui->lb_err_lost->setText(clear);
    ui->lb_err_lost->setText(err_journ);

}
//---------------------------------------------------------------------------------
// кнопка "Нового соединение"/ "Начать прослушку"
void Widget_Exch_IQ::slotNewBinding()
{
   m_net_corr.port = ui->le_port_cor->text().toInt();
   m_net_corr.address = ui->le_adr_cor->text();
   m_no_connect=false;
   ui->ch_start_buff->setChecked(false);
   ui->ch_start_buff->setEnabled(false);
   emit sigGetSlotSize(); // запрос на получение размера слота модема

  /* if(m_protocolRadio !=nullptr){
      delete  m_protocolRadio;
      m_protocolRadio = nullptr;
   }
   m_protocolRadio = new ProtocolRadio(m_net_corr, m_slot_size);

   connect (this, &Widget_Exch_IQ::sigDisconEx, m_protocolRadio, &ProtocolRadio::slotDiscon);
   connect (this, &Widget_Exch_IQ::sigBindPort, m_protocolRadio, &ProtocolRadio::sigBindPort);
   connect (m_protocolRadio, &ProtocolRadio::sigIQFromCorr, this, &Widget_Exch_IQ::slotIQFromExch);
   connect (m_protocolRadio, &ProtocolRadio::sigIQEnergy, this, &Widget_Exch_IQ::slotIQEnergy);
   connect (m_protocolRadio, &ProtocolRadio::sigErrLost, this, &Widget_Exch_IQ::slotErrLost);
   connect (m_protocolRadio, &ProtocolRadio::sigIQFromRadio, this, &Widget_Exch_IQ::slotDataToJournal);
   connect (m_protocolRadio, &ProtocolRadio::sigIQToRadio, this, &Widget_Exch_IQ::slotDataToJournal);   

   connect (ui->ch_start_buff, SIGNAL(clicked(bool)), m_protocolRadio, SLOT(slotUseBuffSlot(bool))); // использование начальной буф.

   ui->ch_start_buff->setChecked(true);
   emit ui->ch_start_buff->clicked(true);

   m_protocolRadio->setFormatDataPass(m_16b); // установка формата передачи (16 байт по умолч.)

    //ui->pb_net->setEnabled(false);
    ui->le_port_bind->setReadOnly(true);
    ui->le_port_cor->setReadOnly(true);
    ui->le_adr_cor->setReadOnly(true);

    emit sigBindPort(ui->le_port_bind->text().toInt());
*/
}
//-------------------------------------------------------------------
void  Widget_Exch_IQ::slotDataToJournal(QByteArray & buf, int num_pack, int flag_pass)
// заполнение журнала отправленных/ принятых пакетов
{
    Q_UNUSED(buf);
    Q_UNUSED(num_pack);
    if (flag_pass == TX) { // передача
       // ui->te_info->append("\nСообщение корреспонденту отправлено "+ QTime::currentTime().toString("hh:mm:ss:zzz") + "\n");
    }
    else if (flag_pass == RX){   // прием
       // ui->te_info->append("\nСообщение принято от корреспондента "+ QTime::currentTime().toString("hh:mm:ss:zzz")+"\n");
    }
  //  ui->te_info->append("Пакет #" + QString::number(num_pack) + "\n");
    //ui->te_info->append(buf.toHex(' '));
}
//------------------------------------------------------------------
// коррекция буф. перед демодуляцией
 void Widget_Exch_IQ::slotBfShift(qint32 lf_sh, qint32 rt_sh)
 {
     if(m_protocolRadio !=nullptr){
         m_protocolRadio->setBufShift(lf_sh, rt_sh);
     }
 }
//------------------------------------------------------------------
// разрыв соединения
void Widget_Exch_IQ::slotDiscon()
{
    ui->ch_start_buff->setEnabled(false);
    emit sigDisconEx();   

    disconnect (this, &Widget_Exch_IQ::sigDisconEx, m_protocolRadio, &ProtocolRadio::slotDiscon);
    disconnect (this, &Widget_Exch_IQ::sigBindPort, m_protocolRadio, &ProtocolRadio::sigBindPort);
    disconnect (m_protocolRadio, &ProtocolRadio::sigIQFromCorr, this, &Widget_Exch_IQ::slotIQFromExch);
    disconnect (m_protocolRadio, &ProtocolRadio::sigIQEnergy, this, &Widget_Exch_IQ::slotIQEnergy);
    disconnect (m_protocolRadio, &ProtocolRadio::sigErrLost, this, &Widget_Exch_IQ::slotErrLost);
    disconnect (ui->ch_start_buff, SIGNAL(clicked(bool)), m_protocolRadio, SLOT(slotUseBuffSlot(bool))); // использование начальной буф.
    disconnect (m_protocolRadio, &ProtocolRadio::sigIQFromRadio, this, &Widget_Exch_IQ::slotDataToJournal);
    disconnect (m_protocolRadio, &ProtocolRadio::sigIQToRadio, this, &Widget_Exch_IQ::slotDataToJournal);
    disconnect(ui->sp_ZPCH, SIGNAL(valueChanged(int)), m_protocolRadio,SLOT(sigZpchChanged(int)));

    ui->le_port_bind->setReadOnly(false);
    ui->le_port_cor->setReadOnly(false);
    ui->le_adr_cor->setReadOnly(false);
}
//-------------------------------------------------------------------
// пришедшее сообщение об ошибке из модуля connector
void Widget_Exch_IQ::slotErrorFromConnectEx( QString & str, QTime tm)
{
    Q_UNUSED(tm);

    // произошла ошибка при соединении с сервером - связь не установлена

   if(str=="error"){
       // соединение не установлено
     ui->pb_net->setEnabled(true);
     ui->pb_net->setText(" Подключиться ");
     ui->le_port_bind->setReadOnly(false);
     ui->le_port_cor->setReadOnly(false);
     ui->le_adr_cor->setReadOnly(false);

     disconnect (this, &Widget_Exch_IQ::sigDisconEx, m_protocolRadio, &ProtocolRadio::slotDiscon);
     disconnect (this, &Widget_Exch_IQ::sigBindPort, m_protocolRadio, &ProtocolRadio::sigBindPort);
     disconnect (m_protocolRadio, &ProtocolRadio::sigIQFromCorr, this, &Widget_Exch_IQ::slotIQFromExch);
     disconnect (m_protocolRadio, &ProtocolRadio::sigIQEnergy, this, &Widget_Exch_IQ::slotIQEnergy);
     disconnect (m_protocolRadio, &ProtocolRadio::sigErrLost, this, &Widget_Exch_IQ::slotErrLost);
     disconnect (ui->ch_start_buff, SIGNAL(clicked(bool)), m_protocolRadio, SLOT(slotUseBuffSlot(bool))); // использование начальной буф.
     disconnect (m_protocolRadio, &ProtocolRadio::sigIQFromRadio, this, &Widget_Exch_IQ::slotDataToJournal);
     disconnect (m_protocolRadio, &ProtocolRadio::sigIQToRadio, this, &Widget_Exch_IQ::slotDataToJournal);
     disconnect (ui->sp_ZPCH, SIGNAL(valueChanged(int)), m_protocolRadio,SLOT(sigZpchChanged(int)));
     m_no_connect = true;
   }
   else if(str =="ok"){
       // соединение установлено

       ui->pb_net->setText(" Отключиться ");
       ui->pb_net->setEnabled(true);
       ui->le_port_bind->setReadOnly(true);
       ui->le_port_cor->setReadOnly(true);
       ui->le_adr_cor->setReadOnly(true);
       m_no_connect=false;
       // определение ip адреса машины на которой запущено приложение

     //  const QString portNum = QString::number(m_protocolDTE->getPortNumber());
    //   QString nameApp = windowTitle();
    //   setWindowTitle(nameApp +" port[ " + portNum+" ]");
   }
}

//-------------------------------------------------------------------------
//  раскодирование сообщения поля data в команде ExchangeDCE перед отправкой в СО
/*void Widget_Exch_DTE::decodeDataKey(QJsonObject & obj)
{
    QJsonValue objVal = obj.value("data");
// json_obj.insert("data", m_test_ba.toBase64().data());
    QByteArray bb, ba;
    bb.append(objVal.toString().toUtf8());          // зашифрованные данные в байтах помещаем в байтовый массив
    ba.append(QByteArray::fromBase64(bb.data()));   // расшифрованные данные
    obj.insert("data", QString(ba.toHex()));        //помещаем данные в пакет в разакодированном виде

}
*/
//--------------------------------------------------------------------------
/*bool Widget_Exch_IQ::getForUsEx(const QByteArray & ba)
{
    return true;//((m_uvs_obj.DCE_ID == getIDCom(ba)) && (getIDCom(ba)!=getIDOppnCom(ba))) ;
}
*/
//------------------------------------------------------------------
/*uint32_t Widget_Exch_DTE::getIDCom(const QByteArray & ba)
{
    QJsonDocument   doc_in;         // json док. для загрузке в него принятой послед.
    QJsonParseError parseErr_in;    // ошибки возникающие при парсинге принятой послед. в json док.
    QJsonObject     jObj_in;
    QString         err;

    if(ba.count() > 0){  // принятое сообщение не пустое
        doc_in = QJsonDocument::fromJson(ba, & parseErr_in);
            // если есть ошибки JSON - выдаем сообщение
        if (parseErr_in.error != QJsonParseError::NoError){
            err = parseErr_in.errorString();//"JSON-ошибка";
         //   QMessageBox::critical(0, "Сообщение","В сообщении JSON ошибка! ("+QString(ba)+")" ,"");

        }
        // если json последовательности ошибок нет ->
        else if(doc_in.isObject()){
            jObj_in = doc_in.object(); // формируем из сообщения json объект
        }
    }
    return jObj_in.value("DCE_ID").toInt();
}
*/
/*//------------------------------------------------------------------
uint32_t Widget_Exch_DTE::getIDOppnCom(const QByteArray & ba)
{
    QJsonDocument   doc_in;         // json док. для загрузке в него принятой послед.
    QJsonParseError parseErr_in;    // ошибки возникающие при парсинге принятой послед. в json док.
    QJsonObject     jObj_in;
    QString         err;

    if(ba.count() > 0){  // принятое сообщение не пустое
        doc_in = QJsonDocument::fromJson(ba, & parseErr_in);
            // если есть ошибки JSON - выдаем сообщение
        if (parseErr_in.error != QJsonParseError::NoError){
            err = parseErr_in.errorString();//"JSON-ошибка";
         //   QMessageBox::critical(0, "Сообщение","В сообщении JSON ошибка! ("+QString(ba)+")" ,"");

        }
        // если json последовательности ошибок нет ->
        else if(doc_in.isObject()){
            jObj_in = doc_in.object(); // формируем из сообщения json объект
        }
    }
    return jObj_in.value("DCE_ID_OPPN").toInt();
}
*/
//---------------------------------------------------------------------------
// отправка сообщения полученного от СО на ПАК АУ(ПАК МАС) со статусом recv (оригинальное в случ. ПАК МАС)(synchr, connect, discon, exchenge)
void Widget_Exch_IQ::slotIQFromExch(QVector<int_complex> * signal)
{
    emit sigCodoFromCorr(signal);
}
//--------------------------------------------------------------------------------
// данные пришли из CoreDCE для отправки в радио канал (ProtocolRadio)
void Widget_Exch_IQ::slotIQToExch(SlotFieldS & slot)
{
    if(m_protocolRadio!=nullptr)
       m_protocolRadio->sendIQToCorr(slot);
}
//----------------------------------------------------------------------------------
// запрос у модема размера слота
void Widget_Exch_IQ::slotGetModemSlotLen()
{

}
//----------------------------------------------------------------------------------


