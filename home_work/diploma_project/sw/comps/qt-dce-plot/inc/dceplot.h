#ifndef DCEPLOT_H
#define DCEPLOT_H

#include <QtWidgets/QWidget>
#include <complex>

class QVBoxLayout;
class QCustomPlot;
class QCPRange;
class QCPGraph;
class QCPAxisRect;
class QCPBars;

class DCEPlot : public QWidget
{
    Q_OBJECT
public:
    explicit DCEPlot(QWidget *parent = nullptr);
    ~DCEPlot();
    void resizeEvent(QResizeEvent *event) override;
    void addDataToDCEplotAndReplot(int bfVal, float fsVal, int snrVal, int rsVal, int ufVal, QVector<std::complex<float>> cdVals);
    void addDataToDCEplot(int bfVal, float fsVal, int snrVal, int rsVal, int ufVal, QVector<std::complex<float>> cdVals);

private:
    void generalPlotsSetup(QCPAxisRect *AxisRect, QString yLabel, QSharedPointer<QCPRange> yRange, QSharedPointer<QCPRange> xRange = nullptr);
    void setupBarsGraphs(QCPAxisRect *AxisRect);
    void correctTickValues();
    void histogramChangeTicker(const QVector<double> &keys, const QVector<QString> &values);
    void setUpHistogramTicks();
    void alignPlots();
    void updateCDPlot(QCPGraph *graph,QVector<std::complex<float>> &cdVals);
    template<class T> void updateNonCDPlot(QCPGraph *customPlot, T val);
    void updateHistogram(int ufVal, int snrVal);

private:
    QCustomPlot *dcePlot;
    QCPBars *m_grayBars;
    QCPBars *m_greenBars;
    QVector<double> m_keyBars, m_grayBarsData, m_greenBarsData;
    QVector<double> m_barsTicksStep4, m_barsTicksStep8, m_barsTicksStep16;
    QVector<QString> m_barsLabelsStep4, m_barsLabelsStep8, m_barsLabelsStep16;
    const qint32 m_maxCounterValue = 128; // 4*32, где 32 - max кол-во каналов модема
    static constexpr qint32 m_numOfChannels = 256; // 128 - max кол-во каналов в системе
    qint32 m_currentChannelsState[m_numOfChannels] = {0};

    double m_minXinitial = 0;
    double m_maxXinitial = 200;
    int m_curCDPlotwidth = 0;
    int m_curCDPlotheight = 0;

    int m_time = 0;

signals:
    void sigReplot();
    void mouseDouble(QMouseEvent *event) ;

public slots:
    void setRSgraphRange(double lower, double upper);
    void setBFgraphRange(double lower, double upper);
    void setSNRgraphRange(double lower, double upper);
    void replotAllPlots();
    double getCoordBFgraph(int pos);
};

#endif // DCEPLOT_H
