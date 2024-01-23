#ifndef MODEMCHART_H
#define MODEMCHART_H

#include <QtGui>
#include <QtWidgets>
#include <QDialog>
#include <complex>

class DCEPlot;

class ModemChart : public QWidget
{
    Q_OBJECT
public:
    explicit ModemChart(QWidget *parent = nullptr);
    void setModemNumber(int);
    void addData(int bfVal, float fsVal, int snrVal, int rsVal, int ufVal, QVector<std::complex<float>> cdVals);
    void replotData();

private:
     DCEPlot *modemPlot;
     QStatusBar *statusBar;
     int modemNumber; // номер модема
     double bfLower = 0;  // диапазон вертикальной оси графика BestFrame
     double bfUpper = 80;
     int bfValue = 40;   // текущее значение BestFrame
     bool bfZoom=false;  // включение режима лупы на графике BF
     double snrUpper = 31;

     void writeSettings();
     void readSettings();

signals:
     void paintingIsOver(int);

private slots:
    void slotDoubleMousePress(QMouseEvent * event);

public slots:
     void setRSrange(double lower, double upper);
     void setBFrange(double lower, double upper);
     void setSNRrange(double lower, double upper);
     void setProcentDecoded(int procent);
     void setProcentInfo(QString str);
protected:
    void closeEvent(QCloseEvent *event);
};

#endif // MODEMCHART_H
