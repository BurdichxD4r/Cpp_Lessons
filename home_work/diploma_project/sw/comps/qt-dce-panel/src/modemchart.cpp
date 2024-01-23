#include "modemchart.h"
#include "dceplot.h"
#include <QLayout>

ModemChart::ModemChart(QWidget *parent) : QWidget(parent)
{

     modemPlot = new DCEPlot(this);
     statusBar = new QStatusBar(this);
     statusBar->showMessage(tr(" "));

     QVBoxLayout *layout = new QVBoxLayout;
     layout->addWidget(modemPlot);
     layout->addWidget(statusBar);

     setLayout(layout);
     connect(modemPlot, &DCEPlot::mouseDouble, this, &ModemChart::slotDoubleMousePress);
}

void ModemChart::setModemNumber(int num)
{
    modemNumber = num;
    readSettings();
}

void ModemChart::writeSettings()
{
    QString str;
    str.setNum(modemNumber);
    QSettings settings("inteltech","fhss-standalone");
    settings.setValue("SIZE"+str+"/geometry",this->saveGeometry());
}

void ModemChart::readSettings()
{
    QString str;
    str.setNum(modemNumber);
    QSettings settings("inteltech","fhss-standalone");
    this->restoreGeometry(settings.value("SIZE"+str+"/geometry").toByteArray());
}

void ModemChart::addData(int bfVal, float fsVal, int snrVal, int rsVal, int ufVal, QVector<std::complex<float>> cdVals)
{
    bfValue = bfVal;
    if (snrVal > snrUpper){
        snrUpper += 5.0f;
        setSNRrange(-0.78,snrUpper);
    }
    modemPlot->addDataToDCEplotAndReplot(bfVal, fsVal,snrVal, rsVal,ufVal, cdVals);
}

void ModemChart::replotData()
{
    modemPlot->replotAllPlots();
}

void ModemChart::setRSrange(double lower, double upper)
{
    modemPlot->setRSgraphRange(lower,upper);
}

void ModemChart::setBFrange(double lower, double upper)
{
    bfLower = lower;
    bfUpper = upper;
    modemPlot->setBFgraphRange(lower,upper);
}

void ModemChart::setSNRrange(double lower, double upper)
{
    modemPlot->setSNRgraphRange(lower,upper);
}


void ModemChart::closeEvent(QCloseEvent *event)
 {
         //writeSettings();
         QWidget::closeEvent(event);
         emit paintingIsOver(modemNumber);
         writeSettings();
         //qDebug() << "chart closeEvent";
 }

void ModemChart::slotDoubleMousePress(QMouseEvent *event)
{
//    qDebug() << "slotDoubleMousePress state=" << bfZoom<< event->pos().x() << event->pos().y()
//                  <<modemPlot->getCoordBFgraph(event->pos().y());
    Q_UNUSED(event)
    bfZoom = !bfZoom;
    if (bfZoom)
        modemPlot->setBFgraphRange(bfValue - 10,bfValue+10);
    else
        modemPlot->setBFgraphRange(bfLower,bfUpper);

    snrUpper = 31.0f;
    setSNRrange(-0.78,snrUpper);
}

void ModemChart::setProcentDecoded(int procent)
{
    QString str = QString("Декодировано %1%").arg(procent);
    statusBar->showMessage(str);
}

void ModemChart::setProcentInfo(QString str)
{
    statusBar->showMessage(str);
}


