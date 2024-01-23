// Базовый класс TransportBase - модуль отвечающий за установление соединения на уровне сокетов и представления
// сообщения в виде пакетов (при помощи модуля PacketC), является базовым для классов TransportServ, TransportClient

#ifndef TRANSPORTBASE_H
#define TRANSPORTBASE_H

#include <QObject>
#include <QTcpSocket>
#include <QTime>
#include "enum_list.h"
#include "struct_list.h"
#include "logmaker.h"

//Q_DECLARE_METATYPE(MsgDescrS)

class TransportBase :public QObject
{
    Q_OBJECT

public:
    TransportBase(TypeMsgEnum type_msg = JSON);
    ~TransportBase();

protected:
    LogMaker            *m_log_maker;      // логирование сообщений
    int                  m_count{0};       // счетчик сообщений для лога
    TypeMsgEnum          m_type_msg;       // тип сообщения
    QVector <QByteArray> m_vec_data;       // контейнер для накапливания сообщение в пакетаx (с заголовками)
    QByteArray           m_msg_whole;      // сообщение целиком (может состоять из нескольких принятых последовательно частей)

    void                readSocket(QByteArray &msg_whole, QTcpSocket & tcp_socket,
                                   TypeMsgEnum type_msg, QVector<QByteArray> & vec_data);
    void                writeSocket(QByteArray & ba, QTcpSocket & tcp_socket,TypeMsgEnum type_msg);
    bool                checkMsgAll(QByteArray ba, TypeMsgEnum type_msg);   // проверка получено ли сообщение целиком или
                                                                            // ждем окончание сообщения в след. пакете
    QByteArray          getJsonPack(QByteArray & xmlJsonPack);
    QByteArray          getXmlMsg(QByteArray & xmlJsonPack);
    void                cutToPack(QByteArray &ba);                         // очистка принятого сообщения от транспортного заголовка
    void                addToLog(int k,  QByteArray & ba, QDateTime tm, LogUseEnum flag_use);       // внесение записи в лог файл транспорт. уровня

signals:

    void  sigMsg       (int,  MsgDescrS &, QDateTime);    // сигнал о ПРИНЯТОМ/ОТПРАВЛЕННОМ сообщении
    void  sigMsgToJourn(int,  QByteArray &, QDateTime);    // сигнал с принятым/отправленном сообщением для отображения в Журнале
    void  sigOkError   (QString &, QTime );            // сигнал о состоянии соединения
};

#endif // TRANSPORTBASE_H
