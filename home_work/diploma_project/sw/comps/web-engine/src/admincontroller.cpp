#include "admincontroller.h"
#include "global.h"
#include <QDateTime>

extern SecureService * secureService;
//--------------------------------------------------------------------
AdminController::AdminController(QObject *parent)
: HttpRequestHandler{parent}
{

}
//---------------------------------------------------------------------
void AdminController::service(HttpRequest &request, HttpResponse &response)
{
    // создаем список пользователей системы

    QVariantMap  user_list;// = secureService->getUserDataFromJsonObj();

    // считываем данные сессии
    HttpSession session=sessionStore->getSession(request,response,true);
    qDebug()<<session.getId();

    response.setHeader("Content-Type", "text/html; charset=UTF-8");

    // формируем ответ используя темплейт:  admin_templ
    QByteArray language=request.getHeader("Accept-Language");
    Template t=templateCache->getTemplate("admin_templ",language);

    if (session.contains("username")) {           // аутентификация пользователя пройдена
      if(!session.contains("user_list")){         // зашли первый раз, список пользов. еще не загружали в объект сессии

        user_list= secureService->getUserDataFromJsonObj(); // данные пользователей подгружаем из файла
                  // параметры, помещенные в объект сессии при аутентификации
        QByteArray username=session.get("username").toByteArray();
        QTime logintime=session.get("logintime").toTime();
                     // установка переменных в темплейте
        t.loop("row",/*user_count*/user_list.count());         // передаем кол-во пользователей в списке в переменную темплейта
        t.setVariable("username",username);
        t.setVariable("logintime",logintime.toString("HH:mm:ss"));

        // организуем цикл по всем значениям списка пользователей
        // для передачи всего списка в темплейт
        int i = 0;
        foreach(QString login, user_list.keys()){          

           QString user_login = login;
           QString user_pass = user_list.value(login).toString();

               QString number = QString::number(i);
               QString num = QString::number(i+1);
               t.setVariable("row"+number+".number",num);
               t.setVariable("row"+number+".name",user_login);
               t.setVariable("row"+number+".pass",user_pass);
               ++i;
        }
        session.set("user_list",  user_list);   // после первого входа на странцу помещаем данные пользователей в перем. сесии
        response.write(t.toUtf8(),true);
     }
      else{    // зашли повторно, список пользов. загружен в объект сессии
          QByteArray username=session.get("username").toByteArray();
          QTime logintime=session.get("logintime").toTime();
          user_list = session.get("user_list").toMap(); // список пользователей загружаем из объекта сессии
             // данные передаются в темплейт
          t.setVariable("username",username);
          t.setVariable("logintime",logintime.toString("HH:mm:ss"));
            // возможно пользователь уже изменил часть или все значения настроек
            // эти изменения должны быть обработаны сервером и записаны в файл
          m_login_ch = request.getParameter("login_u");  // измененные данные пользователя
          m_pass_ch = request.getParameter("pass_u");
          m_num_ch = request.getParameter("num_u").toInt();  // номер пользователя который был отредактирован

          if (request.getBody().count()==0){        // тело запроса  не содержит данные           

              t.loop("row",user_list.count());

            // t.loop("row",user_list.count());       // передаем кол-во пользователей в списке
                 // организуем цикл по всему списку пользователей которое загрузили в сессию
             int i=0;
             foreach(QString login, user_list.keys()){                

                QString user_login = login;
                QString user_pass = user_list.value(login).toString();

                QString number = QString::number(i);
                QString num = QString::number(i+1);
                t.setVariable("row"+number+".number",num);
                t.setVariable("row"+number+".name",user_login);
                t.setVariable("row"+number+".pass",user_pass);
                ++i;

             }
             response.write(t.toUtf8(),true);

          }
          // после изменений внесенных пользоваетлем в браузере
          else{    // если тело запроса содержит данные (в том числе параметры запроса)
                   // данные пользователя загружены в переменную сессии
          //if (request.getBody().count()!=0){
              QList <QString> keys_old = user_list.keys();  // значение ключей из объекта сесси
              QString old_key;
              if(m_num_ch <= user_list.count()){     // если пользователь из списка удалялся или изменялся, но не добавлялся
                  old_key=keys_old.at(m_num_ch - 1);
                  user_list.remove(old_key);         // удаляем редактируемую запись из списка
               }
              if((m_login_ch!="")&&(m_pass_ch!=""))  // если параметры из запроса не пустые
                 user_list.insert(m_login_ch, m_pass_ch);   // добавляем пользователя (изменяемого или вновь созданного) в список пользователей

              session.set("user_list",  user_list);   // изменненный список пользователей заносим в объект сессии

              t.loop("row",user_list.count());        // передаем кол-во пользователей в списке
                                                      // организуем цикл по всем пользователям для передачи в темплейт
              int i=0;
              foreach(QString login, user_list.keys()){

                 QString user_login = login;
                 QString user_pass = user_list.value(login).toString();

                 QString number = QString::number(i);
                 QString num = QString::number(i+1);
                 t.setVariable("row"+number+".number",num);
                 t.setVariable("row"+number+".name",user_login);
                 t.setVariable("row"+number+".pass",user_pass);
                 ++i;
              }

              QJsonObject obj_secure;
              secureService->setUserList(user_list, obj_secure);
              secureService->saveUserListToFile(obj_secure);
              response.write(t.toUtf8(),true);      // сохраняем изменненые данные пользователя в файле
          }
       }

    }
}

//---------------------------------------------------------------------------------------
