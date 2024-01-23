#ifndef TBESTFREQ_H
#define TBESTFREQ_H

#include <QObject>

#include "fhssslot.h"
#include "clogger.h"
#include "slot_params.h"

class TBestFreq : public QObject
{
    Q_OBJECT
public:
    explicit TBestFreq(QObject *parent = nullptr);

private:
    CLogger belog;
    int frequencyNumber = 16;
    bool statisticsMode=false;  // режим сбора статистики по ЧК
    Quality_Stat qu_stat[32];  // массив статистик по ЧК для входа в НРЧ
    QList<Quality_Stat> stat;
    int statSlotCounter;       // счетчик слотов при наборе статистики
    int number_of_counts;  // число слотов для получения итоговой статистики по ЧК

    void quicksortList(int p, int r);
    int  partitionList(int a, int b);

signals:
    void bestFrequency(int num1,int num2);

public slots:
    void bestFrequencyCalculate();
    void setStatisticMode(bool mode);
    void setLogNuber(int num);
    void setStatCollectMode(int fnum);
    void collecting_statistics_on_frequencies(FhssSlot *rxFslot,int chan,quint8 mn); // сбор статистики по частотам

};

#endif // TBESTFREQ_H
