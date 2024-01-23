// модуль для формирования транспортного пакета (проверка и удаление заголовка при получении пакета )
// при пересылки сообщения между

#ifndef PACKET_H
#define PACKET_H

#include <QByteArray>

enum  TypeErr{NO_ERR, CRC_ERR, JSON_ERR, BAD_COMAND_ERR}; // варианты ошибок

class Packet
{
public:    
    Packet();
    void           setMsg(QByteArray ba);
    void           clear();
    QByteArray     packetCreate();
    TypeErr        getClearMsg(QByteArray & clear_msg, const QByteArray & pack_msg);

private:
    QByteArray      m_msg;
    unsigned short  m_crc = 0;
    unsigned long   m_size = 0;
    QByteArray      m_compres;
    QByteArray      m_start;

    unsigned short crc16(unsigned char * pcBlock, unsigned short len);
    uint16_t       calculate_crc16(char * data, uint32_t size);
    uint16_t       calculateCRC16 (uint16_t crc,
                                          const uint8_t *data,
                                          uint32_t data_len,
                                          const uint16_t * crc_table);

};

#endif // PACKET_H
