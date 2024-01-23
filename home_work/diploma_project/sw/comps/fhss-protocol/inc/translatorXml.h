// класс TRANSLATORXML  для реализации преобразования сообщения XML<->JSON форматами
// работает на стороне клиента и на стороне сервера
//
// основная процедура преобразования сообщения из xml в json : " translateToJson"
// основная процедура преобразования сообщения из json в xml : "translateToXml"
// список ключей из всех команд, которые должны обрабатываться как массивы значений даже если число значений в массиве = 1:
//   "radioParams", "list", "synchroPack", "internalTime","freqList","dceList","dirList";


#ifndef TRANSLATORXML_H
#define TRANSLATORXML_H

#include <QString>
#include <QVector>
#include <QStringList>

#include "struct_list.h"

class TranslatorXML
{
public:
    TranslatorXML();
    QByteArray  translateToJson(const QByteArray & xmlPack);        //перевод из xml->json
    QByteArray  translateToXml(const QByteArray & jsonPack);        //перевод из json->xml
private:

    int         m_arr_end_pos{0};      // позиция "]" в массиве
    QStringList m_array_special_key;  // список ключей, обрабатываемых как массив

    QString     array_process(QString key, QString &str_j);         // обработка массива значений
    QString     value_process(QString key, QVector<QString> &vec_tag, QString &str_j);       // обработка значения
    QString     open_tagXML(QString name, QVector<QString>& vec_tag);   // создание открывающегося тега
    QString     close_tagXML(QVector<QString>& vec_tag);                // создание закрывающегося тега
    QString     info_tagXML(QStringList);                               // создание тега со значением <name>val</val>
    QString     commaXML(QVector<QString>& vec_tag);

    TagFeatures analyseTag(const QVector<QString>& vec_tag, QString cur_tag); // анализ текущего тега (задание характеристик тега по данным вектора всех тегов)
    void        getTranslateItem(QString &json, TagFeatures tag_feat,
                                 QString val, QString tag_name);              // создание последов. json
                                                                              //в зависимости от особенности текущего
                                                                              //(считанного) тега ( для перевода xml->json)
};

#endif // TRANSLATORXML_H
