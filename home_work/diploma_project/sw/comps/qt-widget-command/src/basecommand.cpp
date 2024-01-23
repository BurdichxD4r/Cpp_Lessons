#include "basecommand.h"
#include <QDir>
#include <QFile>
#include <QWidget>
#include <QMessageBox>
#include <QApplication>

QString     INI_PATH;       // путь к текущему каталогу настроек команд
QString     APP_ABS_PATH;   // абс. путь к исполн. прилож
QString     INI_FL;         // имя файла настоек команд

// необходимо соответствие с ModeSwitchEnum, ModeExchEnum, ModeOffOnEnum из enums_list.h (fhss-types компоненты)
QStringList REGIM      = {"ФРЧ","НРЧ","ППРЧ"};
QStringList REGIM_EXCH = {"HALF_DUPLEX", "DUPLEX", "SIMPLEX"};
QStringList ADAPTATION = {"OFF", "ON"};

QStringList YESNO      = {"NO", "YES"};
QStringList REGIM_C    = {"точка-точка", "точка-многоточка"};
QStringList REGIM_S    = {"синхронный", "асинхронный"};
QStringList PRIORITY   = {"M", "B", "P", "C", "O"};

//---------------------------------------------------------------------------------
static const uint16_t CRC16_XMODEM_TABLE[256] = {
    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50A5, 0x60C6, 0x70E7,
    0x8108, 0x9129, 0xA14A, 0xB16B, 0xC18C, 0xD1AD, 0xE1CE, 0xF1EF,
    0x1231, 0x0210, 0x3273, 0x2252, 0x52B5, 0x4294, 0x72F7, 0x62D6,
    0x9339, 0x8318, 0xB37B, 0xA35A, 0xD3BD, 0xC39C, 0xF3FF, 0xE3DE,
    0x2462, 0x3443, 0x0420, 0x1401, 0x64E6, 0x74C7, 0x44A4, 0x5485,
    0xA56A, 0xB54B, 0x8528, 0x9509, 0xE5EE, 0xF5CF, 0xC5AC, 0xD58D,
    0x3653, 0x2672, 0x1611, 0x0630, 0x76D7, 0x66F6, 0x5695, 0x46B4,
    0xB75B, 0xA77A, 0x9719, 0x8738, 0xF7DF, 0xE7FE, 0xD79D, 0xC7BC,
    0x48C4, 0x58E5, 0x6886, 0x78A7, 0x0840, 0x1861, 0x2802, 0x3823,
    0xC9CC, 0xD9ED, 0xE98E, 0xF9AF, 0x8948, 0x9969, 0xA90A, 0xB92B,
    0x5AF5, 0x4AD4, 0x7AB7, 0x6A96, 0x1A71, 0x0A50, 0x3A33, 0x2A12,
    0xDBFD, 0xCBDC, 0xFBBF, 0xEB9E, 0x9B79, 0x8B58, 0xBB3B, 0xAB1A,
    0x6CA6, 0x7C87, 0x4CE4, 0x5CC5, 0x2C22, 0x3C03, 0x0C60, 0x1C41,
    0xEDAE, 0xFD8F, 0xCDEC, 0xDDCD, 0xAD2A, 0xBD0B, 0x8D68, 0x9D49,
    0x7E97, 0x6EB6, 0x5ED5, 0x4EF4, 0x3E13, 0x2E32, 0x1E51, 0x0E70,
    0xFF9F, 0xEFBE, 0xDFDD, 0xCFFC, 0xBF1B, 0xAF3A, 0x9F59, 0x8F78,
    0x9188, 0x81A9, 0xB1CA, 0xA1EB, 0xD10C, 0xC12D, 0xF14E, 0xE16F,
    0x1080, 0x00A1, 0x30C2, 0x20E3, 0x5004, 0x4025, 0x7046, 0x6067,
    0x83B9, 0x9398, 0xA3FB, 0xB3DA, 0xC33D, 0xD31C, 0xE37F, 0xF35E,
    0x02B1, 0x1290, 0x22F3, 0x32D2, 0x4235, 0x5214, 0x6277, 0x7256,
    0xB5EA, 0xA5CB, 0x95A8, 0x8589, 0xF56E, 0xE54F, 0xD52C, 0xC50D,
    0x34E2, 0x24C3, 0x14A0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
    0xA7DB, 0xB7FA, 0x8799, 0x97B8, 0xE75F, 0xF77E, 0xC71D, 0xD73C,
    0x26D3, 0x36F2, 0x0691, 0x16B0, 0x6657, 0x7676, 0x4615, 0x5634,
    0xD94C, 0xC96D, 0xF90E, 0xE92F, 0x99C8, 0x89E9, 0xB98A, 0xA9AB,
    0x5844, 0x4865, 0x7806, 0x6827, 0x18C0, 0x08E1, 0x3882, 0x28A3,
    0xCB7D, 0xDB5C, 0xEB3F, 0xFB1E, 0x8BF9, 0x9BD8, 0xABBB, 0xBB9A,
    0x4A75, 0x5A54, 0x6A37, 0x7A16, 0x0AF1, 0x1AD0, 0x2AB3, 0x3A92,
    0xFD2E, 0xED0F, 0xDD6C, 0xCD4D, 0xBDAA, 0xAD8B, 0x9DE8, 0x8DC9,
    0x7C26, 0x6C07, 0x5C64, 0x4C45, 0x3CA2, 0x2C83, 0x1CE0, 0x0CC1,
    0xEF1F, 0xFF3E, 0xCF5D, 0xDF7C, 0xAF9B, 0xBFBA, 0x8FD9, 0x9FF8,
    0x6E17, 0x7E36, 0x4E55, 0x5E74, 0x2E93, 0x3EB2, 0x0ED1, 0x1EF0
};
//---------------------------------------------------------------------------------
BaseCommand::BaseCommand()
{
    APP_ABS_PATH = QCoreApplication::applicationDirPath();
    INI_PATH     = "/INI/";         // дир. с файлами конфигур.
    INI_FL       = "command.ini";   // имя файла конфигур.
}
//----------------------------------------------------------------------------------
//---------------------------------------------------------------------------------
ModePriorityEnum  BaseCommand::getModePriority(QString str_data)
{
    for (int i = 0; i < PRIORITY.count(); ++i){
      if(str_data == PRIORITY.at(i))
        return ModePriorityEnum(i);
    }
    return ModePriorityEnum(0);   // значение по умолчанию
}
//---------------------------------------------------------------------------------
ModeNetConnectEnum  BaseCommand::getModeConnectType(QString str_data)
{
    for (int i = 0; i < REGIM_C.count(); ++i){
      if(str_data == REGIM_C.at(i))
        return ModeNetConnectEnum(i);
    }
    return ModeNetConnectEnum(0);   // значение по умолчанию
}
//---------------------------------------------------------------------------------
ModeSyncExchEnum  BaseCommand::getModeSynchro(QString str_data)
{
    for (int i = 0; i < REGIM_C.count(); ++i){
      if(str_data == REGIM_C.at(i))
        return ModeSyncExchEnum(i);
    }
    return ModeSyncExchEnum(0);   // значение по умолчанию
}
//---------------------------------------------------------------------------------
ModeOffOnEnum BaseCommand::getModeAdapt(int val)
{
    if(val >= ModeOffOnEnum(0) && val <= ModeOffOnEnum(ADAPTATION.count()))
        return ModeOffOnEnum(val);
    return ModeOffOnEnum(0);
}
//---------------------------------------------------------------------------------
ModeSwitchEnum  BaseCommand::getModeSwitch(QString str_data)
{
    for (int i = 0; i < REGIM.count(); ++i){
      if(str_data == REGIM.at(i))
        return ModeSwitchEnum(i);
    }
    return ModeSwitchEnum(0);   // значение по умолчанию
}
//---------------------------------------------------------------------------------
ModeExchEnum BaseCommand::getModeExch(int val)
{
  //   if (value >= Month::January && value <= Month::December) return true;
    if(val >= ModeExchEnum(0) && val <= ModeExchEnum(REGIM_EXCH.count()))
      return ModeExchEnum(val);
    return ModeExchEnum(0);
}
//-----------------------------------------------------------------------------------
ModeYesNoEnum BaseCommand::getModeIO(int val)
{
    if(val >= ModeYesNoEnum(0) && val <= ModeYesNoEnum(YESNO.count()))
        return ModeYesNoEnum(val);
    return ModeYesNoEnum(0);
}
//-----------------------------------------------------------------------------------
QString BaseCommand::getPathToIniFile(){

    // при сохранении объекта настроек в файл, проверяем имеется ли файл на диске, его могли удалить во время работы программы
    QString fail_fields{""};
    QString     fl_path;
    fail_fields = checkDataBeforeWrite();

    if(fail_fields==""){              // если список полей с ошибками пуст
      QString     fl_dir = APP_ABS_PATH + INI_PATH;
      if (!checkDirExist(fl_dir))     // если текущей дир. не существ.
            if(!makeDir(fl_dir))      // пытаемся ее создать
               {}
      fl_path = APP_ABS_PATH + INI_PATH + INI_FL;
       QFile *fl = new QFile(fl_path);

       if (!fl->exists()){
           if(!fl->open(QIODevice::WriteOnly|QIODevice::Append)){
           //   QMessageBox::critical(0, "Ошибка","Файл : "
           //                                        "" + fl_dir  + fl_path +
            //                                       " не может быть создан ! "
            //                                       "Программа будет закрыта.");

            }
           else { // если файл создан
                  //оставим вновь созданный файл пустым
               fl->close();
               delete fl;
            }
       }

     //  QFile(fl_path).remove();
       QFile myfile(fl_path);
       if (myfile.isOpen())
          myfile.close();
    }
    else{    // выдаем сообщение о необходимости заполнить все поля
        QMessageBox::critical(0, "Сообщение", "Слудует заполнить следующие поля формы :\n" ,"");
    }

    return fl_path;
}
//---------------------------------------------------------------------------------
// проверка корректности внесенных на форму данных
QString BaseCommand::checkDataBeforeWrite()
{
    QString err_in_fields{""};

    return err_in_fields;
}
//---------------------------------------------------------------------------------
// рассчет crc по данным считанным из файла настроек
unsigned short BaseCommand:: calcCRCForSettings(QSettings  *sett, QString section_name, QString crc_key_name)
{
    QByteArray ba;

    QStringList group = sett->childGroups();  // получаем список секций в ини-файле
    for (int i = 0; i < group.size(); i++ ) {
        sett->beginGroup(group.at(i));

        if (group.at(i) == section_name){  // основные настройки команды Enable
            QStringList keys = sett->allKeys();//childKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
                if (keys.at(j)==crc_key_name){
                   // crc = sett->value("crc_enableDCE").toInt();
                }
                else{ // собираем все поля команды и значения кроме crc_enableDCE
                    QByteArray b;
                    b.append(keys.at(j).toUtf8());
                    b.append(sett->value(keys.at(j)).toString().toUtf8());
                    ba.append(b);
                }
            }
        }

        sett->endGroup(); // завершение обхода группы "EnableDCE"
    }
    unsigned int len = ba.count();
    char * block = reinterpret_cast<char*>(ba.data());
    unsigned short crc_calc = calculate_crc16(block, len);
    return crc_calc;
}
//---------------------------------------------------------------------------------
// проверка целостности  данных считанных из ini файла настоек
bool BaseCommand:: checkDataBeforRead(QSettings  *sett, QString section_name, QString crc_key_name)
{
    unsigned short crc{0}; // считанное из ини файла команды
    bool sett_exist=false;     // переменная для отображения найден ли секция
                              //данной команды в файле настоек

    QStringList group = sett->childGroups();  // получаем список секций в ини-файле
    for (int i = 0; i < group.size(); i++ ) {
        sett->beginGroup(group.at(i));

        if (group.at(i) == section_name){  // основные настройки команды Enable
            sett_exist = true;   // секция настоек найдена
            QStringList keys = sett->allKeys();  // получаем список ключей данной секции
            for (int j = 0; j < keys.size(); j++ ) {
                if (keys.at(j)==crc_key_name){
                    crc = sett->value(crc_key_name).toInt();   // crc -записанный в файле
                    break;
                }
            }
        }
        sett->endGroup(); // завершение обхода группы
    }

    unsigned short crc_calc = calcCRCForSettings(sett, section_name, crc_key_name );   // crc - рассчитанный по считанным данным из файла настроек

    if(crc_calc == static_cast<unsigned short>(crc)&& (sett_exist))   // целостность подтверждена
      return true;
    else
      return false;
}

//--------------------------------------------------------------------
// проверка целостности файла с настройками
uint16_t  BaseCommand::calculateCRC16 (uint16_t crc,
                                      const uint8_t *data,
                                      uint32_t data_len,
                                      const uint16_t * crc_table)
{
    while (data_len-- > 0)
        crc = (crc_table[((crc >> 8) ^ *data++) & 0xff] ^ (crc << 8)) & 0xffff;
    return crc & 0xffff;
}
//-------------------------------------------------------------------
uint16_t BaseCommand::calculate_crc16(char * data, uint32_t size)
{
    unsigned short crc = 0;
    return calculateCRC16 (crc & 0xffff, (uint8_t *)(data), size , CRC16_XMODEM_TABLE);
}
//---------------------------------------------------------------------------------
// проверка существования файла с настройками.
// если файла не существует (не найден) возвращается пустой путь, если найден, путь к файлу настроек
QString BaseCommand::settFileExist()
{
    QString     fl_path = APP_ABS_PATH + INI_PATH + INI_FL;
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
//-------------------------------------------------------------------
// проверка существования директории с заданным имененм
bool BaseCommand:: checkDirExist(QString &dir_name_)
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
bool BaseCommand:: makeDir(QString &dir_name_)
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
//-----------------------------------------------------------------------
