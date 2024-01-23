#ifndef ADAPT_SPEED_H
#define ADAPT_SPEED_H

#include "clogger.h"
#include "slot_params.h"

#include <QObject>

// поиск свободного ПСП-канала
class TAdaptSpeed : public QObject
{
    Q_OBJECT
public:
    explicit TAdaptSpeed(QObject *parent = nullptr,int num=0,SLOT_VARIANT *var = nullptr);
    bool active = false;
    Speed_Stat* getStatResult();  // передача ссылки на  speed_stat[32]- массив статистик по ЧК
    void initAdaptation(int current_var); // инициация начала адаптации
private:
    SLOT_VARIANT *variants_v;
    CLogger adapts;

    int variantGroup=1; // номер группы в пределах которой происходит адаптация
    int curVariantIndex[32];   // текущий номер строки в массиве режимов(0-28)

    int groupIndexLow=0;  // нижний индекс группы(текущей)
    int groupIndexHigh=3; // верхний индекс группы

    int slot_total=0;    // общее число слотов,принятых после начала сбора статистики
    int curRange=32;
    float fractionSummary;

    const int range_200 = 32; // число слотов 200мс набора статистики до принятия решения
    const int range_50 = 128; // число слотов 50мс -"-

         // пороги принятия решения
    const float thresh_F = 0.9;
    const float thresh_G = 0.5;
    const float thresh_H = 0.1;

    int startFreqNumber=32;  //число активных частот ППРЧ
    Speed_Stat speed_stat[32];  // массив статистик в разрезе частотных каналов
    const int speedWindow = 10; // окно накопления статистик
    const int goodWindow = 3;    // окно для опр-ния хорошего канала

    void logConnectParams(int ind, float q);
    void calc_adapt_action(int ind); // расчет рекомендаций по адаптации в соответствии с порогами

    void logStat(int index);

signals:
    void procentDecoded(int);

public slots:
    void addStat(int err, int max, int ind, int ch);
    int getBaseIndex();  // вернуть индекс базового СКК, на который надо упасть в случае, если пропал канал
    int getStatWindow(); // возвращает ширину окна накопления статистики
    int getNewSpeedNumber(int fnum,int *speed_reset);  // получить новое значение скорости для канала fnum
    int getNewSpeedNumberFix(int fnum,int *speed_reset);  // для фиксированной частоты
    void renewSpeedNumberArray(int freqNum, int variant); // после смены скорости приема
    void clearStatistics(int fnum);
    void clearAllStatistics();
    bool getValidCondition(int var); // возвращает допустимость варианта var
    bool getLowSpeedCondition(int fIndex); // возвращает true, если скорость минимальна
    void setCurIndex(int find, int var);
    bool getStatValid(int index, bool *ok);  // возвращает факт достаточности статистики для канала index

};

#endif // ADAPT_SPEED_H
