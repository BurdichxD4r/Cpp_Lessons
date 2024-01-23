#include "tbestfreq.h"

//#define		DO_LOG

TBestFreq::TBestFreq(QObject *parent) : QObject(parent)
{

}

void TBestFreq::setLogNuber(int num)
{
    QString fname = QString("best_fr%1.txt").arg(num);
    #ifdef	DO_LOG
        belog = newLogger(fname.toUtf8().data(),1,1);
    #else
        belog = newLogger(fname.toUtf8().data(),1,0);
    #endif
     intToFile(belog,1,tr("class TBestFreq ").toUtf8().data(),num, 0, 10,' ',1);
}

void TBestFreq::setStatisticMode(bool mode)
{
    statisticsMode = mode;
}

void TBestFreq::setStatCollectMode(int fnum)
{
    if(!statisticsMode){
        frequencyNumber = fnum;
        number_of_counts = 320;
        statSlotCounter = 0;
        statisticsMode=true;
        memset(qu_stat, 0, 32*sizeof(Quality_Stat));
        intToFile(belog,1,tr("  statisticsMode==  ").toUtf8().data(),statisticsMode, 5, 10,' ',1);
    }
}

void TBestFreq::collecting_statistics_on_frequencies(FhssSlot *rxFslot, int chan, quint8 mn)
 {
    if(statisticsMode){
        statSlotCounter++;
        qu_stat[chan].freqNum = chan;
        qu_stat[chan].number++;
        qu_stat[chan].summa_snr = qu_stat[chan].summa_snr + rxFslot->getSNR();
        qu_stat[chan].summa_rs = qu_stat[chan].summa_rs + rxFslot->getRSerr();
        if(rxFslot->getRSerr()>rxFslot->getmaxRSerr()-1) qu_stat[chan].not_decoded++;
        //qDebug() << "collecting_statistics" << chan<<statSlotCounter<<number_of_counts;
        intToFile(belog,1,tr("  collecting_stat chan=  ").toUtf8().data(),chan, 5, 10,' ',0);
        intToFile(belog,0,tr("  counter=  ").toUtf8().data(),statSlotCounter, 5, 10,' ',0);
        intToFile(belog,0,tr("  porog=  ").toUtf8().data(),number_of_counts/2, 5, 10,' ',1);
        if (statSlotCounter >= number_of_counts/2){
            if (mn == 2) bestFrequencyCalculate();
        }
    }
 }

void TBestFreq::bestFrequencyCalculate()
 {
     int res1 = -1;
     int res2 = -1;
     stat.clear();
     int not_decod_min=number_of_counts+1;
     for(int ii=0;ii<frequencyNumber;ii++){
         if (qu_stat[ii].number > 0){
            qu_stat[ii].snr = qu_stat[ii].summa_snr/qu_stat[ii].number;
            qu_stat[ii].rs = static_cast<float>(qu_stat[ii].summa_rs)/qu_stat[ii].number;
            if (qu_stat[ii].not_decoded < not_decod_min) not_decod_min = qu_stat[ii].not_decoded;
         }
     }
     for(int ii=0;ii<frequencyNumber;ii++){
         if (qu_stat[ii].number > 0){
            if (qu_stat[ii].not_decoded==not_decod_min){
                stat.append(qu_stat[ii]);
            }
         }
     }
     if (!stat.isEmpty()){
         quicksortList(0, stat.count() - 1);
         strToFile(belog,1,tr(" stat=").toUtf8().data(),1);
         for(int ii=0;ii<stat.count();ii++){
             intToFile(belog,0,tr("   num =  ").toUtf8().data(),stat.at(ii).number, 5, 10,' ',0);
             intToFile(belog,0,tr("   Fnum =  ").toUtf8().data(),stat.at(ii).freqNum, 5, 10,' ',0);
             floatToFile(belog,0,tr("  snr,db =  ").toUtf8().data(),stat.at(ii).snr,0,2,'f',0);
             floatToFile(belog,0,tr("  rs =  ").toUtf8().data(),stat.at(ii).rs,0,2,'f',0);
             intToFile(belog,0,tr("   not_decod =  ").toUtf8().data(),stat.at(ii).not_decoded, 5, 10,' ',1);
         }

         res1 = res2 = stat.last().freqNum;
         int len = stat.length();
         if (len > 1){  // для случая дуплекса д.б. две разных частоты
             res2 = stat.at(len-2).freqNum;
         }
         emit bestFrequency(res1,res2);
     }
 }

 void TBestFreq::quicksortList(int p, int r){
     if (p < r) {
         int q = partitionList(p, r);
         quicksortList(p, q - 1);
         quicksortList(q + 1, r);
     }
 }

 int  TBestFreq::partitionList(int a, int b){
     static int cc = 0;
     int k = a - 1;
     float x = stat.at(b).snr;
     for (int j = a; j < b; j++){
         cc++;
         if ( stat.at(j).snr <= x){
             k++;
             if (k != j){
                 stat.swap(k, j);
             }
         }
     }
     if (b != k + 1){
         stat.swap(b, k+1);
     }
     return k + 1;
 }
