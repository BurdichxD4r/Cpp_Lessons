#include "plotcontroller.h"
#include "global.h"
#include <QFile>
#include <QJsonParseError>
#include <QJsonArray>
#include <QCoreApplication>
#include <QMap>
#include <QString>
#include <QJsonObject>
#include <QDir>
#include <QFileInfo>
#include "template.h"
#include <QDateTime>

//-------------------------------------------------------------------------
PlotController::PlotController(const QSettings* settings, QObject *parent)
    : HttpRequestHandler{parent}
{
    m_encoding=settings->value("encoding","UTF-8").toString();
    m_testroot=settings->value("path",".").toString();
    m_plotname=settings->value("plotName",".").toString();

    if(!(m_testroot.startsWith(":/") || m_testroot.startsWith("qrc://")))
    {
        // Convert relative path to absolute, based on the directory of the config file.
        #ifdef Q_OS_WIN32
            if (QDir::isRelativePath(m_testroot) && settings->format()!=QSettings::NativeFormat)
        #else
            if (QDir::isRelativePath(m_testroot))
        #endif
        {
            QFileInfo configFile(settings->fileName());
            m_testroot=QFileInfo(configFile.absolutePath(), m_testroot).absoluteFilePath();
        }
    }
    qDebug("PlotController: testroot=%s, encoding=%s plotname=%s",qPrintable(m_testroot),qPrintable(m_encoding),qPrintable(m_plotname));

}
//--------------------------------------------------------------------------
void PlotController::service(HttpRequest &request, HttpResponse &response)
{
    // считываем данные сессии
    HttpSession session=sessionStore->getSession(request,response,true);
    qDebug()<<session.getId();

    getDataList();

    response.setHeader("Content-Type", "text/html; charset=UTF-8");

    // формируем ответ используя темплейт:  plot_templ
    QByteArray language=request.getHeader("Accept-Language");

    Template t_jq=templateCache->getTemplate("jq_templ",language);  // загружаем билиотеку jQuery
    Template t_hch=templateCache->getTemplate("hc_templ",language); // load lib hightcharts
    Template t=templateCache->getTemplate("plot_templ",language);   // шаблон графика

    t.loop("row",m_uvs_data_map.count());   // кол-во uvs_dce

    QMap <QString, QVector<float>>::iterator it = m_uvs_data_map.begin();

    int i =0;
    for (;it!=m_uvs_data_map.end(); ++it){
        QString uvs_name = it.key();
        QString number = QString::number(i);
        t.setVariable("row"+number+".uvs_name",uvs_name);  // имя текущего графика
        QVector<float> arr_data = it.value();
        t.loop("row"+number +".arr", arr_data.count());
        for (int j =0; j<arr_data.count(); ++j){    // массив данных текущего графика
            QString arr_elem = QString::number(arr_data.at(j));
            QString num = QString::number(j);
            t.setVariable("row"+number+".arr"+num+".element",arr_elem);
        }
        ++i;
    }
    QByteArray summary_t = t_jq.toUtf8() + t_hch.toUtf8() +t.toUtf8();

    response.write(summary_t,true);

}
//--------------------------------------------------------------------------
void PlotController::getDataList()
{
    QFile dat_exch_file (m_testroot+"/"+m_plotname);
    QString data_str;
    QString error = "";

    m_uvs_data_map.clear();
  if (!dat_exch_file.exists()){
      qDebug()<<"File:test_modem_exch.txt - does not exist!";
    }
    if (!dat_exch_file.open(QIODevice::ReadOnly|QIODevice::Text)) // Проверяем, возможно ли открыть наш файл для чтения
            return;                                      // если это сделать невозможно, то завершаем функцию
       data_str = dat_exch_file.readAll();                         //считываем все данные из файла
    dat_exch_file.close();

    data_str.remove('\n');
    QStringList dataList = data_str.split(";");

    for (int i = 0; i < dataList.count(); ++i){
        if(dataList.at(i).length()>0){
           int pos = dataList.at(i).indexOf('=');
           QString key = dataList.at(i).left(pos);

           int pos_beg = dataList.at(i).indexOf('[');
           int pos_end = dataList.at(i).lastIndexOf(']');
            QString data_clear = dataList.at(i).mid(pos_beg+1, (pos_end - pos_beg)-1);
            QStringList arr_data_list = data_clear.split(',');
            QVector <float> vec_data;
            for (int j = 0; j < arr_data_list.count(); ++j){
               vec_data.push_back(arr_data_list.at(j).toFloat());
             }
            m_uvs_data_map[key]=vec_data;
        }
    }
}
//-------------------------------------------------------------------------------
