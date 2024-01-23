#include <QDebug>
#include "math.h"
#include "fhssmodem.h"
#include "eml.h"

using namespace std;

//#define		DO_LOG
#ifdef DO_LOG
#include <stdio.h>
#endif

FhssModem::FhssModem(QObject *parent, int num) : QObject(parent)
{
    Number = num;
    signal = new QList<fft_complex>();
    inputSignal = new QVector<fft_complex>(3840,fft_complex(0,0));
    parcelSpectrum = new QList <fft_complex>();
    channelCds = new  QList<int>();

    infft_op = &infftm1[0];
    infft_cur = &infftm2[0];
    outfft_op = &outfft1[0];
    outfft_cur = &outfft2[0];
    for (int jj=0;jj<4;jj++){
        ifft1[jj] = new FastFourier(this,fftdim[jj], &infftm1[0], &outfft[0], FFTW_BACKWARD);
        ifft2[jj] = new FastFourier(this,fftdim[jj], &infftm2[0], &outfft[0], FFTW_BACKWARD);
        fft1[jj] =  new FastFourier(this,fftdim[jj], &infftd[0], &outfft1[0], FFTW_FORWARD);
        fft2[jj] =  new FastFourier(this,fftdim[jj], &infftd[0], &outfft2[0], FFTW_FORWARD);
    }
    fftmem =  new FastFourier(this,fftdim[0], &infftd[0], &outfft_mem[0], FFTW_FORWARD);
    for (int ii=0;ii<17;ii++) {
        constellation[ii] = 0;
    }
}

FhssModem::~FhssModem()
{
    deleteLogger(mlog);
    delete signal;
    delete parcelSpectrum;
    delete channelCds;
}

void FhssModem::setConfiguration(SLOT_VARIANT *vars)
{
    slot = vars;
    fft_dim = slot->freq_discr/100;
    if (slot->f_change_time == 3){
        freq_adjust_points = static_cast<int>(roundf((float)fft_dim*zero_fraction));
    }else freq_adjust_points = 0;

    guard_interval = fft_dim/4;
    //qDebug() << "freq_adjust_points = "<<slot->num <<  freq_adjust_points<<guard_interval ;
    sozvez_length = 1 << slot->rate;
    slotParcelsNumber = slot->slot_time/one_parcel_time;
    bandNumber = 1;
    first_chan = fft_dim -1 - slot->subchan_number/2;
                              // смещение середины спектра в ноль частоты
                              // переход к комплексной огибающей сигнала
    if (slot->freq_band==fBand_4500) {
        bandNumber = 2;
    }
    if (slot->freq_band==fBand_9000){
        bandNumber = 3;
    }

    for (int ii=0;ii<fft_dim;ii++) {
        infftm1[ii] = 0;
        infftm2[ii] = 0;
        infftd[ii] = 0;
        outfft1[ii] = 0;
        outfft2[ii] = 0;
    }

    constellation_diagram_set();
    testIndex = 0;
    freqCnt = 0;

    intToFile(mlog,1,tr(" SKK Number= ").toUtf8().data(),slot->num, 0, 10,' ',0);
    intToFile(mlog,0,tr(" fft_dim= ").toUtf8().data(),fft_dim, 0, 10,' ',0);
    intToFile(mlog,0,tr(" f1= ").toUtf8().data(),first_chan-(fft_dim-1), 0, 10,' ',0);
    intToFile(mlog,0,tr(" f2= ").toUtf8().data(),(first_chan+slot->subchan_number)%fft_dim, 0, 10,' ',1);
    //normrand_test();
    //awgn_test();

    //    qDebug() << "slot_params"<<Number << slot->num<<slot->freq_mode<< slot->f_change_time<<slot->slot_time<<slot->freq_band<<
    //             slot->mode_type<<slot->rate<<slot->pack_number<<slot->subchan_number<<slot->N << slot->K<<slot->stage;
}

void FhssModem::constellation_diagram_set()
{
    for (int ii=0;ii<sozvez_length;ii++) {
        switch( slot->rate )
        {
        case 1:
           constellation[ii] = ofm1[ii];
           break;
        case 2:
            constellation[ii] = ofm2[ii];
            break;
        case 3:
            constellation[ii] = fft_complex(cos3[ii],sin3[ii]);
            break;
        case 4:
            constellation[ii] = fft_complex(cos4[ii],sin4[ii]);
            break;
        }
    }
}

void FhssModem::setModemNumber(int num)
{
    Number = num;
    QString fname = QString("m_log%1.txt").arg(Number);
    #ifdef	DO_LOG
        mlog = newLogger(fname.toUtf8().data(),1,1);
    #else
        mlog = newLogger(fname.toUtf8().data(),1,0);
    #endif
     intToFile(mlog,1,tr("class FhssModem ").toUtf8().data(),Number, 0, 10,' ',1);
}

int FhssModem::getModemNumber()
{
    return Number;
}

void FhssModem::modulate(QList<int>*codes, int pos)
{
    emit symbolsSended("send:",sendCodes,slot->subchan_number);

    one_parcel_modulation_s(codes,pos);
    one_parcel_ifft();

    //qDebug() << "form a parcel by modem number "<< Number <<sozvez_length<<fft_dim;

    if (infft_op == infftm1){
        infft_op = infftm2;
        infft_cur = infftm1;
    }
    else if (infft_op == infftm2){
        infft_op = infftm1;
        infft_cur = infftm2;
    }
}

void FhssModem::create_infftd(int parcel)
{
    int numBegin = (parcel-1)*(fft_dim + guard_interval)+begin_point_shift;
//    floatToFile(mlog,1,tr("  parcel= ").toUtf8().data(),parcel,0,2,'f',0);
//    intToFile(mlog,0,tr("  numBegin= ").toUtf8().data(),numBegin,0,10,' ',0);
//    intToFile(mlog,0,tr("  numBegin+fft_dim= ").toUtf8().data(),numBegin+fft_dim,0,10,' ',0);
//    intToFile(mlog,0,tr("  inputSignal->count()= ").toUtf8().data(),inputSignal->count(),0,10,' ',1);
    if (numBegin+fft_dim < inputSignal->count()){
        for(int ii=0;ii < fft_dim;ii++){
            float x = inputSignal->at(numBegin+ii).real()/scaleFactor[bandNumber-1];
            float y = inputSignal->at(numBegin+ii).imag()/scaleFactor[bandNumber-1];
            infftd[ii] = fft_complex(x,y);
        }
    }else
    {
        memset(infftd, 0 , fft_dim*sizeof (fft_complex));
    }
}

QList<int>*  FhssModem::demodulate(int parcel)
{
    create_infftd(parcel);
    one_parcel_fft();

    restoreReference(parcel);

    parcelSpectrum->clear();
    get_channel_symbols();
    emit symbolsSended("recv:",recvCodes,slot->subchan_number);

    QList<int>* channelCodesd = channelCds;
    channelCodesd->clear();
    for (int ii=0;ii<slot->subchan_number;ii++) {
        channelCodesd->append(recvCodes[ii]);
    }

    if (outfft_op == outfft1){
        outfft_op = outfft2;
        outfft_cur = outfft1;
    }
    else if (outfft_op == outfft2){
        outfft_op = outfft1;
        outfft_cur = outfft2;
    }
    return channelCodesd;
}

void FhssModem::get_channel_symbols()
{
    switch( slot->rate )
    {
    case	1:
        demod_ofm1();
        break;
    case	2:
        demod_ofm2();
        break;
    case	3:
        demod_ofm3();
        break;
    case	4:
        demod_afm4();
        break;
    }
}

fft_complex FhssModem::get_spectrum_point(int ii)
{
    fft_complex sp_op = (*(outfft_op+ii));
    float module_op = abs(sp_op);
    fft_complex sp_cur = *(outfft_cur+ii);
    fft_complex sp = conj(sp_op)*sp_cur;
    if (module_op) sp = sp/module_op;
    return sp;
}

void FhssModem::demod_ofm1()
{
    float spr;
    int jj=0;
    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++)
    {
        int kk = ii % fft_dim;
        fft_complex sp = get_spectrum_point(kk);
        parcelSpectrum->append(sp);

        spr = real(sp);
        if (spr > 0.0) recvCodes[jj] = 0;
        else recvCodes[jj] = 1;
        jj++;
    }
    //qDebug() << "demod_ofm1 ";
}

void FhssModem::demod_ofm2()
{
    float spr,spi;
    int jj=0;
    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++)
    {
        int kk = ii % fft_dim;
        fft_complex sp = get_spectrum_point(kk);
        parcelSpectrum->append(sp);

        spr = real(sp);
        spi = imag(sp);
        if (spr*spr > spi*spi){
            if( spr > 0.0 )
                recvCodes[jj] = 0;
            else
                recvCodes[jj] = 3;
        }
        else
        {
            if( spi > 0.0 )
                recvCodes[jj] = 1;
            else
                recvCodes[jj] = 2;
        }
        jj++;
    }
    //qDebug() << "demod_ofm2 ";
}

void FhssModem::demod_ofm3()
{
    float spr,spi, spr2, spi2, v, sp2;;
    int jj=0;
    v = (float) sin(pi/8.0);
    v *= v;

    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++)
    {
        int kk = ii % fft_dim;
        fft_complex sp = get_spectrum_point(kk);
        parcelSpectrum->append(sp);

        spr = real(sp);
        spi = imag(sp);

        spr2 = spr*spr;
        spi2 = spi*spi;
        sp2 = spr2 + spi2;

        if( spr2 - sp2*v < 0.0 )
        {
            if( spi > 0.0 )
                recvCodes[jj] = 2;
            else
                recvCodes[jj] = 4;
        }
        else
        {
            if( spi2 - sp2*v < 0.0 )
            {
                if( spr > 0.0 )
                    recvCodes[jj] = 1;
                else
                    recvCodes[jj] = 7;
            }
            else
            {
                if( spr > 0.0 )
                {
                    if( spi > 0.0 )
                        recvCodes[jj] = 0;
                    else
                        recvCodes[jj] = 5;
                }
                else
                {
                    if( spi > 0.0 )
                        recvCodes[jj] = 3;
                    else
                        recvCodes[jj]= 6;
                }
            }
        }
        jj++;
    }
    //qDebug() << "demod_ofm3 ";
}

void FhssModem::demod_afm4()
{
    float spr,spi,module_s,angle_s;
    int jj=0;
    maxMod = 0;
    float val;
    fft_complex sp;
    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++)
    {
        int kk = ii % fft_dim;
        sp = get_spectrum_point(kk);
        parcelSpectrum->append(sp);
        val = abs(sp);
        if (val > maxMod) maxMod = val;
    }
    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++)
    {
        int kk = ii % fft_dim;
        parcelSpectrum->replace(ii-first_chan,parcelSpectrum->at(ii-first_chan) / maxMod);
        sp =  parcelSpectrum->at(ii-first_chan);

        spr = real(sp);
        spi = imag(sp);
        module_s = abs(*(outfft_cur+kk));
        angle_s = atan2(spi, spr)*180/pi;
        if (module_s < 0.7*maxMod){
            if ((angle_s >= -180) && (angle_s < -90))   recvCodes[jj]= 15;
            else if ((angle_s >= -90) && (angle_s < 0)) recvCodes[jj]= 11;
            else if ((angle_s >= 0) && (angle_s < 90))  recvCodes[jj]= 3;
            else  recvCodes[jj]= 7;
        } else{
            if       ((angle_s >= -165) && (angle_s < -135))  recvCodes[jj]= 12;
            else if ((angle_s >= -135) && (angle_s < -105)) recvCodes[jj]= 13;
            else if ((angle_s >= -105) && (angle_s < -75)) recvCodes[jj]= 9;
            else if ((angle_s >= -75)   && (angle_s < -45))  recvCodes[jj]= 8;
            else if ((angle_s >= -45)   && (angle_s < -15)) recvCodes[jj]= 10;
            else if ((angle_s >= -15)   && (angle_s <  15))  recvCodes[jj]= 2;
            else if ((angle_s >=  15)   && (angle_s <  45)) recvCodes[jj]= 0;
            else if ((angle_s >=  45)   && (angle_s <  75)) recvCodes[jj]= 1;
            else if ((angle_s >=  75)   && (angle_s <  105)) recvCodes[jj]= 5;
            else if ((angle_s >=  105) && (angle_s <  135)) recvCodes[jj]= 4;
            else if ((angle_s >=  135) && (angle_s <  165)) recvCodes[jj]= 6;
            else recvCodes[jj]= 14;
        }
        jj++;
    }
    //qDebug() << "demod_afm4 ";
}

void FhssModem::one_parcel_modulation_s(QList<int>* codes,int pos)
{
    quint8 code;
    int jj;
    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++) {
        if (pos < codes->count()) code = (quint8)(codes->at(pos));
        else code=16;     // нулевой сигнал для посылок 1-6 в режимах со сменой частоты 75 мс
        jj = ii % fft_dim;
        fft_complex opor = *(infft_op+jj);
        *(infft_cur+jj) = opor*constellation[code];
        if (slot->rate == 4){
            float module_op = abs(*(infft_op+jj));
            if ((module_op < 0.7)&&(module_op > 0.1)){
                *(infft_cur+jj) =  *(infft_cur+jj) / module_op;
            }
        }
        pos++;
    }
}

void FhssModem::set_reference_parcel()
{
    int jj;
    for (int ii=0;ii<fft_dim;ii++) {
        *(infft_op+ii) = fft_complex(0.0,0.0);
    }
    for (int ii=first_chan;ii<first_chan + slot->subchan_number;ii++) {
        jj = ii % fft_dim;
        *(infft_op+jj) = fft_complex(1.0,0.0);
    }
}

void FhssModem::off_reference_parcel()
{
    for (int ii=0;ii<fft_dim;ii++) {
           *(infft_op+ii) = fft_complex(0.0,0.0);
    }
}

void FhssModem::one_parcel_ifft()
{
    int index = 0;
    if (fft_dim == fftdim[1]) index = 1;
    else if (fft_dim == fftdim[2]) index = 2;
    else if (fft_dim == fftdim[3]) index = 3;
    if (infft_cur == infftm1){
        //qDebug() << "ifft1[index]->fft_execute()";
        ifft1[index]->fft_execute();
    }
    else if (infft_cur == infftm2){
        //qDebug() << "ifft2[index]->fft_execute()";
        ifft2[index]->fft_execute();
    }

    for (int ii=0;ii<fft_dim;ii++) {
        outfft[ii] = outfft[ii] * (float)fft_dim;
        signal->append(outfft[ii]);
    }
//    fft_complex temp = sigma(outfft, fft_dim);
//    qDebug() << "temp=" << real(temp)  << imag(temp);

}

void FhssModem::one_parcel_fft()
{
    int index = 0;
    if (fft_dim == fftdim[1]) index = 1;
    else if (fft_dim == fftdim[2]) index = 2;
    else if (fft_dim == fftdim[3]) index = 3;
    if (outfft_cur == outfft1){
       // qDebug() << "fft1[index]->fft_execute()";
        fft1[index]->fft_execute();
        for (int ii=0;ii<fft_dim;ii++) {
            outfft1[ii]= outfft1[ii]/(float)fft_dim;
        }
    }
    else if (outfft_cur == outfft2){
       // qDebug() << "fft2[index]->fft_execute()";
        fft2[index]->fft_execute();
        for (int ii=0;ii<fft_dim;ii++) {
            outfft2[ii] = outfft2[ii]/(float)fft_dim;
        }
    }

}

void FhssModem::clearSignal()
{
    signal->clear();
}

void FhssModem::createSlotSignal()
{
    //qDebug() << "opora0= "<< signal->at(0).real() << signal->at(0).imag()<< signal->at(1).real() << signal->at(1).imag();
    int kk=0;
    int nn=0;
    for(int ii = 0;ii < slotParcelsNumber;++ii){
        for(int jj=0;jj < fft_dim; jj++)
        {
            short x1 =goToShort(scaleFactor[bandNumber-1], real(signal->at(nn)));
            short x2 =goToShort(scaleFactor[bandNumber-1], imag(signal->at(nn)));
            outputSignal[kk] = short_complex(x1,x2);
            kk++;
            nn++;
        }
        for(int jj=0;jj < guard_interval; jj++)
        {
            outputSignal[kk] =      outputSignal[kk-fft_dim];
            kk++;
        }
    }
}

qint16 FhssModem::goToShort(int koef, float value)
{
    qint16 res=0;
    qint16 lim = 0x7fff;
    value *= koef;
    if (fabs(value) < lim) res = static_cast<qint16>(value);
    else {
        if (value > 0) res = lim;
        else res = -lim;
    }
    return res;
}

short_complex* FhssModem::getOutput()
{
    return outputSignal;
}

void FhssModem::freqAdjustInterval()
{
    int beg =begin_to_zero;
    for(int ii=beg ;ii< freq_adjust_points+beg;ii++){
        outputSignal[ii] =    0;
    }
}

bool FhssModem::shift_begin_point()
{
    bool res = false;

//    if (begin_to_zero ==0) begin_to_zero = 1;
//    else begin_to_zero = 0;
//    return true;

    if (begin_point_shift==0) begin_point_shift = 1;
    else if (begin_point_shift==1) begin_point_shift = 4;
    else begin_point_shift += 4;
    if (begin_point_shift <= guard_interval){
        res = true;
    }else{
        begin_point_shift = 0;
        res = false;
    }
    qDebug() << "begin_point_shift=" << begin_point_shift;
    return res;
}

QList<fft_complex> *FhssModem::getDemodSpectr()
{
    return parcelSpectrum;
}

void FhssModem::setCompensationMode(bool state)
{
    frequencyCompensation = state;
    freqCnt=0.0;
}

void FhssModem::setModemVariant(QVector<fft_complex> *signal)
{
     inputSignal = signal;
}

void FhssModem::memoryParcelsFft(int parcel)
{ 
    create_infftd(parcel);
    fftmem->fft_execute();
    for (int ii=0;ii<fft_dim;ii++) {
        outfft_mem[ii]= outfft_mem[ii]/(float)fft_dim;
    }
}

void FhssModem::restoreReference(int parcel)
{
    if (parcel==1){
        if (slot->num <=4){
            if (outfft_op == outfft1){
                for(int ii=0;ii < fft_dim;ii++){
                    outfft1[ii] = outfft_mem[ii];
                }
            }
            else if (outfft_op == outfft2){
                for(int ii=0;ii < fft_dim;ii++){
                    outfft2[ii] = outfft_mem[ii];
                }
            }
        }
    }
}
