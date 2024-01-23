#include "protocolRadio.h"
#include <QApplication>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDomDocument>
#include <QtXml>
#include <cmath>

//-----------------------------------------------------------------------------------------------
ProtocolRadio::ProtocolRadio(NetS & net_adrs, int slot_size):
               m_net_adr(net_adrs), m_slot_size(slot_size)
{
    qRegisterMetaType<NetS>();
    qRegisterMetaType<PackFieldS>();

    m_timer_pack = new QTimer(this);
    m_timer_pack->setTimerType(Qt::PreciseTimer);
    connect(m_timer_pack, SIGNAL(timeout()), this, SLOT(slotPassTimerAlarm()));

    m_transportUDP = new TransportUDP();
    m_transportUDP->setCorrAdr(m_net_adr);   // передаем адрес корреспонд. сокета
  //  connect (m_transportUDP, &TransportUDP::sigReadyRead, this, &ProtocolRadio::slotGetCodoFromCorr);
    connect (this, &ProtocolRadio::sigBindPort, m_transportUDP, &TransportUDP::slotBindingPort);
    connect (this, &ProtocolRadio::sigStopServOrDiscon, m_transportUDP, &TransportUDP::slotDisconnect);
    connect (this, &ProtocolRadio::sigZpchChanged, m_transportUDP, &TransportUDP::slotZpchChanged);
    connect (m_transportUDP, &TransportUDP::sigIQFromRadio, this, &ProtocolRadio::slotGetIQFromCorr);
    connect (m_transportUDP, &TransportUDP::sigIQFromRadio, this, &ProtocolRadio::sigIQFromRadio);
    connect (m_transportUDP, &TransportUDP::sigIQToRadio, this, &ProtocolRadio::sigIQToRadio);
    connect (m_transportUDP, &TransportUDP::sigErrLost, this, &ProtocolRadio::sigErrLost);

    //removeLog_time("log_pack10.txt");       // удалим лог для перезаписи
    nullingCBuff(m_cbuff, m_slot_size/2);        // заполняем нулями первую часть цикл. буф.

 }
//-------------------------------------------------------------------------------------------------
ProtocolRadio::~ProtocolRadio()
{
}
//-------------------------------------------------------------------------------------------------
// обработчик сигнала из MW УВС об отключении клиента
 void ProtocolRadio::slotDiscon()
 {
     emit sigStopServOrDiscon();  // сигнал о завершении прослушки в транспорт
 }

 //-----------------------------------------------------------------------------------------------
 void ProtocolRadio::nullingCBuff(CQueue<BuffVal> &buff, int num)
 {
     for(int j = 0; j < num; ++j){  // заполняем нулями
         int re = 0;
         int im = 0;
         int_complex val;
         val.real(re);
         val.imag(im);

         BuffVal buf_val;
         buf_val.seq_num = 0;
         buf_val.sig = 10 * val;
         buff.enqueue(buf_val);
         ++ m_point_num;
     }
 }
 //-----------------------------------------------------------------------------------------------
 void ProtocolRadio::denullingCBuff(CQueue<BuffVal> &buff, int num)
 {
     for(int j = 0; j < num; ++j){  // убираем нули из хвостика буф.
         buff.removeLast();
         --m_point_num;
     }
 }
 //-----------------------------------------------------------------------------------------------
 // добавление нулей в массив для увеличения его размера
 void ProtocolRadio::nullingIQarray(QVector<int_complex> &null_data, int num)
 {
     for(int j = 0; j < num; ++j){  // заполняем нулями
         int re = 0;
         int im = 0;
         int_complex val;
         val.real(re);
         val.imag(im);
         null_data.append(val);
     }
 }

 //-----------------------------------------------------------------------
 // Конвертация 24-х разрядного массива в 32-х разрядный
 quint16 ProtocolRadio::conversion24to32(char* pBuff24, char* pBuff32, qint64 sBuff24, qint16 gain)
 {
     // Определить размер в байтах массива 32-х битных отсчётов
     qint64 sBuff32 = (sBuff24 / Q24_SIZE) * Q32_SIZE;

     // Определение указателей на входной и выходной массивы
     qint16* pInp24 = (qint16*)(pBuff24 + sBuff24 - Q24_SIZE) + 2;
     qint32* pOut32 = (qint32*)(pBuff32 + sBuff32 - Q32_SIZE) + 1;

     // Нормировать коэффициент усиления I/Q составлюящих
     gain = 8 - gain;

     // Цикл переупаковки входного массива
     for (int i=0; i < (sBuff24 / Q24_SIZE); ++i)
     {
         // Считать коэффициенты
         qint16 coeff24IQ = *pInp24--;
         qint16 coeff24QH = *pInp24--;
         qint16 coeff24IH = *pInp24--;

         // Квадратурный канал (Q)
         *pOut32-- = ((coeff24QH << 16) | (LOBYTE(coeff24IQ) << 8)) >> gain;

         // Синфазный канал (I)
         *pOut32-- = ((coeff24IH << 16) | (HIBYTE(coeff24IQ) << 8)) >> gain;
     };

     // Возвращение размера 32-х разрядного массива
     return sBuff32;
 }
 //---------------------------------------------------------------------------
 // Конвертирование 16-и разрядного массива в 24-х разрядный
 qint64 ProtocolRadio::conversion16to24(char* pInp, char* pOut, qint64 sizeByte)
 {

     // Определить локальные указатели на входной и выходной массивы
     qint16* pData16 = reinterpret_cast<qint16 *>(pInp);
     qint16* pData24 = reinterpret_cast<qint16 *>(pOut);

     // Цикл переупаковки отсчётов
     for(int i = 0; i < (sizeByte >> 2); i++)
     {
         // Считать 16-ти разрядные данные из входного массива
         qint32 coeffI  = *pData16++;
         qint32 coeffQ  = *pData16++;
         //
         //// Упаковать данные в массив для 24-х разрядных отсчётов
          *pData24++ = (coeffI >> 8);
          *pData24++ = (coeffQ >> 8);
          *pData24++ = (((coeffI & 0xFF) << 8) | (coeffQ & 0xFF));

     }

     // Вернуть скорректированный размер выходного массива
     return (sizeByte * 3) >> 1;
 }

 //--------------------------------------------------------------------------------
 // Конвертация 16-х разрядного массива в 32-х разрядный
/* quint16 ProtocolRadio::conversion16to32(char* pBuff16, char* pBuff32, qint64 sBuff16)
 {
     // Определение указателей на входной и выходной массивы
     qint32* pOut = reinterpret_cast<qint32*>(pBuff32);
     qint16* pInp = reinterpret_cast<qint16*>(pBuff16);

     // Цикл переупаковки входного массива
     for(int i = 0; i < (sBuff16 >> 1); i++)
     {
         //qint16 var16;
         //qint32 var32;
         //var16 = *pInp++;
         //var32 = var16 << 8;
         // *pOut++ = var32;

         *pOut++ = (*pInp++ << 8);

         // ОТЛАДКА //////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
         //if(i < 10 ) qDebug() << "CCommon::conversion16to32(). var16 =" << var16 << " var32 =" << var32;
         /////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
     };

     // Возвращение размера 32-х разрядного массива
     return sBuff16 << 1;
 } 
 */
 //-----------------------------------------------------------------------------------------------
 void ProtocolRadio::createLog(CQueue<BuffVal> &buff, int k, int num)
 {
     QString fl_path;
     QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
     QString LOG_PATH  = "/LOG/";               // дир. с файлами конфигур.
     QString log_dir = APP_ABS_PATH + LOG_PATH;
     QString fl_name = "log_demodul.txt";

     fl_path = APP_ABS_PATH + LOG_PATH + fl_name;
     QDateTime cdt = QDateTime::currentDateTime();

     QFile fl_log(fl_path);
     if(fl_log.exists()){
        if (fl_log.open((QIODevice::Append)|(QIODevice::Text)|(QIODevice::ReadOnly))){
            QTextStream logStream(&fl_log);
            //logStream << cdt.toLocalTime().toString("yy_MM_dd_hh_mm")<<"\n";
            logStream <<"Seq_num: ";
           for(int i = k; i < num; i=i+200 )
             {
             // logStream <<"Seq_num:"<< buff.at(i).seq_num << ": ("<<buff.at(i).sig.real()<<","<< buff.at(i).sig.imag()<<")"<<"\n";
                logStream << buff.at(i).seq_num << "\t";
           }
           logStream <<"\n";
           fl_log.close();
         }
     }
 }
//-----------------------------------------------------------------------------------------------
  void ProtocolRadio::setBufShift(qint32 lf, qint32 rt)
  // установка полей m_shift_bf_left, m_shift_bf_right
  {
      m_shift_bf_left = lf;
      m_shift_bf_right = rt;
  }
 //----------------------------------------------------------------------------------------------
  //       ПРМ
  void ProtocolRadio::slotGetIQFromCorr(QByteArray & dataIQ,    // данные принятого пакета
                                               int   num_pack) // номер принятого пакета
  {      
      QByteArray arr32;                         // массив перобразованных данных из пакета
      qint64     sBuff32 =(dataIQ.count() / Q24_SIZE) * Q32_SIZE;   // размер преобразов. массива
      arr32.resize(sBuff32);

      qint16 gain = 0;
      //quint16 sz32 = conversion16to32(dataIQ.data(), arr32.data(), dataIQ.count());
      quint16 sz32 = conversion24to32(dataIQ.data(), arr32.data(), dataIQ.count(), gain);
      sz32=0;
      qint32* p = (qint32*) arr32.data();
      int l = sBuff32/sizeof(qint32); //sz32/sizeof(qint32);       // число элем. в преоб. массиве

      for (int i = 0; i < l; i = i + 2) {  // данные из принятого пакета заносятся в циклический буф.
          int_complex val;
          val.real(p[i]);
          val.imag(p[i+1]);
          BuffVal buf_val;
          buf_val.seq_num = num_pack;
          buf_val.sig = 10 * val;
          m_cbuff.enqueue(buf_val);
          ++ m_point_num;                 // ведем подсчет элементов, помещенных в цикл. буф.
      }

      if(m_point_num >= (2 * m_slot_size) - m_slot_size/2){  // если в цикл. буф. в текущий момент находится кол-во точек >= 2* размер слота
          nullingCBuff(m_cbuff, m_slot_size/2);        // заполняем нулями часть цикл. буф.
         m_point_num = m_point_num - m_slot_size;    // значение счетчика точек уменьшаем на размер слота
         m_signal_IQ = new QVector<int_complex>;     // двойной буф.IQ для декодирования

         for(int i = 0; i < 2 * m_slot_size; ++i){ // заполняем из циклического буф данными в размере 2-x слотов
             m_signal_IQ->append(m_cbuff.at(i).sig);
          }

         for(int j = 0; j < m_slot_size; ++j ){   // удаляем данные из цикл. буф. в размере 1-го слота
             m_cbuff.dequeue();
          }

         denullingCBuff(m_cbuff, m_slot_size/2);    // очищаем от нулей правую часть цикл. буф.
         // наш цикл. буф. готов для принятия след. слота

        emit sigIQFromCorr(m_signal_IQ);       // передача данных двойного буф. из радио канал далее в CoreDCE(модем) для декодирования
    }                                        // дин. выделение памяти m_signal_IQ освобождается после демодуляции в модулеCoreDCE
  }

 //-----------------------------------------------------------------------------------------------
 //       ПРМ  
 /*void ProtocolRadio::slotGetIQFromCorr(QByteArray & dataIQ,    // данные принятого пакета
                                              int   num_pack) // номер принятого пакета
 {     
     QByteArray arr32;                         // массив перобразованных данных из пакета

     qint64 sBuff32 =(dataIQ.count() / Q24_SIZE) * Q32_SIZE;   // размер преобразов. массива    
     arr32.resize(sBuff32);
     qint16 gain = 0;
     //quint16 sz32 = conversion16to32(dataIQ.data(), arr32.data(), dataIQ.count());
     quint16 sz32 = conversion24to32(dataIQ.data(), arr32.data(), dataIQ.count(), gain);
     qint32* p = (qint32*) arr32.data();
     int l = sBuff32/sizeof(qint32); //sz32/sizeof(qint32);       // число элем. в преоб. массиве

     for (int i = 0; i < l; i = i + 2) {  // данные из принятого пакета заносятся в циклический буф.
         int_complex val;
         val.real(p[i]);
         val.imag(p[i+1]);
         BuffVal buf_val;
         buf_val.seq_num = num_pack;
         buf_val.sig = 10 * val;
         m_cbuff.enqueue(buf_val);
         ++ m_point_num;                 // ведем подсчет элементов, помещенных в цикл. буф.
     }

     if(m_point_num >= 2 * m_slot_size){  // если в цикл. буф. в текущий момент находится кол-во точек >= 2* размер слота

        m_point_num = m_point_num - m_slot_size;    // значение счетчика точек уменьшаем на размер слота
        m_signal_IQ = new QVector<int_complex>;     // двойной буф.IQ для декодирования

        for(int i = 0; i < 2 * m_slot_size; ++i){ // заполняем из циклического буф данными в размере 2-x слотов
            m_signal_IQ->append(m_cbuff.at(i).sig);
         }

        for(int j = 0; j < m_slot_size; ++j ){   // удаляем данные из цикл. буф. в размере 1-го слота
            m_cbuff.dequeue();
         }

       emit sigIQFromCorr(m_signal_IQ);       // передача данных двойного буф. из радио канал далее в CoreDCE(модем) для декодирования
   }                                        // дин. выделение памяти m_signal_IQ освобождается после демодуляции в модулеCoreDCE
 }
 */
 //---------------------------------------------------------------------------------------------
 QByteArray &operator<<(QByteArray &l, qint8 r)
 {
     l.append(r);
     return l;
 }
 QByteArray &operator<<(QByteArray &l, qint16 r)
 {
     //return l<<qint8(r>>8)<<qint8(r);
     return l<<qint8(r)<<qint8(r>>8);
 }
 QByteArray &operator<<(QByteArray &l, qint32 r)
 {
     return l<<qint16(r>>16)<<qint16(r);
 }
 //-----------------------------------------------------------------------------------------------
 void ProtocolRadio :: slotUseBuffSlot(bool state)
 {
     if(state)
         m_buff_slot = 2;  //буфферизация есть(накапливаем 2 слота)
     else
         m_buff_slot = 1;  // буфферизации нет(отправляем 1 слот)
 }
 //-----------------------------------------------------------------------------------------------
 QByteArray &operator<<(QByteArray &l, quint8 r)
 {
     l.append(r);
     return l;
 }
 QByteArray &operator<<(QByteArray &l, quint16 r)
 {
     return l<<quint8(r>>8)<<quint8(r);
 }
 QByteArray &operator<<(QByteArray &l, quint32 r)
 {
     return l<<quint16(r>>16)<<quint16(r);
 }

 //----------------------------------------------------------------------------------------------- 
 //   ПРД
 // отправка кодограммы корреспонденту
 void ProtocolRadio::sendIQToCorr(SlotFieldS & current_slot/*short_complex * signal, int len, int freq_num*/)
 {
     m_queue_slot.enqueue(current_slot);

     if(!m_timer_pack->isActive())   // если таймер передачи пакетов еще не включен - выключаем его
          m_timer_pack->start(m_time_part_slot_pass);
     if(m_slot_proc){
      // makePacksFromSlot(current_slot);  // нарезаем пришедший слот на пакеты и помещаем в очередь для пакетов
       SlotFieldS slot = m_queue_slot.dequeue();
         makePacksFromSlot(slot);

     }


    //   m_queue_slot.enqueue(current_slot);
    //  if(m_queue_slot.count() > m_buff_slot)  // если буферизация слотов произошла
    //    m_slot_pass = true;                // слоты можно брать из очереди и отправлять по пакетам
    //  if( m_slot_pass){                     // проверяем  состояние буф.
    //    for(int i = 0; i < m_queue_slot.count(); ++i)
    //     queueSlotDequeue();                // опусташаем всю очередь слотов

    // }
 }

 //----------------------------------------------------------------------------
 // опустошение очереди слотов для отправки
 void ProtocolRadio::queueSlotDequeue()
 {
    QJsonObject jObj_out;
    if(m_queue_slot.count() > 0){
       SlotFieldS current_slot = m_queue_slot.dequeue();   // выбираем объект из очереди предназначенный для отправки
       makePacksFromSlot(current_slot);
    }
 }
 //-----------------------------------------------------------------------------------------------
 void ProtocolRadio::makePacksFromSlot(SlotFieldS & slot)
 {
    QByteArray ba;                   // данные за один пакет
    QByteArray buff_data;

    m_freq_slot = slot.freq_num;   // частота на которой необх. передавать слот
    m_slot_proc = false;
     for (int i = 0; i < slot.signal_len; ++i){

        if((ba.count() > 0) && (i % 200 == 0) ){   // если набрали 200 точек для пакета
          if(m_format16){  // если формат отправл. данных 16 байт
              PackFieldS data_pack;
              data_pack.ba_pack.append(ba);
              data_pack.freq_num = slot.freq_num;
              m_queue_pack.enqueue(data_pack);
          }
          else{
              qint64 sz_24 = ((ba.size() * 3) >> 1);
             // QByteArray buff_data;
              buff_data.resize(sz_24);
              qint64 sz_data = conversion16to24(ba.data(), buff_data.data(), ba.size()); // преобразование поля дата 16 -> 24
              sz_data =0;
              PackFieldS data_pack;
              data_pack.ba_pack.append(buff_data);
              data_pack.freq_num = slot.freq_num;
              m_queue_pack.enqueue(data_pack);
          }
           ba.clear();
           buff_data.clear();
        }

        short_complex val = *(slot.signal + i);
        //qDebug()<<"Sig_mod: "<< i <<re << im;
        short re1 = val.real();
        short im1 = val.imag();
      //  qDebug()<<"Sig_mod+: "<< i <<val.real() <<val.imag();// << re1 << im1;
        ba<<qint16(reinterpret_cast<qint16>(re1));
        ba<<qint16(reinterpret_cast<qint16>(im1));
      }
          // добавляем в очередь последний пакет
   //  if(m_format16){  // если формат отпарвл. данных 16 байт
         PackFieldS data_pack;
         data_pack.ba_pack.append(ba);
         data_pack.freq_num = slot.freq_num;
         m_queue_pack.enqueue(data_pack);
     // }
    /* else {// если 24 байт
         qint64 sz_24 = ((ba.size() * 3) >> 1);
        // QByteArray buff_data;
         buff_data.resize(sz_24);
         PackFieldS data_pack;
         data_pack.ba_pack.append(buff_data);
         data_pack.freq_num = slot.freq_num;
         qint64 sz_data = conversion16to24(ba.data(), buff_data.data(), ba.size()); // преобразование поля дата 16 -> 24
         m_queue_pack.enqueue(data_pack);
     }
     */
     ba.clear();
     buff_data.clear();

     slot.signal = nullptr;
     delete[] slot.signal;

     m_slot_proc=true;   // обработка слота завершена
 }
 //------------------------------------------------------------------------------------------------------
 // обработка таймера отправки слота в радиоканал (Сервер обмена)
 void ProtocolRadio::slotPassTimerAlarm()
 {
     if(m_transportUDP != nullptr){
         if(m_queue_pack.count()>0){ // если есть хоть 1 пакет на передачу в очереди
            PackFieldS pack_curr;
            ++m_curr_slot_part;
              pack_curr = m_queue_pack.dequeue();
              m_transportUDP->sendToCorr(pack_curr.ba_pack, pack_curr.freq_num);       // отправляем корр. первую из 8 частей слота
            //  ++m_num_log_p;
             // QString title{"timer10_pack: "};
            //  createLog_time("log_pack10.txt",m_num_log_p, title);
         }
         else{
             queueSlotDequeue();
         }
    }
 }
 //-----------------------------------------------------------------------------------------------
 bool ProtocolRadio::removeLog_time(const QString fl_name)
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
 void ProtocolRadio::createLog_time(const QString fl_name, int num, QString &title)
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

             logStream <<title<<QString::number(num)<< " " <<QTime::currentTime().toString("ss.zzz");

           logStream <<"\n";
           fl_log.close();
         }
     }
     if (!fl_log.exists()){
         if((!fl_log.open(QIODevice::WriteOnly))|(QIODevice::Append)){

         }
         else{
             QTextStream logStream(&fl_log);
             logStream <<title<<QString::number(num)<<" " <<QTime::currentTime().toString("ss.zzz") << "\t";
             logStream <<"\n";
             fl_log.close();
         }
     }
 }

 //-----------------------------------------------------------------------------------------------------
 // обработчик сигнала  об остановки сервера из MW сервера обмена
 void ProtocolRadio::slotStopServer()
 {
     emit sigStopServOrDiscon();
 }
//------------------------------------------------------------------------------------------------------

