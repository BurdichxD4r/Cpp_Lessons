#include "transportServ.h"
#include <QHostInfo>
#include <QMessageBox>

//const int TX = 1;   // передача
//const int RX = 0;   // прием

//-----------------------------------------------
TransportServ::TransportServ( NetS & net_adr, TypeMsgEnum type_msg):
    TransportBase(type_msg)
{
    m_server = new QTcpServer(this);
    m_socket = new QTcpSocket(this); // для передачи ответного сообщения

    // проверка на занятость порта для прослушивания
    if (!m_server->listen(QHostAddress::AnyIPv4, net_adr.port)) {
       QMessageBox::critical(0, "Сообщение","Порт " +QString::number(net_adr.port)+ "занят" ,"");
       m_server->close();
       return;
     }
     else
        {   // если порт свободен, отправляем сигнал о рабочем состоянии
            QString str = "ok";
            emit sigOkError(str, QTime::currentTime() );
            emit sigNetAdr(net_adr);
        }
    // организуем обработку нового присоединения клиента
    connect(m_server, &QTcpServer::newConnection, this, &TransportServ::slotNewConnection);
}
//----------------------------------------------------------------
TransportServ::~TransportServ() = default;


//--------   Основные процедуры коммуникации(приема / передачи)---
//       вызываются из MWind -------------------------------------
//------------------------------------------------------------------
//               основная функция оправки пакета КЛиенту
//------------------------------------------------------------------
void TransportServ::sendToClient(int descr, QByteArray& ba)
{
    QTcpSocket *socket = new QTcpSocket();
    socket = nullptr;
    if(descr!=0){
      socket = SClients.value(descr);     // выбираем сокет из пула подкл. клиентов по дискриптору
    }
    else {
      socket = m_socket;  // если пришедший дискриптор не валидный, отправляем в текущий сокет
    }
    writeSocket(ba, *socket, m_type_msg);
  //  delete socket;
}
//------------------------------------------------------------------
//    getter Сокета
//------------------------------------------------------------------
// получение дискриптора сокета текущего соденияения
QTcpSocket* TransportServ:: getSocket()
{ 
     return m_socket;
}
//-------------------------------------------------------------------
void TransportServ::namedClient(int descr/*QTcpSocket* pSocket*/, QString client_name)
{
    if(client_name!=""){   // обновление инф. о клиенте если пришла ком. TermType
       if(SClientsType.contains(descr)){
           SClientsType.remove(descr);
       }
      // SClientsType.insert(descr, client_name);    //pSocket->socketDescriptor(), client_name);
      // QVector <NetConnectS> list_client = formClientListDCE(); // формирование списка клиентов подключенных
      // emit sigClientListDCE(list_client);                      // обновляем информацию  о списке подключенных клинентов
    }
    else if(client_name == ""){  // занесение инф. о клиенте по умолчанию
        client_name = "MAS";
       // SClientsType.insert(descr, client_name);    //pSocket->socketDescriptor(), client_name);
       // QVector <NetConnectS> list_client = formClientListDCE(); // формирование списка клиентов подключенных
      //  emit sigClientListDCE(list_client);                      // обновляем информацию  о списке подключенных клинентов
    }
    SClientsType.insert(descr, client_name);
    QVector <NetConnectS> list_client = formClientListDCE(); // формирование списка клиентов подключенных
    emit sigClientListDCE(list_client);
}
//-------------------------------------------------------------------
// получение списка подключенных к серверу клиентов, кроме сокета клиента от которого пришло сообщение
QVector<QTcpSocket*>TransportServ::getSocketOpponents()
{
    QVector <QTcpSocket*> ClientsList;  // пул подключенных клиентов оппонентов кому будут пересылаться сообщения

    QMap<int, QTcpSocket *> ::iterator it = SClients.begin();

    int descr = m_socket->socketDescriptor();           // дескриптор клиента отправившего команду
    QString cl_type = SClientsType.find(descr).value(); // тип клиента отправившего команду

    for (; it!=SClients.end(); ++it){           // просматриваем список клиентов
        if(it.value()!= m_socket ){        // если сокет в списке подкл. клиентов не тот что у отправившего клиента
          QString type_cl_cur = SClientsType.find(it.value()->socketDescriptor()).value(); // узнаем тип подходящего клиента
          if (type_cl_cur == cl_type)      // если тип текущего по списку клиента совпадает с типом клинета отправившеллго команду
             ClientsList.append(it.value());  // вновим его в список для рассылки
         }
     }
    return ClientsList;
}
//-------------------------------------------------------------------
// формирование списка подключенных клиентов для отправки в главное окно для отображения
QVector <NetConnectS> TransportServ::formClientListDCE()
{
    QMap<int, QTcpSocket *> ::iterator it = SClients.begin();  
    QVector <NetConnectS> list_client;

    for (; it!=SClients.end() ; ++it){     // просматриваем список клиентов
         NetConnectS  feat_client;
         if(SClientsType.contains(it.value()->socketDescriptor())){// если дискр. от тек. соединения присутствует в списке SClientsType
             QMap<int, QString>::const_iterator i =  SClientsType.find(it.value()->socketDescriptor()); // берем этот дискриптор
             feat_client.client_type = i.value();    // получаем значение типа присоединенного клинета по дискриптору
            }
        else
           feat_client.client_type = "UNKN";   // если тек. дискриптора в списке SClientsType нет  т.е. клиент еще не представился
        feat_client.discr = it.key();
        feat_client.port = it.value()->peerPort();
        feat_client.address = it.value()->peerAddress().toString();

        list_client.append(feat_client);    // формируем сам список
    }
    return list_client;                     // отправляем его интересующимся
}
//--------  основные слоты модуля TransportServ -------------------------

//------------------------------------------------------------------
//  подключение новых клиентов
//-----------------------------------------------------------------
void TransportServ::slotNewConnection()
{
    //QString str_out = "";
    QTcpSocket* pClientSocket = m_server->nextPendingConnection();
    m_socket = pClientSocket;
    int id_user_socs = pClientSocket->socketDescriptor();
    SClients[id_user_socs]= pClientSocket;
    //!!
    namedClient(id_user_socs,"");  // при новом подкл. именуем клиента по умолч.

    QVector <NetConnectS> feat_connect_vec;        // массив характеристик присоединенных к серверу клиентов
    feat_connect_vec = formClientListDCE();
    emit sigClientListDCE(feat_connect_vec);      // обновляем информацию  о списке подключенных клинентов

    // организуем обработки: чтения сообщений от пула клиентов и разрыв связи с каждым из них
    connect (SClients[id_user_socs], SIGNAL(readyRead()), this, SLOT(slotReadClient()));
    connect(SClients[id_user_socs], &QTcpSocket::disconnected, this, &TransportServ::slotDiscClient);
}
//-----------------------------------------------------------------
// остановка сервера
//-----------------------------------------------------------------
void TransportServ::slotStopServer()
{
    if (SClients.count() > 0){  // если есть подключенные клиенты
       QMessageBox::information(nullptr, "Сообщение Сервера",
                                      "После остановки Сервера все подключенные Клиенты будут отключены."
                                      );
       QList <QTcpSocket*> list = SClients.values();
       for (int i =0; i<list.count(); ++i){   // откл. всех клиентов
           QTcpSocket * socket = list.at(i);
            if(socket->isOpen())
               socket->close();
       }
    }
    SClients.clear();   // список подключенных клиентов очищаем
    m_server->close();  // сервер закрываем
}
//-----------------------------------------------------------------
//    отключение клиента
//-----------------------------------------------------------------
 void TransportServ::slotDiscClient()
{
    QMap<int, QTcpSocket *> ::iterator it = SClients.begin();
    QMap<int, QString> ::iterator it_name = SClientsType.begin();
    int k = 0;  // перем. куда считываем действующее значение дискрипкора сокета клиента
    int l = 0;
    //QString str_out = "";

    for (; it!=SClients.end(); ++it){  // просматриваем список сокетов подключенных клиентов
         k = it.value()->socketDescriptor();       
         if(k == -1){                    // ищем какой клиент отключился
             SClients.remove(it.key());  // удаляем его из списка сокетов
             break;
          }
     }
    for (; it_name!=SClientsType.end(); ++it_name){  // просматриваем список типов клиентов
         l = it_name.key();
         if(!SClients.contains(l)){  // если дескрир из списка SClientsType НЕ содержится в списке SClients
             SClientsType.remove(it_name.key());  // удаляем его из списка SClientsType
             break;
         }
     }
    QVector <NetConnectS> feat_connect_vec; // массив характеристик присоединенных к серверу клиентов
                // обновляем список действующих клиентов 
    feat_connect_vec = formClientListDCE();
    emit sigClientListDCE(feat_connect_vec);
 }
 //------------------------------------------------------------------------------------------------
 //  слот для обработки полученного сообщения от клиента, создается при отработке слота "slotNewConnection()"
void TransportServ::slotReadClient()
{
    QTcpSocket* pClientSocket = static_cast<QTcpSocket*>(sender());
    m_socket = pClientSocket;

    readSocket(m_msg_whole, *m_socket, m_type_msg, m_vec_data);
}
//------------------------------------------------------------------
