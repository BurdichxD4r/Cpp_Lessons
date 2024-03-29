#include "translatorXml.h"
#include <QStringList>

//------------------------------------------------------------------------
TranslatorXML::TranslatorXML()
{
    m_array_special_key<<"radioParams"<<"list"<<"synchroPack"<<"internalTime"<<"freqList"<<"freqPrimList"<<"freqAddList"<<"dceList"<<"dirList";
    m_array_special_key<<"/radioParams"<<"/list"<<"/synchroPack"<<"/internalTime"<<"/freqList"<<"/freqPrimList"<<"/freqAddList"<<"/dceList"<<"/dirList";
}
//-------------------------------------------------------------------------
QByteArray TranslatorXML::translateToXml(const QByteArray & jsonPack)
{
   QVector<QString> vec_tag;             // вектор назв. элементов док. xml
   QString res_mess;                     // результирующее сообщение, по окончанию преобразования
   res_mess = res_mess + "<message>\n";  // в рез. сообщение записываем сразу начало корневого элемента

         // удаление корневых скобок из json сообщение
   QString str_j = jsonPack;         // копируем переданную последрвательность во временную переменную
   int pos = str_j.indexOf("{");     // находим первое вхождение корневых скобок
   str_j.remove(pos, 1);             // удаляем пока коревую { из последовательности

   pos = str_j.lastIndexOf("}");     // находим последнее вхождение корневой скобки
   str_j.remove(pos, 1);             // удаляем коревую } из последовательности

      // начало обработки json сообщения
   while (str_j.size() > 0){
        QString     key;     //  ключ  для формирования элемента( <ключ> значение </ключ>)
        QString     cur_char;    // считанный из str_j очередной символ для анализа и обработки

/*ключ*/
      //  процедура получения ключа
      cur_char = str_j.at(0);                   // считываем первый символ слева
      if ((cur_char != "{")&&(cur_char !="[")&&(cur_char !="}")
                          &&(cur_char !="]")&&(cur_char !=","))
      {    // если символ не  входит в выбранную последовательность
                                                // значит это ключ
          int pos = str_j.indexOf(":");         // положение ":" в сообщ.
          key.append(str_j.left(pos).remove("\"")); // удалим из ключа все "
          key = key.trimmed();  //!!!
          key = key.simplified();//!!!
          str_j = str_j.right(str_j.count() - pos -1);  // сообщ.json без считанного ключа и ":"
       }

      // начало процедуры получения значения
      str_j = str_j.simplified();              // очищаем от незначащих пробелов
      if(str_j.size()>0){                      // если в сообщ. еще что то осталось
         cur_char = str_j.at(0);               // еще раз считываем первый символ слева

/*нач.объекта*/
          if (cur_char == "{"){       // начало ОБЪЕКТА
            if (key.size()>0)        // если в переменной уже имеется считанный ключ
               res_mess = res_mess + open_tagXML(key+"_o", vec_tag); // формируем открывающийся тег для начала формирования объекта
            else                          // если ключа нет, открыв. тега не формируется, но начало объекта необходимо внести в вектор тегов
               res_mess = res_mess + open_tagXML("x", vec_tag);
            str_j.remove(0,1);            // удаляем из последовательности обработанный символ
           }                        // окончание обработки начала ОБЪЕКТА

/*нач.массива*/
          else if (cur_char =="["){   //начало МАССИВА
            str_j.remove(0,1);           // этот символ мы не будем отображать в результирующей последоватлельности
            cur_char = str_j.at(0);      // еще раз считываем первый символ слева
   /*нач.массива объектов*/
            if (cur_char =="{"){     // если след. за [,  символ  { - начало МАССИВА ОБЪЕКТОВ
               if (key.size()>0){    // если к данному моменту в переменной уже имеется считанный ключ
                  res_mess = res_mess + open_tagXML(key+"_a", vec_tag); // формируем открывающийся тег для начала формирования массива объектов
               }
               else                      // ключа в переменной нет
                  res_mess = res_mess + open_tagXML("x", vec_tag); // если ключа нет, открывающего тега не формируется, но начала массива необходимо зафиксир. в векторе тегов
            }   // окончание обработки начала МАССИВА ОБЪЕКТОВ
   /*нач.массива значен.*/
            else{   // если след. за [,  символ  НЕ{ - начало МАССИВА ЗНАЧЕНЙ
                                       // обработаем его иначе
                if (key.size()>0){  // если имеется ключ
                    res_mess = res_mess + array_process(key, str_j);
                 }
            }   // окончание обработки начала МАССИВА ЗНАЧЕНИЙ
          }     // окончание обработки начала МАССИВА

/*оконч.массива*/
          else if (cur_char =="]"){ //окончание МАССИВА
             res_mess = res_mess + close_tagXML(vec_tag);   // формируем закрывающий тег
             str_j.remove(0,1);
          }

/*оконч.объекта*/
         else if (cur_char =="}"){ //окончание ОБЪЕКТА
             res_mess = res_mess + close_tagXML(vec_tag);  // формируем закрывающий тег
             str_j.remove(0,1);
         }

/*оконч.объекта или массива*/
         else if (cur_char ==","){ // окончание объекта или элемента массива
             res_mess = res_mess + commaXML(vec_tag);
             str_j.remove(0,1);
         }
/*значение*/
         else{  // далее в последовательности находится значение
                // его обработаем в отдельной функции
              if (key.size()>0){
                 res_mess = res_mess + value_process(key, vec_tag, str_j);
              }
         }   // это значение
      }   // если в сообщ. еще что то осталось
   }    // начало обработки сообщения while

  res_mess = res_mess + "</message>\n";  // добавляем в рез. сообщение закрывающий корневой тег

  return res_mess.toUtf8();
}
//------------------------------------------------------------------
//   вспомогательные процедуры для перевода json->xml
//------------------------------------------------------------------
// обработка принятой последовательности для выделения из нее значения для формирования элемента xml
// значение может быть числом или строкой, содержащей любые символы кроме ""
QString TranslatorXML::value_process(QString key, QVector<QString> &vec_tag, QString & str)
  {
     QString     res;       // значение
     QStringList key_val;
     QString     info_unit; // элементарный информационный элемент ( <ключ> значение </ключ>)

     key_val.append(key);   // ключ помещаем в переменную пары ключ / значение

     // значение может быть либо числом(даже дробным) либо строкой, которая в свою очередь может включать любые символы  даже "," но не (")
     QString cur_char = str.at(0);  // считываем первый символ
/*строка */
     if (cur_char == "\""){ // значением является строка
         str.remove(0,1);          // удаляем открыв. кавычки       
         int  pos = str.indexOf("\"");  // ищем закрыв. кавычку
         if(pos!=-1){ // если закр. " найдена
            key_val.append(str.left(pos)); // считываем данные внутри "
         }
         str.remove(0, pos+1);          // удаляем закр. кавычку
         if(str.size()>0)
            cur_char = str.at(0);      // считываем первый символ
         if(cur_char ==",")
            str.remove(0, 1);          // удалим её из последовательности
     }
/*число*/
     else if (cur_char.toInt()>=0){ // значением является число возможно дробное (может быть 0)
          int pos = str.indexOf(",");    // ищем окончание значения (ближайшую "," т.к в числовом значениеи этот символ не используется)
                                            // но символа , может и не быть
          QString val = str.left(pos);    // если это последнее значение, скопируем принятую последовательность до конца
          if(val.contains("}")){   // в скопированной послед. могут находится признаки окончания объекта, напомним массив значений обрабатывается отдельно и сюда не попадет
             int pos1 = val.indexOf("}");  // находим признак окончания объекта
             if (pos1!=-1)         // если этот символ } был найден
                 pos = pos1;      // он является границей искомого значения в последовательности
          }
          ///!!!! new
          else if(pos==-1){     // если в скопированной последовательности не содержится "}" и не имеет "," (т.е. это последнее значение)
                    // предполагаем что в ней только значение ключа
              key_val.append(val);
              str.remove(0, val.size());
          }

          if(pos!=-1){   //  если у нас есть точная граница выделяемого числового значения
             key_val.append(str.left(pos));  // выделим числовое значение
             str.remove(0, pos);             // удалим из переданной послед. то что мы обработали (значение) но не , или }
          }
          if(str.size()>0) // если еще что то осталось в разбираемой последовательности
             cur_char = str.at(0);  // считываем еще один символ
          if(cur_char ==",")
             str.remove(0, 1); // удалим её из последовательности
     }
   // обработка ключа/значения -создание информ. элемента
     info_unit = info_tagXML(key_val);  // формируем информ элемент
     res = res + info_unit;

     if (cur_char =="}"){  // если считанный из послед. символ }
        res = res + close_tagXML(vec_tag);// организуем закрывающийся тег
        str.remove(0, 1);      // удалим её из последовательности
     }
      return res;
  }
//------------------------------------------------------------------------------
// обработка "," в json последовательности
// конвертируется в закрыв. + откры. тег
QString TranslatorXML::commaXML(QVector<QString>& vec_tag)
{     // считываем последнее значение в векторе тегов без префика _а или _о, формируя закрыв. тек и следом открывающий
    QString comma_tag{""};
    if(vec_tag.count()>0){
        QString flag = vec_tag.last();
        if(flag != "x"){
            comma_tag = "</"+vec_tag.last().left(vec_tag.last().size()-2)+">\n" +  // закрвы. тег
                        "<"+vec_tag.last().left(vec_tag.last().size()-2)+">\n";    // открыв. тег
        }
    }
    return comma_tag;
}
//-----------------------------------------------------------------
// обработка "{" или "[" в json последовательности, конвертируется в открывающий тег
QString TranslatorXML::open_tagXML(QString name, QVector<QString>& vec_tag)
{
    QString open_tag{""};
    name.remove("\"");
    if(name !="x"){                         // имя тега нужно сохранить для организации закрыв. тега
       open_tag = "<"+name.left(name.size()-2)+">\n";       // сформировали открывающийся тег <smth>\n
    }
    vec_tag.append(name);  // добавляем данные об открыв. теге
    return open_tag;
}
//------------------------------------------------------------------
 // обработка "}" или "]" в json последовательности, конвертируется в закрывающийся тег
QString TranslatorXML::close_tagXML(QVector<QString>& vec_tag)
{                            // считываем последнее значение в векторе тегов, формируя закрыв. тек
    QString close_tag{""};
    if(vec_tag.count()>0){
        QString flag =vec_tag.last();

        if(flag != "x"){
            close_tag = "</"+vec_tag.last().left(vec_tag.last().size()-2)+">\n";
        }
        vec_tag.removeLast();  // удаляем тег который закрыли
    }

    return close_tag;
}
//------------------------------------------------------------------
QString   TranslatorXML::info_tagXML(QStringList list)
{
   QString key{""};
   QString val{""};

   if (list.count()>1){
       key =list.at(0);
       val =list.at(1);
   }
   key.remove("\"");
   val.remove("\"");
   key = key.trimmed();
   val = val.simplified();
   key = key.simplified();
   val = val.trimmed();
  // if(key == "cmd")
  //   key = "type";
   QString info =  "<" + key +">" + val + "</" + key + ">\n";

   return info;
}
//------------------------------------------------------------------
// обработка массива значений
// значение может быть числом или строкой!
// переданный пар -str -является сообщением json- выделим из него массив и
// отрежим часть - являющуюся выделенным массивом
QString TranslatorXML::array_process(QString key, QString & str)
  {
     QString res;
   // определим массив чисел перед нами или массив строк
     str = str.trimmed();
     str = str.simplified();
     QString cur_char = str.at(0);
     QString array;
     QStringList arr_list;

     if (cur_char == "\""){ // значит это массив строк
        do{                // в одном из элементов могут внутри находится символы "]"
           str.remove(0,1);          // удаляем открыв. кавычки
           int  pos = str.indexOf("\"");  // ищем закрыв. кавычку
           arr_list.append(str.left(pos)); //считываем данные внутри кавычек как элемент массива
           str.remove(0, pos+1);  // удалим то, что мы выделили как массив значений вместе с "
           cur_char = str.at(0);  // считываем след. символ в послед.
           if (cur_char==","){
               str.remove(0, 1);
               cur_char = str.at(0); // считываем след. символ в послед.
           }
         }while (cur_char!="]");
        str.remove(0, 1);            // удалим найденную ]
        if(arr_list.count()==0){
            res = res + "<" + key +">" +""+ "</" + key + ">\n";  // формируем ключ : значение для случая пустого массива чисел
        }
     }

     else if (cur_char.toInt()){  // перед нами числовой массив
         int  pos = str.indexOf("]");    // найдем конец числового массива значений
         QString array = str.left(pos);  // выделяем последовательность являющуюся массивом
         str.remove(0, pos+1);           // удалим то, что мы выделили как массив значений вместе с ]
         arr_list = array.split(",");    // массив значений
     }

     else if (cur_char == "]"){  // перед нами - пустой массив чисел
         str.remove(0, 1);   // убираем "]" из входящей последовательности
         res = res + "<" + key +">" + "</" + key + ">\n";  // формируем ключ : значение для случая пустого массива чисел
     }

        for (int i = 0; i <arr_list.count(); ++i){  // формируем массив значений
            res = res + "<" + key +">" + arr_list.at(i) + "</" + key + ">\n";
        }


      return res;
  }
//-------------------------------------------------------------------
//                    xml->json
//-------------------------------------------------------------------

//------------------------------------------------------------------
// основная процедура перевода сообщения xml->json
QByteArray TranslatorXML::translateToJson(const QByteArray & xmlPack)
{
   QString           str_x = xmlPack;    // принятая последовательность в формате xml
   QString           res_mess;           // общее результ.(переведенное в json формат) сообщение
   QVector <QString> vec_tag;             // вектор тегов

   str_x = str_x.remove("<message>\n");  // удаляем коревые теги
   str_x = str_x.remove("</message>");
   str_x = str_x.remove("\n");           // удаляем переход строки

       // обрабатываем сообщение без корневых тегов
   while (str_x.size()>0){
        // ищем очередной тег (откр. - (а)/закрыв. - (/а))
        int pos1 = str_x.indexOf("<");
        int pos2 = str_x.indexOf(">");

        QString val{""};        // переменная для хранения последовательности в котором имеется значение

        if(pos1 > 0)                                        // если символ "<" найден
          val = str_x.left(pos1);        // если тег откр. -> значения не будет, если тег закры - в val будет значение тега
        QString tag_name = str_x.mid(pos1+1, pos2 - pos1-1); // выделяем имя текущего тега
        tag_name = tag_name.trimmed();
        tag_name = tag_name.simplified();
       // if(tag_name == "type") tag_name = "cmd";
       // if (tag_name =="/type")tag_name = "/cmd";
        TagFeatures tag_feat = analyseTag(vec_tag, tag_name); // анализируем очередной тег в контексте уже имеющихся
        vec_tag.append(tag_name);                             // помещаем тег в вектор
        getTranslateItem(res_mess, tag_feat, val, tag_name);  // формируем элемент json послед.
        str_x = str_x.right(str_x.size() - pos2-1);           // убираем обработанную часть из принятого сообщения
   }

       // собираем сообщение целиком

   QString coma{};   // обрабатываем конверт. сообщение на предмент ненужных запятых
   do{  // убираем лишние запятые в конце
      coma = res_mess.right(1);
      if (coma ==",")
        res_mess = res_mess.left(res_mess.size()-1);
      coma = res_mess.right(1);
   }while(coma ==",");

   res_mess.insert(0,"{"); // помещаем в конвертированное сообщение корневую {
   res_mess = res_mess +"}"; // помещаем в конвертированное сообщение корневую }

   return res_mess.toUtf8();
}

//------------------------------------------------------------------
// анализ текущего тега (задание характеристик тега по данным вектора всех тегов) для перевода xml->json
// текущего тега пока нет в векторе тегов, он добавляется туда когда будет обработан
TagFeatures TranslatorXML::analyseTag(const QVector<QString>& vec_tag,
                                            QString cur_tag)
{
  TagFeatures feat;
  feat.open = !(cur_tag.contains("/"));    // определяем закрытый или открытый текущий тег
  feat.repeat = vec_tag.contains(cur_tag); // проверяем был ли уже такой же тег ранее (совпадение должно быть полным)

  QString tag_last{""};
  if(vec_tag.count()>0){
    tag_last = vec_tag.last();       // определяем - соседний тег имеет то же имя что и у текущего?

    if(tag_last.contains(cur_tag) || cur_tag.contains(tag_last))
        feat.pre_tag_same = true;
    else
        feat.pre_tag_same = false;

    if(!(tag_last.contains("/")))  // определяем соседний тег открыв. или закрыв.
        feat.pre_tag_open = true;

    for (int i = 0; i < vec_tag.count()-1; ++i){  // проверяем является ли соседний тег повторяющимся
        if (vec_tag.at(i) == tag_last){
            feat.pre_tag_repeat = true;
            break;
         }
    }
  }
  return feat;
}
//--------------------------------------------------------------------
// создание последов. json в зависимости от особенности текущего (считанного) тега ( для перевода xml->json)
// json последовательность передаем внуть что бы была возможность корректировать ее в процессе конвертации (добавл. ], например)
void TranslatorXML::getTranslateItem(QString &json,
                                           TagFeatures tag_feat,
                                           QString val,
                                           QString tag_name)
{
   bool ok1, ok2, ok3;       // флаги числового значения перем. val

       // определим какого типа значение ключа собираемся добавлять в сообщение
   if(val!=""){
      val.toInt(&ok1);
      val.toDouble(&ok2);
      val.toLong(&ok3);

      if((!ok1)&&(!ok2)&&(!ok3))         // не числовое значение (строка)
        val = "\""+val +"\"";           // строковое значение необходимо поместить в кавычки
   }

   // допустим что все значения ключей являются строками по умолчанию придадим им правильный тип в модуле ProtocolXXX
 //  val = "\""+val +"\"";              // строковое значение необходимо поместить в кавычки

   if (json.size() == 0){               // это первый элемент в сообщении т.к. json сообщение еще пустое
       json = "\"" + tag_name + "\":";  // он является первым ключ. в сообщении
   }
   else if(json.size() > 0){            // если элемент сообщения уже не первый

       // обработка ключ:значение
/*ключ :*/
   if ((tag_feat.open)&&           // тек.тег - открыв.
      (!tag_feat.pre_tag_open)&&   // сосед - закрыв.
      (!tag_feat.pre_tag_same)&&   // у соседа - другое имя
      (val =="")){                 // значение ключа не переданно
                   if(m_array_special_key.contains(tag_name))                //если имя тега входит в список ключей-массивов
                        json = json + "\"" + tag_name + "\":[";    // устанавливаем открыв. скобку массива
                   else
                        json = json + "\"" + tag_name + "\":";      // если нет , то нет
   }
/*знач,*/
   else if((!tag_feat.open)&&           // тек.тег - закрыв.)
          (tag_feat.pre_tag_open)&&     // сосед - открыт.
          (tag_feat.pre_tag_same)){     // сосед - с тем же имененм
                                        // значение ключа может быть пустым
           if (val.size() == 0){         // для пустого значение
               val = "\""+val +"\"";//,";  // если значение пустое "," после него не ставим , она будет поставлена ниже
            }
           QString cur = json.mid(json.size()-2,1);  // проверяем есть ли перед вносимсым значением символ - "]"(предпослед. символ)
                                                     //- его нужно передвинуть
           if(cur =="]"){                       // есть признак закр. массива
                json.remove(json.size()-2,1);         // удалим "]"
                json = json + val+"],";               // добавим на новом месте
                m_arr_end_pos = json.lastIndexOf("]");// сохраним текущее значение закрыв. массива в перем.
            }
            else     // если нет признака закр. массива в json - строке
               if(m_array_special_key.contains(tag_name)) {        // но имя тега входит в список ключей-массивов
                   json = json + val+"],";               // добаляем после внесенного значения закрыв. скобку массива
                   m_arr_end_pos = json.lastIndexOf("]");// сохраним текущее значение закрыв. массива в перем.
               }
               else                                      // имя тега не входит в список ключей-массивов
                   json = json + val+",";

   }
       // обработка объекта
/* нач.объекта.*/
    else if ((tag_feat.open)&&          // тек.тег - открыв.
            (tag_feat.pre_tag_open)&&   // сосед - открыт.
            (!tag_feat.pre_tag_same)&&  // у соседа - другое имя
            (val =="")){                // значение ключа не переданно
                   json = json + "{\"" + tag_name + "\":";
    }
/* окончание.объекта.*/
   else if ((!tag_feat.open)&&           // тек.тег - закрыв.
           (!tag_feat.pre_tag_same)&&    // у соседа - другое имя
           (!tag_feat.pre_tag_open)&&    // сосед - закрыв.
           (!tag_feat.repeat)){
           /*(val !="")){*/              // значения может не быть

                   QString coma = json.right(1);             // считываем последний символ из json послед.
                   if (coma ==",")
                       json.remove(json.size()-1, 1);       // удаляем последний символ(,)
                   if(m_array_special_key.contains(tag_name)){  //проверяем входит ли имя текущего тега в список ключей-массивов
                       json = json + val+"}],";
                       m_arr_end_pos = json.lastIndexOf("]");   // запоминаем новое положение окончание массива
                   }
                   else
                       json = json + val + "},";          // добавляем признак завершения объекта
   }
       // обработка массива
/* нач.масс.*/
   else if ((tag_feat.open)&&            // тек.тег - открыв.
           (tag_feat.repeat)&&           // тек.тег - повтор.
           (!tag_feat.pre_tag_open)&&    // сосед закрытый
           (tag_feat.pre_tag_same)&&     // c тем же именем
           (val =="")){                  // значение ключа не переданно
                   int pos  = json.indexOf(tag_name); // найдем первое вхождение тек. тега в сообщении (тег с данным именени первый будет всегда окрытым)
                   QString test{""};
                   if (pos >-1){   // найдено первое вхождение имени текущего тега в сообщении
                     test  = json.mid(pos, json.size() - pos); // считываем json послед. от имени тега до конца (для проверки наличия "["после ключа
                     int pos1 = test.indexOf(":");             // найдем ближайший к имени тега":"
                     if (pos1 >-1){           // если ":" найдено
                        QString test2 = test.mid (pos1, json.size() - pos1-1); // считываем test от : до конца
                        test2 = test2.trimmed();                // удаляем все незначащие пробелы
                        test2 = test2.mid(1,1);                 // считываем первый символ
                        if (test2 !="["){                       // если за "ключом :" нет "["
                           json.insert(pos + pos1+1,"[");       // добавляем признак начала массива
                        }
                        int pos1 = json.lastIndexOf("[");       // мы только что добавили [ в json послед.// был pos1
                        int pos2 = m_arr_end_pos;/*json.lastIndexOf("]");*/ // это значение было получено на одном из предыдущев шагов
                        if(pos1 < pos2)
                           json.remove(pos2,1);                 // удаляем ] от предыдущего значения массива

                        QString coma = json.right(1);           // проверка на последнюю ,
                        if (coma ==",")
                          json = json.left(json.size()-1); //удалить посл. ","
                        json = json + "],";   // переносим окончание массива в текущее положение
                                              //(возможно за массивом будут другие объекты поэтому ",")
                        m_arr_end_pos = json.lastIndexOf("]");   // запоминаем новое положение окончание массива
                      }   // если ":" найдено
                    }     // найдено первое вхождение имени текущего тега в сообщении
   }                      // обработка начала массива завершена
/* оконч.масс.*/
   else if ((!tag_feat.open)&&
           (tag_feat.repeat)&&
           (!tag_feat.pre_tag_same)&&
           (!tag_feat.pre_tag_open)){
                       /*(val !=""))*/
                        int pos = m_arr_end_pos; //json.lastIndexOf("]");
                        if(pos >-1){             // на предыдущих шагах мы установили окончание массива ]
                           json.remove(pos,1);   // удалим его от туда (])
                           //json.insert(pos,",");
                        }
                        pos = json.lastIndexOf(","); // последняя "," из ключ:значение
                        if (pos >-1)
                           json.remove(pos,1);
                        json = json + val +"}],";     //  завершаем массив
                        m_arr_end_pos = json.lastIndexOf("]"); // сохраняем положение окончание массива
    }    // окончание обработки завершения массива
 }       // если элемент сообщения уже не первый
}
//------------------------------------------------------------------------------
