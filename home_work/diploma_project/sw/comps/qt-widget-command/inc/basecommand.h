#ifndef BASECOMMAND_H
#define BASECOMMAND_H

#include <QObject>
#include <QSettings>
#include <QWidget>
#include "structs_list.h"


class BaseCommand
{
 public:
    BaseCommand();

    QString        getPathToIniFile();
    QString        checkDataBeforeWrite();
    unsigned short calcCRCForSettings(QSettings  *sett,
                                      QString section_name,
                                      QString crc_key_name);
    uint16_t       calculate_crc16(char * data, uint32_t size);
    uint16_t       calculateCRC16 (uint16_t crc,
                                   const uint8_t *data,
                                   uint32_t data_len,
                                   const uint16_t * crc_table);
    bool           checkDirExist(QString &dir_name_);
    bool           makeDir(QString &dir_name_);
    QString        settFileExist();
    bool           checkDataBeforRead(QSettings  *sett,      // объект настоек
                                      QString section_name,  // имя секции в ини файле команды
                                      QString crc_key_name); // имя ключа crc_ в секции команды
public:
    ModeYesNoEnum      getModeIO(int val);
    ModeSwitchEnum     getModeSwitch(QString str_data);
    ModeExchEnum       getModeExch(int val);
    ModeOffOnEnum      getModeAdapt(int val);

    ModePriorityEnum   getModePriority(QString str_data);
    ModeNetConnectEnum getModeConnectType(QString str_data);
    ModeSyncExchEnum   getModeSynchro(QString str_data);
};

#endif // BASECOMMAND_H
