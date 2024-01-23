#ifndef WIDGET_EXCH_IQ_H
#define WIDGET_EXCH_IQ_H

#include <QWidget>
#include "protocolRadio.h"
#include <complex>
#include <QVector>

typedef  std::complex<short> short_complex;
typedef  std::complex<int> int_complex;

QT_BEGIN_NAMESPACE
namespace Ui { class Widget_Exch_IQ; }
QT_END_NAMESPACE

//Q_DECLARE_METATYPE(NetConnectS)

class Widget_Exch_IQ : public QWidget
{
    Q_OBJECT

public:
    Widget_Exch_IQ(QWidget *parent = nullptr);
    ~Widget_Exch_IQ();
    void    setParams(NetS portAdr, int port_bind, int index_zpch, int start_buf);   // установка адреса кореспонд. и порта прослушки и индекса ЗПЧ
    void    setRXInterface(); // настройка интерфейса виджета в случае роли RX (ПРМ) ПАК УВС
    void    setTXInterface(); // настройка интерфейса виджета в случае роли TX (ПРД) ПАК УВС
    void    setBinding();                                         // соединение
    void    setDisconnect();                                      // разрыв соединения
    void    setSlotSize(int sz);
    int     getSlotSize();
    bool    getNoConnect();                                       // получение значения перем. m_no_connect

public slots:   

 //   void slotPackFromConnectorEx(int tx, QByteArray & ba, QTime tm); // обработка события прихода пакета от connector через Сервер Обмена       

    void slotErrorFromConnectEx( QString & str, QTime tm);       // событие обработки состояния соединения
    void slotIQFromExch(QVector<int_complex> * signal);          // обработка ответной кодограммы из радио канала
    void slotIQEnergy(qreal);
    void slotIQToExch(SlotFieldS & slot);                        // отправка signal из ModemPanel через радио канал корреспонденту
    void slotBfShift(qint32 lf_sh, qint32 rt_sh);                // сдвиг буф перед демодуляцией для коррект. bf
    void slotErrLost(QString & err_journ);                       // отправка статистики потерянных пакетов
    void slotGetModemSlotLen();                                  // запрос длинны слота у модема
private slots:

    void slotNetWorkingExch();                                   // начало прослушки порта с заданным номером ()
    void slotNewBinding();
    void slotDiscon();                                           // закрыть прослушиваемый порт    
    void slotDataToJournal(QByteArray & buf, int num_pack, int flag_pass); // заполнение журнала отправленных/ принятых пакетов
    void slotZpchChanged(int);

private:
    Ui::Widget_Exch_IQ *ui;   
    ProtocolRadio   *m_protocolRadio{nullptr};      // протокол радио канала
    NetS             m_net_corr;                    // адрес, порт корресп.
    int              m_port_bind;                   // порт прослушки    
    int              m_slot_size{0};                // размер слота модема
    bool             m_no_connect{true};            // флаг неудачной попытке соединения с Radio

    //  bool     getForUsEx(const QByteArray & ba);
    //  bool     getForUsEx(QJsonObject & obj);
    void      startRadioWork();   // запуск радио сети с новыми параметрами
    void      stopRadioWork();

signals:
    void     sigDisconEx();
    void     sigCodoFromCorr(QVector<int_complex>*signal);
    void     sigBindPort(int);      // начать прослушку порта с заданным номером
    void     sigUseBuffSlot(bool);  // использовать буф. слотов перед отправкой по udp
    void     sigGetSlotSize();      // запрос о получении размера слота модема в рабочем режиме
    void     sigZpchCh(int);

    // сигнал об отключении от Сервера Обмена
    // void     sigSendIQExch(QJsonObject &);          // сигнал ProtocolRadio об отправке команды через радио канал

    //  void     sigWriteMsgToJournEx(int, QTime, QString &, const QByteArray &, QColor);
    //  void     sigGetIQFromExch(QJsonObject &);

};
#endif // WIDGET_EXCH_IQ_H
