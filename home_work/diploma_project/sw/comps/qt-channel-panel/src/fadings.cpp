#include "fadings.h"
#include "eml.h"
#include <QVector>
#include <QDebug>
#include <QDir>
#include <QCoreApplication>

//#define		DO_LOG
#ifdef DO_LOG
#include <stdio.h>
#endif

TFadings::TFadings(QObject *parent) : QObject(parent)
{
    for (int ii=0;ii<3;ii++) {
        for (int jj=0;jj<num_chan;jj++) {
            depart[jj][ii] = new QList<fft_complex>();
        }
    }

    for (int ii=0;ii<6;ii++) {
        randArray[ii] = new QVector<float>();
    }

    for(int ii=0;ii<num_chan;ii++){
        cs[ii] = new ChannelParams();
        cs[ii]->Private = nullptr;
    }

    QString fname = QString("fadings_log%1.txt").arg(1);
    #ifdef	DO_LOG
        flog = newLogger(fname.toUtf8().data(),1,1);
    #else
        flog = newLogger(fname.toUtf8().data(),1,0);
    #endif
     strToFile(flog,1,tr("class TFadings").toUtf8().data(),1);

}

TFadings::~TFadings()
{
    deleteLogger(flog);
    for(int ii=0;ii<num_chan;ii++){
        mdm_channel_close(ii);
    }
}

void  TFadings::form_gwave_output(QVector<fft_complex>* doubleSlot,QVector<fft_complex>* chanSignal, int chan,float sh)
{
    ChannelParams* pub = cs[chan];
    //if (chanSignal->count() < slotLength) return;
    depart[chan][1]->clear();
    for (int ii=0;ii<slotLength;ii++) {
        depart[chan][1]->append(pub->gwave*chanSignal->at(ii));
    }
    QVector<fft_complex> departs;
    for (int ii = 0;ii < 3;ii++) {
        departs.append(depart[chan][1]->toVector());
    }
    doubleSlot->clear();
    int start = replaceOffset(0,0);
    int finish = static_cast<int>(fminf(departs.count(), start+2*slotLength));
    for (int ii=start;ii<finish;ii++) {
       doubleSlot->append(departs.at(ii));
    }
    doppler_add(doubleSlot, sh);
}

void TFadings::get_impulse_response(ChannelParams* pub,int variant)
{
    pub->ih_length = 300;
    switch( variant )
    {
    case 0:
        if (pub->slot_mc == 200){
            pub->ih_length = 300;
            pub->variant_ih = good[0];
        }else{
            pub->ih_length = 400;
            pub->variant_ih = good50[0];
        }
        break;
    case 1:
        if (pub->slot_mc == 200){
            pub->ih_length = 300;
            pub->variant_ih = med[0];
        }else{
            pub->ih_length = 100;
            pub->variant_ih = med50[0];
        }
        break;
    case 2:
        if (pub->slot_mc == 200){
            pub->ih_length = 300;
            pub->variant_ih = bad1[0];
        }else{
            pub->ih_length = 50;
            pub->variant_ih = bad501[0];
        }
        break;
    case 3:
        if (pub->slot_mc == 200){
            pub->ih_length = 300;
            pub->variant_ih = bad2[0];
        }else{
            pub->ih_length = 30;
            pub->variant_ih = bad502[0];
        }
        break;
    case 4:
        pub->variant_ih = nullptr;
        break;
    case 5:
        pub->variant_ih = nullptr;
        break;
    default:
        pub->variant_ih = nullptr;
        break;
    }
}

void  TFadings::form_skywave_output(QVector<fft_complex>* doubleSlot,QVector<fft_complex>* chanSignal, int chan,int nwave,float sh)
{
    ChannelParams* pub = cs[chan];
    channel_private*	priv	= (channel_private*) pub->Private;
    if ((nwave == 0)&&(pub->variant_ih != nullptr)){
        GenerQ( pub,false );
        floatToFile(flog, 0, tr("").toUtf8().data(),priv->Q[0][0], 6,2,'f',0);
        floatToFile(flog, 0, tr(" ").toUtf8().data(),priv->Q[1][0], 6,2,'f',0);
        floatToFile(flog, 0, tr(" ").toUtf8().data(),priv->Q[2][0], 6,2,'f',0);
        floatToFile(flog, 0, tr(" ").toUtf8().data(),priv->Q[3][0], 6,2,'f',1);
    }
    fft_complex qq;
    int delay = priv->dt[nwave];
    depart[chan][skyIndex[nwave]]->clear();
    for (int ii=0;ii<slotLength;ii++) {
        depart[chan][skyIndex[nwave]]->append(chanSignal->at(ii));
    }
    skyIndex[nwave]++;
    skyIndex[nwave] %= 3;

    QVector<fft_complex> departs;
    int index1=skyIndex[nwave];
    for (int ii = 0;ii < 3;ii++) {
        departs.append(depart[chan][index1]->toVector());
        index1++;
        index1 %= 3;
    }
    doubleSlot->clear();
    int start = replaceOffset(delay, nwave) ;
    int finish = static_cast<int>(fminf(departs.count(), start+2*slotLength));
    for (int ii=start;ii<finish;ii++) {
       if (pub->variant_ih == nullptr) qq = fft_complex(1,0);
       else{
          qq = fft_complex(priv->Q[2*nwave][ii%slotLength],priv->Q[2*nwave+1][ii%slotLength]);
       }
       doubleSlot->append(qq* departs.at(ii));
    }
    doppler_add(doubleSlot, sh);
}

void TFadings::doppler_add(QVector<fft_complex>* sig, float sh)
{
    fft_complex cs;
    float frqCnt=0;
    for (int ii=0;ii<sig->count();ii++) {
         cs = fft_complex(cosf(frqCnt),sinf(frqCnt));
         //chanSignal[ii]=  fft_complex((*(inChan+ii)).real(),(*(inChan+ii)).imag());
         sig->replace(ii,cs*sig->at(ii));
         frqCnt += sh;
         if (frqCnt > 2*pi) frqCnt -= 2*pi;
    }
}

void TFadings::setBfDrift(float drift)
{
    bf_drift_velocity = drift;
}

void TFadings::setSlotLength(int len)
{
    slotLength = len;
    for (int ii=0;ii<3;ii++) {
        offset[ii] = (float)(slotLength/2-1);
    }
}

int TFadings::replaceOffset(int dt,int nw)
{

    offset[nw] = offset[nw] + bf_drift_velocity ;
    int nset = (int)(offset[nw]+ dt) % slotLength;
    if (nset<0) nset = slotLength - nset;
    if (nset>slotLength) nset = nset - slotLength;
    //qDebug() << "offset="<<nset;
    return nset;
}

int TFadings::getSkyWaves(int chan)
{
    ChannelParams* pub = cs[chan];
    return pub->n_sky;
}

float TFadings::getgWave(int chan)
{
    ChannelParams* pub = cs[chan];
    return pub->gwave;
}

int	 TFadings::ctrl_channel_init( ChannelParams cp, int ind )
{
    cs[ind]->frequency_spreading	= cp.frequency_spreading;
    cs[ind]->gwave			= cp.gwave;
    cs[ind]->Length			= cp.Length;
    cs[ind]->n_sky			= cp.n_sky;
    cs[ind]->orthInt		= cp.orthInt;
    cs[ind]->Shift			=  2*pi*cp.Shift/(cp.orthInt*100);
    cs[ind]->SNR			= cp.SNR;
    cs[ind]->t1				= cp.t1;
    cs[ind]->t2				= cp.t2;
    cs[ind]->t3				= cp.t3;
    cs[ind]->slot_mc       =cp.slot_mc;
    cs[ind]->ih_number = cp.ih_number;
    cs[ind]->var_number = cp.var_number;
    cs[ind]->variant_ih = nullptr;

    if (cp.ih_number >= 0){
            get_impulse_response(cs[ind],cp.ih_number);
    }
    return	mdm_channel_init( cs[ind] );
}

float TFadings::getSNR(int chan)
{
    return cs[chan]->SNR;
}

int	 TFadings::mdm_channel_init(ChannelParams* pub)
{
    int					i;
    bool noalloc = (pub->Private == nullptr);
    channel_private*	priv = (channel_private*)pub->Private;
    if (noalloc)
        priv = (channel_private *)malloc(sizeof(channel_private));
    float freq_kgc = static_cast<float>(pub->orthInt)/10;
    priv->dt[0]			= (int)(freq_kgc*pub->t1);
    priv->dt[1]			= (int)(freq_kgc*(pub->t1 + pub->t2));
    priv->dt[2]			= (int)(freq_kgc*(pub->t1 + pub->t2 + pub->t3));
    pub->SNR			= (float) exp(-0.11512925464970228420089957273422*pub->SNR);

    for(i=0; i < 6; i++)
    {
        if (noalloc)
            priv->Q[i]	= (float*) malloc(pub->Length*sizeof(float));
        if( priv->Q[i] == nullptr )
            return CHANNEL_RET_CANNOT_ALLOCATE_MEMORY;
    }
    pub->Private	= priv;
    if ((pub->ih_number >= 0)&&(pub->variant_ih != nullptr)) GenerQ( pub ,true);
    return CHANNEL_RET_OK;
}


void TFadings::mdm_channel_close( int ind)
{
    int		i;
    ChannelParams* pub = cs[ind];
    channel_private*	priv	= (channel_private*) pub->Private;

    for(i=0; i < 2*pub->n_sky; i++)
        free(priv->Q[i]);

    free(priv);
    free(pub);
}

void	TFadings::getChannelParams(int chanNum, ChannelParams* cp )
{
    if(chanNum < 0){
        cp->frequency_spreading = 0.0f;
        cp->Shift   = 0.0f;
        cp->SNR    = 30;
        cp->t2 = 0;
    }else{
        cp->frequency_spreading = cs[chanNum]->frequency_spreading;
        cp->var_number = cs[chanNum]->var_number;
        cp->t2 = cs[chanNum]->t2;
        cp->ih_number = cs[chanNum]->ih_number;
        cp->Shift   = static_cast<float>(cs[chanNum]->orthInt*100*cs[chanNum]->Shift/(2*pi));
        cp->SNR    = static_cast<float>(-20.0*log10( static_cast<double>(cs[chanNum]->SNR )));
    }
}

void TFadings::GenerQ(ChannelParams* pub, bool init)
{
    channel_private*	priv	= (channel_private*) pub->Private;
    float yy	= 0;
    fft_complex ihx;

    for(int ii=0; ii < 3; ii++){
        while (randArray[ii]->count() < pub->ih_length) {
            yy	= (rand()%1001)*0.002 - 1.0;
            randArray[ii]->append(yy);
        }

        randArray[ii]->remove(0);
        yy	= (rand()%1001)*0.002 - 1.0;
        randArray[ii]->append(yy);

        fft_complex x = fft_complex(0,0);
        for (int kk=0;kk<pub->ih_length;kk++) {
            ihx = fft_complex(*(pub->variant_ih+3*kk+1),*(pub->variant_ih+3*kk+2));
            x += ihx * randArray[ii]->at(pub->ih_length-kk-1);
        }
        x = x/(float)pub->ih_length;

        if (init) {
            priv->Q[2*ii][pub->Length-1] = real(x);
            priv->Q[2*ii+1][pub->Length-1] = imag(x);
        }
        else{
            yy = priv->Q[2*ii][pub->Length-1];
            interpol(pub,yy,real(x),2*ii);
            yy = priv->Q[2*ii+1][pub->Length-1];
            interpol(pub,yy,imag(x),2*ii+1);
        }
    }
}

void TFadings::interpol(ChannelParams* pub,float  q0,float  q1,int ii)
{
    channel_private*	priv	= (channel_private*) pub->Private;
    for (int jj=0;jj<pub->Length;jj++) {
        priv->Q[ii][jj] = q0 + (jj+1)*(q1-q0)/pub->Length;
    }
}

