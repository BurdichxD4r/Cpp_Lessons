#include <QtWidgets/QGridLayout>
#include <QSharedPointer>

#include "dceplot.h"
#include "qcustomplot.h"

DCEPlot::DCEPlot(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *vLayout = new QVBoxLayout(this);
    vLayout->setSpacing(6);
    vLayout->setContentsMargins(11, 11, 11, 11);
    vLayout->setObjectName(QStringLiteral("vLayout"));
    vLayout->setContentsMargins(0, 0, 0, 0);

    dcePlot = new QCustomPlot(this);
    vLayout->addWidget(dcePlot);

    dcePlot->plotLayout()->clear(); // удаляем координатную систему, создаваемую по-умолчанию
    QCPLayoutGrid *subLayout = new QCPLayoutGrid;
    dcePlot->plotLayout()->addElement(0, 0, subLayout);
    QCPAxisRect *HistoAxisRect = new QCPAxisRect(dcePlot);
    dcePlot->plotLayout()->addElement(1, 0, HistoAxisRect);
    dcePlot->plotLayout()->setRowStretchFactors({2,1});

    dcePlot->setObjectName(QStringLiteral("dcePlot"));
    QCPAxisRect *BFAxisRect = new QCPAxisRect(dcePlot);
    QCPAxisRect *FSAxisRect = new QCPAxisRect(dcePlot);
    QCPAxisRect *SNRAxisRect = new QCPAxisRect(dcePlot);
    QCPAxisRect *CDAxisRect = new QCPAxisRect(dcePlot);
    QCPAxisRect *RSAxisRect = new QCPAxisRect(dcePlot);
    QCPAxisRect *UFAxisRect = new QCPAxisRect(dcePlot);

    connect(dcePlot, &QCustomPlot::mouseDoubleClick, this, &DCEPlot::mouseDouble);

    subLayout->addElement(0, 0, BFAxisRect);
    subLayout->addElement(0, 1, FSAxisRect);
    subLayout->addElement(1, 0, SNRAxisRect);
    subLayout->addElement(1, 1, CDAxisRect);
    subLayout->addElement(2, 0, RSAxisRect);
    subLayout->addElement(2, 1, UFAxisRect);

    setupBarsGraphs(HistoAxisRect);

    QSharedPointer<QCPRange> yRangeBF(new QCPRange(-2,79));
    generalPlotsSetup(BFAxisRect, "BF", yRangeBF);

    QSharedPointer<QCPRange> yRangeFS(new QCPRange(-3.15,3));
    generalPlotsSetup(FSAxisRect, "FS", yRangeFS);

    QSharedPointer<QCPRange> yRangeSNR(new QCPRange(-0.78,31));
    generalPlotsSetup(SNRAxisRect, "SNR", yRangeSNR);

    QSharedPointer<QCPRange> xRangeCD(new QCPRange(-1.05,1.05));
    QSharedPointer<QCPRange> yRangeCD(new QCPRange(-1.05,1.05));
    generalPlotsSetup(CDAxisRect, "CD", yRangeCD, xRangeCD);

    QSharedPointer<QCPRange> yRangeRS(new QCPRange(-0.13,5.0));
    generalPlotsSetup(RSAxisRect, "RS", yRangeRS);

    QSharedPointer<QCPRange> yRangeUF(new QCPRange(-3.3,127));
    generalPlotsSetup(UFAxisRect, "UF", yRangeUF);

    correctTickValues();
    alignPlots();

    connect(this, &DCEPlot::sigReplot, this, &DCEPlot::replotAllPlots);
    setMinimumSize(300,450);
}

DCEPlot::~DCEPlot()
{

}

void DCEPlot::resizeEvent(QResizeEvent *event)
{
    auto oldSizeWidth = event->oldSize().width();
    auto newSizeWidth = event->size().width();
    if(newSizeWidth < 500 && oldSizeWidth >= 500) histogramChangeTicker(m_barsTicksStep16, m_barsLabelsStep16);
    if(newSizeWidth >= 500 && oldSizeWidth < 800) histogramChangeTicker(m_barsTicksStep8, m_barsLabelsStep8);
    if(newSizeWidth >= 800 && oldSizeWidth < 800) histogramChangeTicker(m_barsTicksStep4, m_barsLabelsStep4);
    QWidget::resizeEvent(event);
}

void DCEPlot::histogramChangeTicker(const QVector<double> &keys, const QVector<QString> &values)
{
    QSharedPointer<QCPAxisTickerText> textTicker(new QCPAxisTickerText);
    textTicker->addTicks(keys, values);
    m_greenBars->keyAxis()->setTicker(textTicker);
    m_grayBars->keyAxis()->setTicker(textTicker);
    replotAllPlots();
}

void DCEPlot::generalPlotsSetup(QCPAxisRect *AxisRect, QString yLabel, QSharedPointer<QCPRange> yRange, QSharedPointer<QCPRange> xRange)
{
    QCPGraph *graph = dcePlot->addGraph(AxisRect->axis(QCPAxis::atBottom), AxisRect->axis(QCPAxis::atLeft));
    graph->valueAxis()->setRange(*yRange.data());
    (xRange) ? graph->keyAxis()->setRange(*xRange.data()) : graph->keyAxis()->setRange(QCPRange(m_minXinitial,m_maxXinitial));
    graph->valueAxis()->setLabel(yLabel);
    graph->keyAxis()->setTicks(false);
    graph->keyAxis()->grid()->setVisible(false);
    graph->valueAxis()->grid()->setVisible(false);
    graph->valueAxis()->setSubTicks(false);
    graph->setLineStyle(QCPGraph::lsNone);
    graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,1));
    if(graph->valueAxis()->label() == "CD") {
        graph->valueAxis()->setTicks(false);
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssDisc,2));

        QPen pen(Qt::red, 0.8);
        QCPItemLine *horizontalLine = new QCPItemLine(dcePlot);
        horizontalLine->setClipAxisRect(AxisRect);
        horizontalLine->start->setAxes(AxisRect->axis(QCPAxis::atBottom),AxisRect->axis(QCPAxis::atLeft));
        horizontalLine->end->setAxes(AxisRect->axis(QCPAxis::atBottom),AxisRect->axis(QCPAxis::atLeft));
        horizontalLine->start->setCoords(-1,0);
        horizontalLine->end->setCoords(1,0);
        horizontalLine->setPen(pen);

        QCPItemLine *verticalLine = new QCPItemLine(dcePlot);
        verticalLine->setClipAxisRect(AxisRect);
        verticalLine->start->setAxes(AxisRect->axis(QCPAxis::atBottom),AxisRect->axis(QCPAxis::atLeft));
        verticalLine->end->setAxes(AxisRect->axis(QCPAxis::atBottom),AxisRect->axis(QCPAxis::atLeft));
        verticalLine->start->setCoords(0,-1);
        verticalLine->end->setCoords(0,1);
        verticalLine->setPen(pen);
    }
}

void DCEPlot::setupBarsGraphs(QCPAxisRect *AxisRect)
{
    // Prepare Data
    for(int i = 0; i < 256; ++i){
       m_keyBars.push_back(i);
       m_grayBarsData.push_back(0);
       m_greenBarsData.push_back(0);
    }
    // Setup histogram GUI
    m_grayBars = new QCPBars(AxisRect->axis(QCPAxis::atBottom), AxisRect->axis(QCPAxis::atLeft));
    m_grayBars->valueAxis()->setLabel("SNR");
    m_grayBars->keyAxis()->grid()->setVisible(false);
    m_grayBars->keyAxis()->setRange(QCPRange(-0.5, 255.5));
    m_grayBars->valueAxis()->grid()->setVisible(false);
    m_grayBars->valueAxis()->setSubTicks(false);
    m_grayBars->valueAxis()->setRange(QCPRange(0,31));
    m_grayBars->setPen(Qt::NoPen);
    m_grayBars->setBrush(Qt::lightGray);
    m_grayBars->setWidth(50/(double)m_grayBarsData.size());
    m_grayBars->setAntialiased(false);

    m_greenBars = new QCPBars(AxisRect->axis(QCPAxis::atBottom), AxisRect->axis(QCPAxis::atLeft));
    m_greenBars->valueAxis()->setLabel("SNR");
    m_greenBars->keyAxis()->grid()->setVisible(false);
    m_greenBars->keyAxis()->setRange(QCPRange(-0.5, 255.5));
    m_greenBars->valueAxis()->grid()->setVisible(false);
    m_greenBars->valueAxis()->setSubTicks(false);
    m_greenBars->valueAxis()->setRange(QCPRange(0,31));
    m_greenBars->setPen(Qt::NoPen);
    m_greenBars->setBrush(QColor(144,238,144));
    m_greenBars->setWidth(50/(double)m_greenBarsData.size());
    m_greenBars->setAntialiased(false);
    // Add data to histogram
    m_grayBars->setData(m_keyBars, m_grayBarsData, true);
    m_greenBars->setData(m_keyBars, m_greenBarsData, true);
}

void DCEPlot::alignPlots()
{
    QCPMarginGroup *group = new QCPMarginGroup(dcePlot);
    dcePlot->axisRect(0)->setMarginGroup(QCP::msLeft, group); // axisRect(0) - Histogram axis rect
    dcePlot->axisRect(1)->setMarginGroup(QCP::msLeft, group); // axisRect(1) - BF axis rect
    dcePlot->axisRect(3)->setMarginGroup(QCP::msLeft, group); // axisRect(3) - SNR axis rect
    dcePlot->axisRect(5)->setMarginGroup(QCP::msLeft, group); // axisRect(5) - RS axis rect

    dcePlot->axisRect(2)->setMarginGroup(QCP::msLeft, group); // axisRect(2) - FS axis rect
    dcePlot->axisRect(4)->setMarginGroup(QCP::msLeft, group); // axisRect(4) - CD axis rect
    dcePlot->axisRect(6)->setMarginGroup(QCP::msLeft, group); // axisRect(6) - UF axis rect
}

void DCEPlot::correctTickValues()
{
//    QVector<double> bfTicks;
//    QVector<QString> bfLabels;
//    bfTicks << 0 << 20 << 40 << 60 << 79;
//    bfLabels << "0" << "20" << "40" << "60" << "79";
//    QSharedPointer<QCPAxisTickerText> bfTextTicker(new QCPAxisTickerText);
//    bfTextTicker->addTicks(bfTicks, bfLabels);
//    dcePlot->graph(0)->valueAxis()->setTicker(bfTextTicker);

    QVector<double> ufTicks;
    QVector<QString> ufLabels;
    ufTicks << 0 << 50 << 100 << 150 << 200 << 250;//127;
    ufLabels << "0" << "50" << "100" << "150" << "200" << "250";//"127";
    QSharedPointer<QCPAxisTickerText> uftextTicker(new QCPAxisTickerText);
    uftextTicker->addTicks(ufTicks, ufLabels);
    dcePlot->graph(5)->valueAxis()->setTicker(uftextTicker);

    setUpHistogramTicks();
}

void DCEPlot::setUpHistogramTicks()
{
    m_barsTicksStep4 << 0;
    m_barsLabelsStep4 << "0";
    for(int i = 7; i <= m_numOfChannels - 1; i += 8){
        m_barsTicksStep4 << i;
        m_barsLabelsStep4 << QString::number(i);
    }
    m_barsTicksStep8 << 0;
    m_barsLabelsStep8 << "0";
    for(int i = 15; i <= m_numOfChannels - 1; i += 16){
        m_barsTicksStep8 << i;
        m_barsLabelsStep8 << QString::number(i);
    }
    m_barsTicksStep16 << 0;
    m_barsLabelsStep16 << "0";
    for(int i = 31; i <= m_numOfChannels - 1; i += 32){
        m_barsTicksStep16 << i;
        m_barsLabelsStep16 << QString::number(i);
    }
    histogramChangeTicker(m_barsTicksStep16, m_barsLabelsStep16); // В предположении, что изначально окно dce plot всегда открывается самым маленьким
}

void DCEPlot::addDataToDCEplotAndReplot(int bfVal, float fsVal, int snrVal, int rsVal, int ufVal, QVector<std::complex<float>> cdVals)
{
    updateHistogram(ufVal, snrVal);
    updateNonCDPlot(dcePlot->graph(0), bfVal);   // graph(0) - BF graph
    updateNonCDPlot(dcePlot->graph(1), fsVal);   // graph(1) - FS graph
    updateNonCDPlot(dcePlot->graph(2), snrVal);  // graph(2) - SNR graph
    updateCDPlot(dcePlot->graph(3), cdVals);     // graph(3) - CD graph
    updateNonCDPlot(dcePlot->graph(4), rsVal);   // graph(4) - RS graph
    updateNonCDPlot(dcePlot->graph(5), ufVal);   // graph(5) - UF graph
    emit sigReplot();
    ++m_time;
}

void DCEPlot::addDataToDCEplot(int bfVal, float fsVal, int snrVal, int rsVal, int ufVal, QVector<std::complex<float>> cdVals)
{
    updateHistogram(ufVal, snrVal);
    updateNonCDPlot(dcePlot->graph(0), bfVal);  // graph(0) - BF graph
    updateNonCDPlot(dcePlot->graph(1), fsVal);  // graph(1) - FS graph
    updateNonCDPlot(dcePlot->graph(2), snrVal); // graph(2) - SNR graph
    updateCDPlot(dcePlot->graph(3), cdVals);    // graph(3) - CD graph
    updateNonCDPlot(dcePlot->graph(4), rsVal);  // graph(4) - RS graph
    updateNonCDPlot(dcePlot->graph(5), ufVal);  // graph(5) - UF graph
    ++m_time;
}

void DCEPlot::setRSgraphRange(double lower, double upper)
{
    dcePlot->graph(4)->valueAxis()->setRange(lower, upper);
    emit sigReplot();
}


void DCEPlot::setBFgraphRange(double lower, double upper)
{
    dcePlot->graph(0)->valueAxis()->setRange(lower, upper);
    emit sigReplot();
}

void DCEPlot::setSNRgraphRange(double lower, double upper)
{
    dcePlot->graph(2)->valueAxis()->setRange(lower, upper);
    emit sigReplot();
}

double DCEPlot::getCoordBFgraph(int pos)
{
    return dcePlot->yAxis->pixelToCoord(pos);
}

void DCEPlot::replotAllPlots(){
    dcePlot->replot(QCustomPlot::rpQueuedReplot);
}

template<class T>
void DCEPlot::updateNonCDPlot(QCPGraph *graph, T val)
{
    graph->addData(m_time,static_cast<double>(val));
    if (m_time > 200) {
        // Удаляем старые значения на графике и сдвигаем график вправо
        graph->data()->removeBefore(m_time - 200);
        graph->keyAxis()->setRange(m_time - 200, m_time);
    }
}

void DCEPlot::updateCDPlot(QCPGraph *graph,QVector<std::complex<float>> &cdVals)
{
    if(m_curCDPlotwidth != dcePlot->axisRect(4)->width() || m_curCDPlotheight != dcePlot->axisRect(4)->height()){
        graph->keyAxis()->setScaleRatio(graph->valueAxis(),1);
        m_curCDPlotwidth = dcePlot->axisRect(4)->width();
        m_curCDPlotheight = dcePlot->axisRect(4)->height(); // dcePlot->axisRect(4) - система координат CD графика
    }
    graph->data()->clear();
    if(cdVals.size() != 0){
        float maxVal = -1;
        for(int i =0; i < cdVals.size(); ++i){
            qreal temp = static_cast<double>(cdVals[i].real()*cdVals[i].real() +
                                            cdVals[i].imag()*cdVals[i].imag());
            float curVal = static_cast<float>(qSqrt(temp));
            if(curVal > maxVal)
                maxVal = curVal;
        }
        for(std::complex<float> &num : cdVals){
            num.real(num.real()/maxVal);
            num.imag(num.imag()/maxVal);
            graph->addData(static_cast<double>(num.real()),static_cast<double>(num.imag()));
        }
    }
}

void DCEPlot::updateHistogram(int ufVal, int snrVal)
{
    for(int ix = 0; ix < m_numOfChannels; ++ix){
        qint32 prevCounter = m_currentChannelsState[ix];
        if(prevCounter > 0) --m_currentChannelsState[ix];
        if(ix == ufVal){
            m_grayBarsData[ufVal] = 0;
            m_greenBarsData[ufVal] = snrVal;
            m_currentChannelsState[ufVal] = m_maxCounterValue;
            continue;
        }
        if(prevCounter == 1){
           m_grayBarsData[ix] =  m_greenBarsData[ix];
           m_greenBarsData[ix] = 0;
        }
    }
    // Add data to histogram
    m_grayBars->setData(m_keyBars, m_grayBarsData, true);
    m_greenBars->setData(m_keyBars, m_greenBarsData, true);
}
