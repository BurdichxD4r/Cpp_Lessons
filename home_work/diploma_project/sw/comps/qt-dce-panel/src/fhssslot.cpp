#include <QDebug>
#include "fhssslot.h"
#include "math.h"

//#define		DO_LOG
#ifdef DO_LOG
#include <stdio.h>
#endif

FhssSlot::FhssSlot(QObject *parent) : QObject(parent)
{
   referenceCodes = new  QList<int>();
   modems = new FhssModem(this);
   rscodec = new TReedSol(this);
   slotSpectrum = new  QList <fft_complex>();
   bestSpectrum = new  QList <fft_complex>();
   floatDoubleSlot = new QVector<fft_complex>();
   decodedWord = new  QList<int>();
   bestDecoded = new  QList<int>();
   codedWrd = new  QList<int>();   // закодированное сообщение для слота
   channelCds = new  QList<int>();  // сообщение слота в канальных кодах
}

FhssSlot::~FhssSlot()
{
    deleteLogger(slog);
}

int FhssSlot::getRefParcel()
{
    int ref;
    if (varslot->f_change_time == 3) {
        ref = 1;
    }else if (varslot->f_change_time == 75) {
        ref = 7;
    }else ref = -1;
    return ref;
}

void FhssSlot::setSlotVariant(SLOT_VARIANT *vars)
{
    //intToFile(slog, 1, tr(" setSlotVariant vars.num=  ").toUtf8().data(),vars->num, 0, 10,' ',1);
    varslot = vars;
    if (rscodec->RSCodec_init( 1<<varslot->symbol_bits,  varslot->N,varslot->K) != 0)
    {
        rsValid = false;
        qDebug() << "rscodec is not valid";
    }
    rsValid = rsValid && (RS_CODE > 0);
    slotParcelsNumber = varslot->slot_time/one_parcel_time;
    referenceParcel = getRefParcel();
    referenceCodesCreate();

    modems->setConfiguration(vars);
    modems->set_reference_parcel();
    frequencyShift = 0;
    slotLength = (vars->freq_discr/100)*(vars->slot_time/10);
    freqCnt = 0;
    MinError = (vars->N - vars->K)/2 + 1;
    maxErrorsRS = (vars->N - vars->K)/2 ;
    DecShift = vars->freq_discr/1600;
    bestFrameShifts = slotLength/DecShift;
//    qDebug() << "DecShift, bestFrameShifts="<< DecShift<<bestFrameShifts;
//    qDebug() << "slots_params="<< vars->num<<vars->freq_mode<< vars->f_change_time<<vars->slot_time<<vars->freq_band<<
//                    vars->mode_type<<vars->rate<<slotParcelsNumber<<vars->pack_number<<vars->subchan_number<<vars->N << vars->K<<
//                    vars->stage<<referenceParcel;

}

int FhssSlot::getSlotMessageLength()
{
    if (rsValid){
        return varslot->K;
    }else{
        return varslot->N; // передача без кодирования
    }
}

void FhssSlot::setSlotMessage(QList<int>* mess)
{
    QList<int>* codedWord = codedWrd;
    QList<int>* channelCodes = channelCds;
    if (rsValid){
        rscodec->Encoder(mess,codedWord);
        scrambler_symbols(codedWord);
        Rewrite(codedWord, 8, channelCodes, varslot->rate);
    }else{
        Rewrite(mess, 8, channelCodes, varslot->rate);
    }

    modems->clearSignal();
    int jj = 0;
    for(int ii = 1;ii <= slotParcelsNumber;ii++){
        if ((ii <  referenceParcel)||(jj >= varslot->pack_number)){
            modems->modulate(channelCodes, channelCodes->count()+1);
        }else if (ii == referenceParcel){
            modems->set_reference_parcel();
            referenceCodesCreate();
            modems->modulate(referenceCodes, 0);
        }else{
            modems->modulate(channelCodes, jj*varslot->subchan_number);
            jj++;
        }
    }
    modems->createSlotSignal();
    if ( ZEROING ) modems->freqAdjustInterval();
}

QList<int>* FhssSlot::getSlotMessage()
{ 
    bestDecoded->clear();
    phase_variance	= -1.0;
    phase_shift	= 0.0;
    SNR_from_phase_variance=0;

    compensate_for_the_doppler();
    calculateBestFrame();

    return bestDecoded;
}

void FhssSlot::Rewrite(QList<int>* Input, int Input_Size, QList<int>* Output, int Output_Size)
{
    int		i, k, n;
    k = 0;
    n = 0;
    int		mask = (1 << Output_Size) - 1;
    int Input_Len = Input->count();
    Output->clear();

    for(i=0; i < Input_Len;)
    {
        if( Output_Size > Input_Size + n  )
        {
            k |= Input->at(i) << (Output_Size - Input_Size - n);
            n += Input_Size;
            i++;
        }
        else
        {
            if( Output_Size == Input_Size + n )
            {
                k |= Input->at(i);
                n = 0;
                Output->append(k & mask);
                i++;
            }
            else
            {
                k |= Input->at(i) >> (Input_Size + n - Output_Size);
                n -= Output_Size;
                Output->append(k & mask);
            }
            k = 0;
        }
    }
    Output->append(k & mask);
    //Output->append('\0');

}

short_complex* FhssSlot::getOutput()
{
    return modems->getOutput();
}

SLOT_VARIANT* FhssSlot::getVarslot()
{
    return varslot;
}

void FhssSlot::referenceCodesCreate()
{
    int index=0;
    int count=0;
    referenceCodes->clear();
    int M = 1 << varslot->rate;
    while(referenceCodes->count() < varslot->subchan_number){
        if (varslot->rate == 4){
            count++;
            if (count < 4){  // для АФМ-4 пропускаем каждую 4-ую точку(это точки внутреннего круга с номерами 3,7,11,15)
                if (randReferenceParcel) index = rand()%M;
                if (!((index==3)||(index==7)||(index==11)||(index==15)))
                    referenceCodes->append(index);
            }else{
                count = 0;
            }
        }else{
            if (randReferenceParcel) index = rand()%M;
            referenceCodes->append(index);
        }
    }
}

 bool FhssSlot::getShiftingState()
{
    return modems->shift_begin_point();
}

 QList <fft_complex>* FhssSlot::getSlotSpectrum()
 {
    return bestSpectrum;
 }

 void FhssSlot::GetShift(QList <fft_complex>* sozv )
 {
     float	r, s;
     float	m0, mu, ml;
     float	d0, du, dl;
     m0	= mu	= ml	= d0	= 0.0;
     float swr =  SectWidthRec[varslot->rate - 1];

     QList <float> phase;
     for (int ii=0;ii<sozv->count();ii++) {
         if (varslot->rate != 4){
             phase.append(atan2f(sozv->at(ii).imag(),sozv->at(ii).real()));
         }else{
             if (abs(sozv->at(ii)) > 0.7){
                 phase.append(atan2f(sozv->at(ii).imag(),sozv->at(ii).real()));
             }
         }
     }
     if (phase.isEmpty()) return;
     s = pi;
     //s = (float) (pi + 0.5* swr);

     for(int ii=0; ii < phase.count(); ii++)
     {
         r = getPhaseAdd(phase.at(ii) + s,swr);
         m0	+= r;
         d0	+= r*r;
         mu	+=  getPhaseAdd(phase.at(ii)+ Delta + s,swr);
         ml	+=  getPhaseAdd(phase.at(ii) - Delta + s,swr);
     }

     r = 1.0f/phase.count();

     m0	*= r;
     mu	*= r;
     ml	*= r;
     d0	*= r;

     r	= (float) (SNR_a * log(d0 - m0*m0) + SNR_b);

     if(	SNR_from_phase_variance < r )
         SNR_from_phase_variance = (int) r;

//     setFlags(slog, FORCESIGN | FORCEPOINT);
//     floatToFile(slog,1,tr("   SNR,dB =  ").toUtf8().data(),SNR_from_phase_variance,0,2,'f',0);
//     floatToFile(slog,0,tr("   r,dB =  ").toUtf8().data(),r,0,2,'f',1);

     d0	=   du	=  dl	= 0.0;

     for(int ii=0; ii < phase.count(); ii++)
     {
         r	= getPhaseAdd(phase.at(ii) - m0 + s,swr);
         d0	+= r*r;

         r	= getPhaseAdd(phase.at(ii) - mu + s,swr);
         du	+= r*r;

         r	= getPhaseAdd(phase.at(ii) - ml + s,swr);
         dl	+= r*r;
     }

     if( d0 > du )
     {
         if( du > dl )
         {
             phase_shift	= ml + Delta;
             phase_variance	= dl;
         }
         else
         {
             phase_shift	= mu - Delta;
             phase_variance	= du;
         }
     }
     else
     {
         if( d0 > dl )
         {
             phase_shift	= ml + Delta;
             phase_variance	= dl;
         }
         else
         {
             phase_shift	= m0;
             phase_variance	= d0;
         }
     }
// qDebug() << "GetShift " <<SNR_from_phase_variance << phase_shift*180/pi<<phase_shift*radiansToGerz
//                     <<"d0,du,dl="<<d0<<du<<dl <<"fs="<<frequencyShift;
 }

 float FhssSlot::getDoppler()
 {
     return phase_shift*radiansToGerz;
 }

 float FhssSlot::getSNR()
 {
     return SNR_from_phase_variance;
 }

 int FhssSlot::getRSerr()
 {
     int err = MinError;
     if (err > maxErrorsRS) err = maxErrorsRS;
     if (err < 0) err = maxErrorsRS;
     return err;
 }

 int FhssSlot::getmaxRSerr()
 {
     return maxErrorsRS;
 }

 float FhssSlot::getPhaseAdd(float phase,float sector)
 {
     float res=0;
     //res =  fmodf(phase,sector)- 0.5*sector;
     res =  fmodf(phase,sector);
     if (res > 0.5*sector) res = res-sector;
     return res;
 }

 void FhssSlot::setCompensationMode(bool state)
 {
     modems->setCompensationMode(state);
     frequencyShift = 0;
     frequencyCompensation = state;
     freqCnt=0.0;
 }

 void FhssSlot::setDoubleSlotInput(QVector<int_complex>*signal)
 {
      doubleSlot = signal;
      //qDebug() <<"doubleSlot[]="<< doubleSlot->at(0).real() << doubleSlot->at(0).imag();
 }

 void FhssSlot::compensate_for_the_doppler()
 {
     fft_complex cs,sig;
     floatDoubleSlot->clear();
     freqCnt -= frequencyShift*slotLength;
     if (freqCnt > 2*pi) freqCnt -= 2*pi;
     for(int ii=0;ii < doubleSlot->count();ii++){
         sig = fft_complex(static_cast<float>(doubleSlot->at(ii).real()),static_cast<float>(doubleSlot->at(ii).imag()));
         if (frequencyCompensation){
             cs = fft_complex(cosf(freqCnt),-sinf(freqCnt));
             floatDoubleSlot->append(cs * sig);
             freqCnt += frequencyShift;
             if (freqCnt > 2*pi) freqCnt -= 2*pi;
         }else{
             floatDoubleSlot->append(sig);
         }
     }
 }

 int FhssSlot::getBestFrame()
 {
     return BestFrame;
 }

 int FhssSlot::getBestFrameShifts()
 {
     return bestFrameShifts;
 }

 void FhssSlot::calculateBestFrame()
 {
     bool decoded=false;
     slotSpectrum->clear();
     float disp=0;
     if (!rsValid) {BestFrame =bestFrameShifts/2; return;}
     MinError = (varslot->N - varslot->K)/2 + 1;
     int MinErr = MinError;
     int errNum=MinErr;
     for(int kk=0;kk<bestFrameShifts;kk++){
         errNum = frameDecoding(kk);
         if ((errNum>=0)&&(errNum<MinErr)) seachBestVariance(errNum, kk, &decoded,&disp);
     }
     if(decoded){
         if (disp < 0){
             GetShift(bestSpectrum);
         }
         frequencyShift += 80*phase_shift/(varslot->freq_discr);
     } else {
         errNum=frameDecoding(BestFrame);
         getBestCode();
     }
     if (varslot->num <=4){
         QVector<fft_complex> sign;
         sign = floatDoubleSlot->mid(BestFrame * DecShift, slotLength);
         modems->setModemVariant(&sign);
         modems->memoryParcelsFft(slotParcelsNumber);
     }
 }

 void FhssSlot::getBestCode()
 {
     bestSpectrum->clear();
     bestSpectrum->append(*slotSpectrum);
     bestDecoded->clear();
     bestDecoded->append(*decodedWord);
 }

 void FhssSlot::seachBestVariance(int errNum,int kk,bool *success,float *disp)
 {
     if (errNum < MinError){
         MinError = errNum;
         BestFrame=kk;
         *disp=-1.0;
         getBestCode();
         *success = true;
     }else{
         if (errNum == MinError){
             if (*disp < 0){
                 GetShift(bestSpectrum);
                 *disp = phase_variance;
             }
             GetShift(slotSpectrum);
             if (phase_variance < *disp){
                 //qDebug() << "rs_err2="<<errNum<<"disp1="<<phase_variance<<"disp2"<<disp<<"bestFrame="<<kk;
                 *disp = phase_variance;
                 BestFrame=kk;
                 getBestCode();
                 *success = true;
             }
         }
     }
 }

 int   FhssSlot::frameDecoding(int frameNumber)
 {
     int res = MinError;
     QList<int>* channelCodes = channelCds; // принятое сообщение слота в канальных кодах
     channelCodes->clear();
     slotSpectrum->clear();
     QList<int>* channelWord = codedWrd;  // сообщение слота в символах
     channelWord->clear();
     QVector<fft_complex> sign;
     sign = floatDoubleSlot->mid(frameNumber * DecShift, slotLength);
     if (sign.count() >= slotLength){
         modems->setModemVariant(&sign);
         int jj =0;
         for(int ii = 1;ii <= slotParcelsNumber;ii++){
             QList<int>* codesd = modems->demodulate(ii);
             if ((ii >= referenceParcel + 1)&&(jj < varslot->pack_number)) {
                 channelCodes->append(*codesd);
                 QList <fft_complex>*sp = modems->getDemodSpectr();
                 slotSpectrum->append(*(sp));
                 jj++;
             }
         }
         Rewrite(channelCodes, varslot->rate, channelWord, 8);
         scrambler_symbols(channelWord);
         res = rscodec->Decoder(channelWord,decodedWord);
     }else res =-1;
     return res;
 }

 int FhssSlot::getSlotLength()
 {
      return slotLength;
 }

 void FhssSlot::scrambler_symbols(QList<int>* cword)
 {
     for(int ii=0; ii < cword->length(); ii++)
     {
         int x = cword->at(ii) ^ ScramblerBuf[ii];
         cword->replace(ii,	x);
     }
 }

 void FhssSlot::setModemNumber(int num)
 {
     modems->setModemNumber(num);
     QString fname = QString("slot_log%1.txt").arg(num);
     #ifdef	DO_LOG
         slog = newLogger(fname.toUtf8().data(),1,1);
     #else
         slog = newLogger(fname.toUtf8().data(),1,0);
     #endif
      intToFile(slog,1,tr("class FhssSlot ").toUtf8().data(),num, 0, 10,' ',1);
 }

 void FhssSlot::setRR2(bool rnd)
 {
     randReferenceParcel = rnd;
     qDebug() << "FhssSlot::setRR2 rnd=" <<randReferenceParcel;
 }




