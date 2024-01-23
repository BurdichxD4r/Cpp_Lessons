#ifndef TADAPTFREQ_H
#define TADAPTFREQ_H

#include "clogger.h"
#include "slot_params.h"

#include <QObject>

class TAdaptFreq : public QObject
{
    Q_OBJECT
public:
    explicit TAdaptFreq(QObject *parent = nullptr,int num=0,int fNumber=32,int allF=128);
    ~TAdaptFreq();

    bool isActive = false;
    Frequency_Stat* getStatArray();  // передача ссылки на  freq_stat[32]- массив статистик по ЧК
    int changeFrequenciesSet(int indChange, int oldFreq); // возвращает новое значение ЧК
    int changeImplicitFreq(int indChange, int oldFreq, int frChange); //возвращает >=0, если замена возможна

private:
    CLogger adaptf;

    int startFreqNumber=32;  //число активных частот ППРЧ
    int allFreqNumber = 128;   // общее число частот
    Frequency_Stat freq_stat[32];  // массив статистик по ЧК
    const int freqWindow = 3; // окно накопления статистики по частоте
    const int goodWindow = 3;    // окно для опр-ния хорошего канала
    QList<int> *setOfFreeFreqs; // свободные частоты

    void logStat(int index);

signals:

public slots:

    void addStatFreq(int err, int max,int ind, int ch); // сбор статистики по каналам
    void clearAllStatistics();   // очистка всего массива статистики
    void clearFreqStatistics(int fIndex);   // очистка статистики для 1-ой частоты
    void set_allFreqNumber(int num);
    void set_activeFreqNumber(int num);
    void allow_replace_freq(int ind, bool st);
    void initFreqStat(int fNumber=32, int allF=128);
    bool getStatValid(int index,bool *ok);

};

#endif // TADAPTFREQ_H
