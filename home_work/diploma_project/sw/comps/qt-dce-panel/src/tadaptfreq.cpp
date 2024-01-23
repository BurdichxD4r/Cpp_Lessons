#include <QDebug>
#include "tadaptfreq.h"
#include "math.h"

//#define		DO_LOG

TAdaptFreq::TAdaptFreq(QObject *parent, int num, int fNumber, int allF) : QObject(parent)
{
    isActive = true;
    startFreqNumber =  fNumber;
    allFreqNumber = allF;
    clearAllStatistics();

    setOfFreeFreqs = new QList<int>();
    for (int jj=startFreqNumber;jj<allFreqNumber;jj++) {
        setOfFreeFreqs->append(jj);
    }

    QString fname = QString("adapt_freq%1.txt").arg(num);
    qDebug() << "num="<<num<< "logname="<< fname;
    #ifdef	DO_LOG
        adaptf = newLogger(fname.toUtf8().data(),1,1);
    #else
        adaptf = newLogger(fname.toUtf8().data(),1,0);
    #endif
     strToFile(adaptf,1,tr("class TAdaptFreq").toUtf8().data(),1);
     intToFile(adaptf, 0, tr(" active freqs = ").toUtf8().data(),startFreqNumber, 5, 10,' ',0);
     intToFile(adaptf, 0, tr(" allFreqNumber = ").toUtf8().data(),allFreqNumber, 5, 10,' ',0);
     intToFile(adaptf, 0, tr(" Modem num = ").toUtf8().data(),num, 5, 10,' ',1);
}

TAdaptFreq::~TAdaptFreq()
{
    isActive = false;
}

void TAdaptFreq::initFreqStat(int fNumber,int allF)
{
    startFreqNumber =  fNumber;
    allFreqNumber = allF;
    clearAllStatistics();

    setOfFreeFreqs->clear();
    for (int jj=startFreqNumber;jj<allFreqNumber;jj++) {
        setOfFreeFreqs->append(jj);
    }
}

int TAdaptFreq::changeFrequenciesSet(int indChange,int oldFreq)
{
    int newFreq = setOfFreeFreqs->first(); // первая свободная частота
    setOfFreeFreqs->removeFirst();
    setOfFreeFreqs->append(oldFreq); // забракованную частоту в конец списка свободных
    freq_stat[indChange].summa_rs = 0;
    freq_stat[indChange].reject = false;
    freq_stat[indChange].rslist.clear();
    freq_stat[indChange].mxlist.clear();
    //freq_stat[indChange].rsgood.clear();
    return newFreq;
}

int TAdaptFreq::changeImplicitFreq(int indChange,int oldFreq,int frChange)
{
    int index=-1;
    for (int ii=0;ii<setOfFreeFreqs->count();ii++) {
        if (setOfFreeFreqs->at(ii) == frChange){
            index = ii;
            break;
        }
    }
    if (index>=0){
        setOfFreeFreqs->removeAt(index);
        setOfFreeFreqs->append(oldFreq); // забракованную частоту в конец списка свободных
        freq_stat[indChange].summa_rs = 0;
        freq_stat[indChange].reject = false;
        freq_stat[indChange].rslist.clear();
        freq_stat[indChange].mxlist.clear();
        //freq_stat[indChange].rsgood.clear();
    }

    return index;
}

void TAdaptFreq::set_allFreqNumber(int num)
{
    allFreqNumber = num;
    qDebug()<<"TAdaptFreq::set_allFreqNumber num="<<num;
 }

void TAdaptFreq::set_activeFreqNumber(int num)
{
    startFreqNumber = num;
    qDebug()<<"TAdaptFreq::set_activeFreqNumber num="<<num;
 }

void TAdaptFreq::clearAllStatistics()
{
    for(int ii=0;ii<32;ii++)
    {
        freq_stat[ii].indf =freq_stat[ii].number=freq_stat[ii].summa_snr=0;
        freq_stat[ii].not_decoded = freq_stat[ii].snr=freq_stat[ii].rs=freq_stat[ii].nodec=0;
        freq_stat[ii].rsMax = 0;
        freq_stat[ii].freqNum = 0;
        freq_stat[ii].summa_rs=0;
        freq_stat[ii].allow_reject = false;
        freq_stat[ii].reject = false;
        freq_stat[ii].rslist.clear();
        freq_stat[ii].mxlist.clear();
    }
}

void TAdaptFreq::allow_replace_freq(int ind,bool st)
{
    freq_stat[ind].allow_reject = st;
}

void TAdaptFreq::clearFreqStatistics(int fIndex)
{
//    freq_stat[fIndex].indf =freq_stat[fIndex].number=freq_stat[fIndex].summa_snr=0;
//    freq_stat[fIndex].not_decoded = freq_stat[fIndex].snr=freq_stat[fIndex].rs=freq_stat[fIndex].nodec=0;
//    freq_stat[fIndex].rsMax = 0;
//    freq_stat[fIndex].freqNum = 0;
    freq_stat[fIndex].summa_rs=0;
    freq_stat[fIndex].reject = false;
    freq_stat[fIndex].rslist.clear();
    freq_stat[fIndex].mxlist.clear();
    freq_stat[fIndex].rsgood.clear();  //?
}

Frequency_Stat* TAdaptFreq::getStatArray()
{
    return freq_stat;
}

void TAdaptFreq::addStatFreq(int err, int max, int ind, int ch)
{
    int no_dec=0;
    int err1,max1;
    //areRejectives=false;
    freq_stat[ind].freqNum = ch;
    freq_stat[ind].rsMax = max;

    int statLen=freq_stat[ind].rslist.count();
    if (statLen >= freqWindow){
        freq_stat[ind].rslist.removeFirst();
        freq_stat[ind].mxlist.removeFirst();
    }
    freq_stat[ind].rslist.append(err);
    freq_stat[ind].mxlist.append(max);

    int statLen1=freq_stat[ind].rsgood.count();
    if (statLen1 == freqWindow){
        freq_stat[ind].rsgood.removeFirst();
    }
    freq_stat[ind].rsgood.append(err);

    freq_stat[ind].summa_rs = 0;
    freq_stat[ind].summa_mx = 0;
    for (int ii=0;ii<freq_stat[ind].rslist.count();ii++){
        err1 = freq_stat[ind].rslist.at(ii);
        max1 = freq_stat[ind].mxlist.at(ii);
        freq_stat[ind].summa_rs += err1;
        freq_stat[ind].summa_mx += max1;
        if(err1 >= max1 ){
            no_dec++;
        }
    }
    freq_stat[ind].not_decoded = no_dec;
    if (no_dec >= freqWindow) freq_stat[ind].reject = true;
    else freq_stat[ind].reject = false;

    logStat(ind);
}

void TAdaptFreq::logStat(int index)
{
    strToFile(adaptf,1,tr(" ").toUtf8().data(),0);
    intToFile(adaptf, 0, tr(" ii=").toUtf8().data(),index, 3, 10,' ',0);
    intToFile(adaptf, 0, tr(" F=").toUtf8().data(),freq_stat[index].freqNum, 3, 10,' ',0);
    intToFile(adaptf, 0, tr(" count=").toUtf8().data(),freq_stat[index].rslist.count(), 3, 10,' ',0);
    intToFile(adaptf, 0, tr(" errRS=").toUtf8().data(),freq_stat[index].summa_rs, 3, 10,' ',0);
    intToFile(adaptf, 0, tr(" errMax=").toUtf8().data(),freq_stat[index].summa_mx, 3, 10,' ',0);
    intToFile(adaptf, 0, tr(" no_dec").toUtf8().data(),freq_stat[index].reject, 3, 10,' ',1);
}

bool TAdaptFreq::getStatValid(int index, bool *ok)
{
    bool res = false;
    if (freq_stat[index].rslist.count() >= freqWindow){
        if (freq_stat[index].rsgood.count() >= goodWindow)
            res = true;
    }
    int res0=0;
    for (int ii = 0; ii < freq_stat[index].rsgood.count(); ii++) {
        res0 = res0 + freq_stat[index].rsgood.at(ii);
    }
    if (res0 == 0) *ok = true;
    else *ok = false;
    return res;
}



