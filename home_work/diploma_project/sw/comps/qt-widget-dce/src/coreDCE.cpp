// модуль реализующий логику протокола взаимодействия ПАК УВС с ПАК МАС, ПАК АУ

#include "coreDCE.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QtXml>
#include <QTimer>

//-------------------------------------------------------------------
CoreDCE::CoreDCE(QWidget *parent/*NetS & net_adr,TypeMsgEnum type_msg, bool format_msg*/)
{
    Q_UNUSED(parent);
    qRegisterMetaType<SlotFieldS>();

    m_slot_timer = new QTimer(this);
    m_slot_timer->setTimerType(Qt::PreciseTimer);

    connect(m_slot_timer, SIGNAL(timeout()),this, SLOT(slotPassTimerAlarm()));

    connect(this, &CoreDCE::sigSlotNumPass, this, &CoreDCE::slotSlotNumPass);

  //  set_turnOn_State(true); // вкл. запуск таймера
  //  setModemNumber(1);      // установка номера
  //  setChannelLevel(true);
   // setPanelVariant(4);     // установка 5 режима (режим должен меняться по команде EnableDCE)

    QString log_name1{"journal_modem_before.log"};
    QString main_title1{"NUM Time_REC Module_Use TX/RX Time_slot_TX/RX signal "};

    QString log_name2{"journal_modem_after.log"};
    QString main_title2{"NUM Time_REC Module_Use TX/RX Time_slot_TX/RX demod_slot_status "};

    m_log_maker_b = new LogMaker(log_name1, true, main_title1);
    m_log_maker_b->logOpen();

    m_log_maker_a = new LogMaker(log_name2, true, main_title2);
    m_log_maker_a->logOpen();
}
//---------------------------------------------------------------------------
 CoreDCE:: ~CoreDCE()
 {
     m_slot_timer->stop();
     logFileClose();

     m_log_maker_b->logClose();
     delete m_log_maker_b;
     m_log_maker_a->logClose();
     delete m_log_maker_a;
 }
 //--------------------------------------------------------------------------
 void CoreDCE::setRatePass(int rate)
 {
     m_rate = rate;
 }

 //--------------------------------------------------------------------------
 void CoreDCE::setQueueDepth(int depth)
 {
     m_queue_depth = depth;
 }
 //--------------------------------------------------------------------------
 void CoreDCE::slotUseDST(bool use)
 {
     m_use_dst = use;
 }
 //-----------------------------------------------------------------------
 // длина сообщения в разкодированном виде
 qint32 CoreDCE::getMsgSize(QString &data_field)
 {
     quint32 sz{0};
     QByteArray ba, ba_;
     ba.append(data_field.toUtf8());
     ba_.append(QByteArray::fromBase64(ba.data())); // декодируем сообщение
     sz = ba_.count();                               // подсчитываем кол-во байт в нем

     return sz;
 } 
 //-----------------------------------------------------------------------
 void CoreDCE::slotTempExchDCE(MsgDescrS & msg)
 {
    if (msg.obj_msg_in.contains("data"))
       {
        if(!m_slot_timer->isActive()){
           m_slot_timer->start(m_time_slot_pass);  // запустим таймер отправки слота если еще не запускали
         //  logFileOpen("log_mod200.txt");
         }
        m_queue_msg.enqueue(msg);                 // помещаем в очередь пришедщее от клиента сообщение

       // QString data_str{msg.obj_msg_in.value("data").toString()};
      //  m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue + getMsgSize(data_str);

        emit sigEnqueueAnsw(msg, m_queue_msg.count());
        //prepareEnqueuedAnsw(msg);                 // постановка в очередь прищедшего сообщения, выдача сообщения :enqueued клиенту
        if((m_queue_msg.count() >= m_queue_depth) && m_msg_pass){ // проверяем глубину очереди и состояние отправляемого объекта
            queueMsgDequeue();                    // опусташаем очередь сообщений
         }
     }
 }
 //----------------------------------------------------------------------------
 // опустошение очереди сообщений
 void CoreDCE::queueMsgDequeue()
 {
    QJsonObject jObj_out;    

    if(m_queue_msg.count() > 0){
        m_current_msg = m_queue_msg.dequeue();   // выбираем объект из очереди предназначенный для отправки

      //  QString data_str{m_current_msg.obj_msg_in.value("data").toString()};
      //  m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue - getMsgSize(data_str) ;
      //  prepareStartAnsw(m_current_msg);         // подгот. и отправляем оствет :start клиенту
        emit sigStartAnsw(m_current_msg);
      //  qDebug()<<"Modul_mess: "<< data_str;
        QByteArray ba;
        ba = decodingData(m_current_msg.obj_msg_in); // декодируем пришедшее сообщение для передачи в модулятор
        m_msg_pass = false;                    // текущее сообщение не отправлено
        if(isActive){    // проверка, запущен ли модулятор
           int slotLen = getSlotMsgLen();         // запрашиваем у модулятора размер слота (в зависимости от режима работы)

        // алгоритм создания сообщения для демодулятора из нестольких коротких сообщений клиента
           while (ba.count() < slotLen){                     // если длинна сообщения меньше длинны слота
             if(m_queue_msg.count() > 0){        // и если в очереди сообщений есть еще сообщения
              m_current_msg = m_queue_msg.dequeue();   // выбираем объект из очереди предназначенный для отправки
             // QString data_str{m_current_msg.obj_msg_in.value("data").toString()};
             // m_uvs_obj.bytesInQue = m_uvs_obj.bytesInQue - getMsgSize(data_str) ;
             // prepareStartAnsw(m_current_msg);         // подгот. и отправляем оствет :start клиенту
              emit sigStartAnsw(m_current_msg);
              QByteArray ba_add;
              ba_add = decodingData(m_current_msg.obj_msg_in); // декодируем пришедшее сообщение для передачи в модулятор
              m_msg_pass = false;                    // текущее сообщение не отправлено
              ba = ba + ba_add;                     // соединяем сообщения вместе
            }
            else
                break;
        }
        modulatorStart(ba);                  // запуск процедуры модуляции и отправки в радиоканала сообщения
     }
   }
  /*  else{     // если очередь сообщений пуста
        QByteArray ba;  // отправим пустое сообщение в модулятор
        ba.append(0x01);
      //  ba = decodingData(m_current_msg.obj_msg_in); // декодируем пришедшее сообщение для передачи в модулятор
        m_msg_pass = false;                  // текущее сообщение не отправлено
        modulatorStart(ba);                  // запуск процедуры модуляции и отправки в радиоканала сообщения
    }
    */
 }
 //--------------------------------------------------------------------------
 // модуляция сообщения и считывание модулированных слотов из модулятора
 void CoreDCE::modulatorStart(QByteArray & ba)
 {
    int slotLen = getSlotMsgLen();         // запрашиваем у модулятора размер слота (в зависимости от режима работы)
    int k = ba.length() % slotLen;         // остаток
    m_slot_count = ba.length() / slotLen;  // число слотов для модулирования кратно значению слота сообщения slotLen
    if(k)                                  // если остаток от деления на размер слота > 0
       m_slot_count++;                     // кол-во слотов в сообщении для модуляции
    m_queue_slot_count.enqueue(m_slot_count); // помещаем в очередь число слотов в сообщении
    send_data(ba.data(), ba.length());     // загрузка сообщение в модулятор целиком
   // qDebug()<<"Modul_ba: "<< ba;

    for(int i = 0; i < m_slot_count; ++i){
       modulate();                          // вызов функции модуляции (происходит m_slot_count раз)
       SlotFieldS slot_obj;           // слот после модулятра
       slot_obj.signal = new short_complex[getSlotLength()];    // удаляется в ProtocolRadio::makePacksFromSlot(SlotFieldS & slot)
       // считываем из модулятора модулированный слот
       memcpy(slot_obj.signal, getOutput(), getSlotLength() * sizeof(short_complex));

       slot_obj.signal_len = getSlotLength();// его длинну
       slot_obj.freq_num = chanNumber_send();// номер частоты для передачи этого слота

       m_queue_slot.enqueue(slot_obj);       // помещаем модул. слот в очередь слотов для отправки
    }
 }
 //----------------------------------------------------------------------------
 void CoreDCE::slotSlotNumPass(qint32 num)
 {
     if (num == m_queue_slot_count.first()){ //отправлено столько слотов сколько записано в последнем элементе очереди
         m_curr_slot_num = 0;               // счетчик отправл. слотов
         m_queue_slot_count.dequeue();      // удаляем элемент из очереди
         m_msg_pass = true;                 // текущее сообщение отправлено      

         emit sigEndAnsw(m_current_msg, m_queue_msg.count());
         //prepareEndAnsw(m_current_msg);     // сообщаем что текущее сообщение отправлено
         queueMsgDequeue();                 // переходим к загрузке в модулятор след. сообщения в очереди
     }
 }
 //----------------------------------------------------------------------------
 // опустошение очереди слотов
 void CoreDCE::queueSlotDequeue()
 {
    QJsonObject jObj_out;
    if(m_queue_slot.count() > 0){
        SlotFieldS current_slot = m_queue_slot.dequeue();   // выбираем объект из очереди предназначенный для отправки
        emit sigIQSignalReady(current_slot);                // сигнал о передаче слота на трансп. уровень для разбиения на пакеты и отправки по UDP
                                                            // sendIQToCorr(SlotFieldS & current_slot) ProtocolRadio
        ++m_curr_slot_num;                     // счетчик передачи слотов
        emit sigSlotNumPass(m_curr_slot_num);  // для контроля отправки сообщения состоящего из нескольких слотов       
    }
    else{
        queueMsgDequeue();
    }   
 }

 //--------------------------------------------------------------------------
 // обработка таймера отправки слота в радиоканал (Сервер обмена)
 void CoreDCE::slotPassTimerAlarm()
 {
     queueSlotDequeue();  // опустошение очереди слотов при отправки слотов в сеть
 }
 //-----------------------------------------------------------------------------------------------
 bool CoreDCE::removeLog_time(const QString fl_name)
 {
     QString fl_path;
     bool res{true};
     QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
     QString LOG_PATH  = "/LOG/";               // дир. с логами.
     QString log_dir = APP_ABS_PATH + LOG_PATH;
     QString file_name = fl_name; //"log_mod_demod.txt";
     fl_path = APP_ABS_PATH + LOG_PATH + file_name;
     QFile fl_log(fl_path);
     if(fl_log.exists()){
         res = fl_log.remove();
     }
     return res;
 }
 //-----------------------------------------------------------------------------------------------
 void  CoreDCE::logFileOpen(QString file_name)
 {
     QString fl_path;
     QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
     QString LOG_PATH  = "/LOG/";               // дир. с логами.
     QString log_dir = APP_ABS_PATH + LOG_PATH;

     fl_path = APP_ABS_PATH + LOG_PATH + file_name;

     m_log_file.setFileName(fl_path);

     if(m_log_file.exists()){
        if (m_log_file.open((QIODevice::Append)|(QIODevice::Text)|(QIODevice::WriteOnly))){

         }
     }
     if (!m_log_file.exists()){
         if(!m_log_file.open(QIODevice::WriteOnly)/*|(QIODevice::Append)*/){

         }
     }
 }
 //-----------------------------------------------------------------------------------------------
 void CoreDCE::logFileClose()
 {
     if (m_log_file.isOpen()){
         m_log_file.close();
     }
 }
 //-----------------------------------------------------------------------------------------------
 void CoreDCE::logFileWrite(int num, QString &title)
 {
     if (m_log_file.isOpen()){
     //if (m_log_file.open((QIODevice::Append)|(QIODevice::Text)|(QIODevice::WriteOnly))){
         QTextStream logStream(&m_log_file);

         logStream <<title<<QString::number(num)<< " " <<QDateTime::currentDateTime().currentMSecsSinceEpoch();//QTime::currentTime().toString("ss.zzz");
         logStream <<"\n";
      }
 }
 //-----------------------------------------------------------------------------------------------
 void CoreDCE::createLog_time(const QString fl_name, int num, QString &title)
 {
     QString fl_path;
     QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
     QString LOG_PATH  = "/LOG/";               // дир. с логами.
     QString log_dir = APP_ABS_PATH + LOG_PATH;
     QString file_name = fl_name;//"log_mod_demod.txt";

     fl_path = APP_ABS_PATH + LOG_PATH + file_name;
     QDateTime cdt = QDateTime::currentDateTime();

     QFile fl_log(fl_path);
     if(fl_log.exists()){
        if (fl_log.open((QIODevice::Append)|(QIODevice::Text)|(QIODevice::WriteOnly))){
            QTextStream logStream(&fl_log);

            logStream <<title<<QString::number(num)<< " " <<QDateTime::currentDateTime().currentMSecsSinceEpoch();//QTime::currentTime().toString("ss.zzz");
            logStream <<"\n";
             fl_log.close();
         }
     }
     if (!fl_log.exists()){
         if(!fl_log.open(QIODevice::WriteOnly)/*|(QIODevice::Append)*/){

         }
         else{
             QTextStream logStream(&fl_log);

             logStream <<title<<QString::number(num)<<" " <<QDateTime::currentDateTime().currentMSecsSinceEpoch();//QTime::currentTime().toString("ss.zzz") << "\t";
             logStream <<"\n";
             fl_log.close();
         }
     }
 }
 //---------------------------------------------------------------------------
 void CoreDCE::addToLog_b(int k,  QVector<int_complex> *signal, QDateTime tm, LogUseEnum flag_use)
 {
     QString data;

     ++m_count_b;

     if(k)
       data = " TX";
     else
       data = " RX";

    data = data + " "+ QString::number(tm.currentMSecsSinceEpoch());//tm.toString("hh:mm:ss.zz");
    data = data + " ";
    for (int i =0; i < signal->count(); ++i){
       data = data +"(" +QString::number(signal->at(i).real()) +":"+ QString::number(signal->at(i).imag())+")";
    }

    m_log_maker_b->logWrite(m_count_b, flag_use, data);
 }
//----------------------------------------------------------------------------
 void CoreDCE::addToLog_a(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use)
 {
     QString data;

     ++m_count_a;

     if(k)
       data = " TX";
     else
       data = " RX";

    data = data + " "+ QString::number(tm.currentMSecsSinceEpoch());//tm.toString("hh:mm:ss.zz");
    data = data + " ";

    data = data + ba.toHex(':');
    m_log_maker_a->logWrite(m_count_a, flag_use, data);
 }
//----------------------------------------------------------------------------
 //данные пришли от корреспондента по радио каналу
 void CoreDCE::slotIQFromCorr(QVector<int_complex> *signal)
 {
    QByteArray data_corr{};
  //  QByteArray data_sig{};
    QByteArray data_demod{}; // демодулированное сообщение
    int rs{0};
    int bf{0};
    int num{0};

 //  if(m_slot_timer->isActive())
 //     m_slot_timer->stop();    // работа модуля в режиме демодул. исключает модуляцию

   // data_sig.append("signal_"+QString::number(signal->count()));
    QDateTime tm = QDateTime::currentDateTime();
    addToLog_b(0,  signal, tm, MODEM_BEFORE);            // добавляем запись в лог
   if(isActive){   // если модем запущен
    if(getChanExit(signal))
      {
       qreal energy{0.0};
        for(int i = 0; i < 2 * getSlotLength(); ++i )
        {
         energy = energy + qSqrt(qPow(signal->at(i).real(),2) + qPow(signal->at(i).imag(),2));
        }
        energy = energy/(2.0 * getSlotLength());
        emit sigIQEnergy(energy);

        demodulate(true);
        data_corr = *slotExitData(&rs, &bf);

        signal=nullptr;
        delete signal;

        data_demod.append(data_corr);
        if(data_demod.count()< 1)
            data_demod.append("no");
        QDateTime tm = QDateTime::currentDateTime();
        addToLog_a(0,  data_demod, tm, MODEM_AFTER);            // добавляем запись в лог

      //  qDebug()<<"Demodul_ba: "<< data_corr;
        if(!m_flag_bf_use) {
            m_flag_bf_use = true;  // больше BF за сеанс работы ПО корректировать не будем
           // if(bf <= BF_LIMIT_MIN + BF_DELTA){
            if(bf >= BF_GOOD + BF_DELTA){
               m_flag_shift_right = 0;   // начало двойного буф. для демод. необх сдвинуть вправо-->
               m_flag_shift_left = bf - BF_GOOD;
            }
            else if(bf <= BF_GOOD - BF_DELTA){
                m_flag_bf_use = true;  // больше BF за сеанс работы ПО корректировать не будем
                m_flag_shift_right = BF_GOOD - bf;
                m_flag_shift_left = 0;    // начало двойного буф. для демод. необх сдвинуть влево<--
            }
            emit sigBfShift(m_flag_shift_left, m_flag_shift_right);  // сообщаем о необходимости корректировать bf
        }        
        else{     // сдвиг буф. для демод. не нужен
            m_flag_bf_use = true;  // больше BF за сеанс работы ПО корректировать не будем
            m_flag_shift_right = false;
            m_flag_shift_left = false;
        }

        num = data_corr.count();
     }

   // ++m_uvs_obj.msg_id;
  //  m_uvs_obj.type = "Raw";
    if(data_corr.count()>0)  // если после демодул. имеется к.л. инф.сообщение,тогда формир. сообщ ExchangeDCE
        emit sigExchDCEAnsw(data_corr);
      // cmdExchangeDCEAnsw(data_corr);   // для формирования иниц. соообщени ExchangeDCE
   }
     // декодирование сигнала
     // анализ полученного декод. сигнала
     // превращение его в ответную команду клиенту (АУ или МАС)
     // не забыть закодировать в BASE64 перед помещением в команду для клиента
 }
 //------------------------------------------------------------------------
 QByteArray CoreDCE::decodingData(QJsonObject json_in)
 {
     QByteArray bb, ba;
     QJsonValue objVal;

     if(json_in.contains("data")){
       objVal = json_in.value("data");
       bb.append(objVal.toString().toUtf8());       // зашифрованные данные в байтах помещаем в байтовый массив
      // ba.append(bb.data());
       ba.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
      }
     return ba;
 }
 //-----------------------------------------------------------------------
 int CoreDCE::decodingDataLength(QJsonValue val)
 {
     int len;
     QByteArray bb, ba;

     bb.append(val.toString().toUtf8());       // зашифрованные данные в байтах помещаем в байтовый массив
     ba.append(QByteArray::fromBase64(bb.data()));// декодируем принятые байты и помещаем в байтовый массив
     len = ba.length();
     return len;
 }
  //-------------------------------------------------------------------
 void CoreDCE::slotDisconnect()
 {

 }  
  //------------------------------------------------------------------


