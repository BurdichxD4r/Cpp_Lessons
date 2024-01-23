#include <QDebug>
#include "adapt_speed.h"
#include "math.h"

//#define		DO_LOG

TAdaptSpeed::TAdaptSpeed(QObject *parent, int num, SLOT_VARIANT *var) : QObject(parent)
{
    variants_v = var;
    for(int ii=0;ii<32;ii++){
        clearStatistics(ii);
        speed_stat[ii].rsgood.clear();
    }
    QString fname = QString("adapt_speed%1.txt").arg(num);
    qDebug() << "num="<<num<< "logname="<< fname;
    #ifdef	DO_LOG
        adapts = newLogger(fname.toUtf8().data(),1,1);
    #else
        adapts = newLogger(fname.toUtf8().data(),1,0);
    #endif
     strToFile(adapts,1,tr("class TAdaptSpeed").toUtf8().data(),1);
     intToFile(adapts, 0, tr(" num[] = ").toUtf8().data(),variants_v[num-1].num, 5, 10,' ',1);
}

void TAdaptSpeed::initAdaptation(int current_var)
{
    active = true;
    variantGroup = variants_v[current_var].section;
    for(int ii=0;ii <32;ii++){
        curVariantIndex[ii] = current_var;
    }

    for(int jj = current_var;jj >= 0;jj--){
        if(variantGroup != variants_v[jj].section){
            break;
        }
        groupIndexLow = jj;
    }
    for(int jj = current_var;jj < 29;jj++){
        if(variantGroup != variants_v[jj].section){
            break;
        }
        groupIndexHigh = jj;
    }


    intToFile(adapts, 0, tr(" curVariantIndex=").toUtf8().data(),curVariantIndex[0], 3, 10,' ',0);
    intToFile(adapts, 0, tr(" groupIndexLow=").toUtf8().data(),groupIndexLow, 3, 10,' ',0);
    intToFile(adapts, 0, tr(" groupIndexHigh=").toUtf8().data(),groupIndexHigh, 3, 10,' ',1);
    if (variants_v[current_var].slot_time == 200) curRange = range_200;
    else curRange = range_50;
    slot_total = 0;
    fractionSummary = 0;
}

void TAdaptSpeed::clearStatistics(int fnum)
{
    speed_stat[fnum].freqNum = 0;
    speed_stat[fnum].rsMax = 0;
    speed_stat[fnum].summa_rs = 0;
    speed_stat[fnum].summa_mx = 0;
    speed_stat[fnum].not_decoded = 0;
    speed_stat[fnum].speed_advise = THE_SAME_SPEED;
    speed_stat[fnum].reject = false;
    speed_stat[fnum].allow_reject = false;
    speed_stat[fnum].rslist.clear();
    speed_stat[fnum].mxlist.clear();
}

void TAdaptSpeed::clearAllStatistics()
{
    for(int ii=0;ii<32;ii++){
        clearStatistics(ii);
        speed_stat[ii].rsgood.clear();
    }
}

Speed_Stat* TAdaptSpeed::TAdaptSpeed::getStatResult()
{
    return speed_stat;
}

void TAdaptSpeed::addStat(int err,int max,int ind, int ch)
{
    int no_dec=0;
    int err1,max1;
    //areRejectives=false;
    speed_stat[ind].freqNum = ch;
    speed_stat[ind].rsMax = max;

    int statLen=speed_stat[ind].rslist.count();
    if (statLen >= speedWindow){
        speed_stat[ind].rslist.removeFirst();
        speed_stat[ind].mxlist.removeFirst();
    }
    speed_stat[ind].rslist.append(err);
    speed_stat[ind].mxlist.append(max);

    int statLen1=speed_stat[ind].rsgood.count();
    if (statLen1 == 3){
        speed_stat[ind].rsgood.removeFirst();
    }
    speed_stat[ind].rsgood.append(err);

    speed_stat[ind].summa_rs = 0;
    speed_stat[ind].summa_mx = 0;
    for (int ii=0;ii<speed_stat[ind].rslist.count();ii++){
        err1 = speed_stat[ind].rslist.at(ii);
        max1 = speed_stat[ind].mxlist.at(ii);
        speed_stat[ind].summa_rs += err1;
        speed_stat[ind].summa_mx += max1;
        if(err1 > speed_stat[ind].mxlist.at(ii) - 1 ){
            no_dec++;
        }
    }
    speed_stat[ind].not_decoded = no_dec;

    calc_adapt_action(ind);

}

void TAdaptSpeed::calc_adapt_action(int ind)
{
    if (speed_stat[ind].summa_mx > 0){
        float indicator = static_cast<float>(speed_stat[ind].summa_rs) / speed_stat[ind].summa_mx;
        if (speed_stat[ind].rslist.count() >= speedWindow){
            if (indicator <= thresh_H/*0.1*/) {
                if (curVariantIndex[ind] < groupIndexHigh) speed_stat[ind].speed_advise = SPEED_UP;
                else speed_stat[ind].speed_advise = THE_SAME_SPEED;
            }
            if ((indicator > thresh_H)&&(indicator <= thresh_G)) speed_stat[ind].speed_advise = THE_SAME_SPEED;
            if ((indicator > thresh_G/*0.5*/)&&(indicator <= thresh_F/*0.9*/)){
                if (curVariantIndex[ind] > groupIndexLow) speed_stat[ind].speed_advise = SPEED_DOWN;
                else speed_stat[ind].speed_advise = THE_SAME_SPEED;
            }
            if (indicator > thresh_F){
                 speed_stat[ind].speed_advise = SPEED_DROP;
                if (variants_v[curVariantIndex[ind]].freq_mode == tr("ППРЧ")){
                    speed_stat[ind].reject = true;
                }
            }

            logStat(ind);
        }
    }
}

int TAdaptSpeed::getNewSpeedNumber(int fnum, int *speed_reset)
{
    int vnum=curVariantIndex[fnum];
    if (speed_stat[fnum].speed_advise == SPEED_UP){
        if (curVariantIndex[fnum] < groupIndexHigh) {
            clearStatistics(fnum);
            curVariantIndex[fnum]++;
            vnum = curVariantIndex[fnum];
        }
    }
    if (speed_stat[fnum].speed_advise == SPEED_DOWN){
        if (curVariantIndex[fnum] > groupIndexLow){
            clearStatistics(fnum);
            curVariantIndex[fnum]--;
            vnum = curVariantIndex[fnum];
        }
    }
    if (vnum != groupIndexLow) *speed_reset = 3;
    if (speed_stat[fnum].speed_advise == SPEED_DROP){
            clearStatistics(fnum);
            if (curVariantIndex[fnum] != groupIndexLow){
                curVariantIndex[fnum] = groupIndexLow;
                *speed_reset = 1;
            }else{
                *speed_reset = 2;
            }
            vnum = curVariantIndex[fnum];
    }

    return vnum;
}

int TAdaptSpeed::getNewSpeedNumberFix(int fnum, int *speed_reset)
{
    int vnum=curVariantIndex[fnum];
    if (speed_stat[fnum].speed_advise == SPEED_UP){
        if (curVariantIndex[fnum] < groupIndexHigh) {
            clearStatistics(fnum);
            curVariantIndex[fnum]++;
            vnum = curVariantIndex[fnum];
        }
    }
    if (speed_stat[fnum].speed_advise == SPEED_DOWN){
        if (curVariantIndex[fnum] > groupIndexLow){
            clearStatistics(fnum);
            curVariantIndex[fnum]--;
            vnum = curVariantIndex[fnum];
        }
    }
    if (vnum != groupIndexLow) *speed_reset = 3;
    if (speed_stat[fnum].speed_advise == SPEED_DROP){
            clearStatistics(fnum);
            if (curVariantIndex[fnum] != groupIndexLow){
                curVariantIndex[fnum] = groupIndexLow;
                *speed_reset = 1;
            }else{
                *speed_reset = 2;
            }
            vnum = curVariantIndex[fnum];
    }

    return vnum;
}

void TAdaptSpeed::renewSpeedNumberArray(int freqNum, int variant)
{
//    curVariantIndex[freqNum] = variant;
//    clearStatistics(freqNum);
}

int TAdaptSpeed::getBaseIndex()
{
    return groupIndexLow;
}

void TAdaptSpeed::setCurIndex(int find, int var)
{
    curVariantIndex[find] = var;
}

bool TAdaptSpeed::getValidCondition(int var)
{
    bool res = false;
    if ((var >= groupIndexLow)&&(var <= groupIndexHigh)) res = true;
    return res;
}

bool TAdaptSpeed::getLowSpeedCondition(int fIndex)
{
    bool res = false;
    if (curVariantIndex[fIndex] == groupIndexLow) res = true;
    return res;
}

void TAdaptSpeed::logConnectParams(int ind,float q)
{
    if (adapts == nullptr) return;
    //strToFile(adapts,0,tr(" ").toUtf8().data(),1);
    intToFile(adapts, 0, tr("     baseVariantNumber= ").toUtf8().data(),variants_v[groupIndexLow].num, 0, 10,' ',1);
    intToFile(adapts, 0, tr("     variantGroup= ").toUtf8().data(),variantGroup, 0, 10,' ',1);
    intToFile(adapts, 0, tr("     curVariantNumber= ").toUtf8().data(),variants_v[curVariantIndex[ind]].num, 0, 10,' ',1);
    floatToFile(adapts, 0, tr("     decoded= ").toUtf8().data(), q, 0, 2,'f',1);
    intToFile(adapts, 0, tr("     next var= ").toUtf8().data(),ind+1, 0, 10,' ',1);
    strToFile(adapts,0,tr(" ").toUtf8().data(),1);
}

void TAdaptSpeed::logStat(int index)
{
    if (speed_stat[index].speed_advise > 0)
    {
        int good=0;
        if (speed_stat[index].rslist.count() >= goodWindow){
            for(int ii=0;ii<goodWindow;ii++){
                good += speed_stat[index].rsgood.at(ii);
            }
        }
        float indicator = static_cast<float>(speed_stat[index].summa_rs) / (speedWindow*speed_stat[index].rsMax);

        strToFile(adapts,1,tr(" ").toUtf8().data(),0);
        intToFile(adapts, 0, tr(" ii=").toUtf8().data(),index, 3, 10,' ',0);
        intToFile(adapts, 0, tr(" F=").toUtf8().data(),speed_stat[index].freqNum, 3, 10,' ',0);
        intToFile(adapts, 0, tr(" count=").toUtf8().data(),speed_stat[index].rslist.count(), 3, 10,' ',0);
        intToFile(adapts, 0, tr(" errRS=").toUtf8().data(),speed_stat[index].summa_rs, 3, 10,' ',0);
        intToFile(adapts, 0, tr(" errMax=").toUtf8().data(),speed_stat[index].summa_mx, 3, 10,' ',0);
        intToFile(adapts, 0, tr(" no_dec=").toUtf8().data(),speed_stat[index].not_decoded, 3, 10,' ',0);

        intToFile(adapts, 0, tr(" variant=").toUtf8().data(),curVariantIndex[index], 3, 10,' ',0);
        floatToFile(adapts, 0, tr(" reper1=").toUtf8().data(),thresh_H, 0, 2,'f',0);
        floatToFile(adapts, 0, tr(" reper2=").toUtf8().data(),thresh_G, 0, 2,'f',0);
        floatToFile(adapts, 0, tr(" reper3=").toUtf8().data(),thresh_F, 0, 2,'f',0);
        floatToFile(adapts, 0, tr(" decision_value=").toUtf8().data(),indicator, 0, 2,'f',0);
        intToFile(adapts, 0, tr(" advise=").toUtf8().data(),speed_stat[index].speed_advise, 3, 10,' ',0);
        intToFile(adapts, 0, tr(" good=").toUtf8().data(),good, 3, 10,' ',1);
    }
}

int TAdaptSpeed::getStatWindow()
{
    return speedWindow;
}

bool TAdaptSpeed::getStatValid(int index, bool *ok)
{
    bool res = false;
    if (speed_stat[index].rslist.count() >= speedWindow){
        if (speed_stat[index].rsgood.count() >= goodWindow)
            res = true;
    }
    int res0=0;
    for (int ii = 0; ii < speed_stat[index].rsgood.count(); ii++) {
        res0 = res0 + speed_stat[index].rsgood.at(ii);
    }
    if (res0 == 0) *ok = true;
    else *ok = false;
    return res;
}
