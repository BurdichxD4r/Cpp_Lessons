#ifndef PLOTCONTROLLER_H
#define PLOTCONTROLLER_H

#include <httprequesthandler.h>
#include <QObject>

using namespace stefanfrings;

class PlotController : public HttpRequestHandler
{
    Q_OBJECT
public:
    PlotController(const QSettings* settings, QObject *parent = nullptr);
    void service(HttpRequest& request, HttpResponse& response);

private:

    /** Root directory of testing data */
    QString m_testroot;

    /** Encoding of text files */
    QString m_encoding;

     /** Name of file with testing data */
    QString m_plotname;


     /** Name of file with testing data */
    QMap<QString, QVector<float> > m_uvs_data_map; // данные для графика

    void    getDataList();   // считывание данных из файла  в перем. m_uvs_data_map
};

#endif // PLOTCONTROLLER_H
