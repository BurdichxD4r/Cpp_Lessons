#include "treedsol.h"
#include <QDebug>

TReedSol::TReedSol(QObject *parent) : QObject(parent)
{
//    data_buf = new int[256];
//    generatePoly = new int[rs2t+1];
}

// Mode - число возможных кодовых символов
// N - число кодовых символов в кодируемом блоке
// K - число информационных битов, подлежащих кодированию
int	TReedSol::RSCodec_init(int mode, int n, int k)
{

    this->K		= k;
    this->N		= n;
    this->mode	= mode;

    N_long=mode-1;

    //Проверка укороченный ли код:
    if(N!=N_long)
        K_long=this->K+(N_long-this->N);
    else
        K_long=this->K;

    if (this->N >=mode||this->K>=this->N) return MDM_RET_IMPROPER_MODE;

    //Выбираем примитивный полином:
    switch (mode)
    {
    case GF_8:
        poly_m=11; break;  //  13
    case GF_16:
        poly_m=25;  break;  //  19
    case GF_32:
        poly_m=37;  break;  //  41   47   55   59   61
    case GF_64:
        poly_m=67;  break;  //  91   97   103  109  115
    case GF_128:
        poly_m=137;  break; //  131    143    145    157
    case GF_256:
        poly_m=285;  break; //  see the notes
    case GF_512:
        poly_m=529;  break;
    case GF_1024:
        poly_m=1033; break;
    case GF_2048:
        poly_m=2053; break;
    default:
        return MDM_RET_IMPROPER_MODE;
    }

    generateGF();
    createGeneratePoly();

    return	MDM_RET_OK;
}

void TReedSol::clearDecodingArr()
{
    memset( ecp_new,0,sizeof ( ecp_new));
    memset( syndrome,0,sizeof (syndrome));
    memset(corrPoly,0,sizeof (corrPoly));
    memset(lambdaPoly,0,sizeof (lambdaPoly));
    memset(lambdaPrimPoly,0,sizeof (lambdaPrimPoly));
    memset(omegaPoly,0,sizeof(omegaPoly));
    memset(errorLocation,0,sizeof (errorLocation));
    memset(errorPosition,0,sizeof (errorPosition));

}

int TReedSol::getdeg(int arr[],int length)
{
    int deg=-1;

    for (int i=0;i<length;i++)
        if(arr[i]!=0)
            deg=i;
    return deg;
}

//Возвращаем результат декодирования:
void TReedSol::getDecodArr(QList<int>*  decodeword)
{

    for (int i = 0; i < this->K; i++){
        decodeword->append(decodRecBuf[i+(N_long - this->N)]);
    }
}


void TReedSol::generateGF()
{
    int init_zero=1;

    //Генерируем поле:
    int lfsr = 0x01;
    unsigned msb, period = 0;

    do {
        alog_tab[period]=lfsr;
        log_tab[lfsr]=(int)period;

        lfsr <<= 1;
        msb =(unsigned)(lfsr & (N_long+1));

        if (msb ==(unsigned)N_long+1)  lfsr ^= poly_m;
        ++period;

    } while(period != (unsigned)N_long);
    log_tab[0]=-1;

    //Генерируем порождающий многочлен:
     int i,j;

    generatePoly[0] = alog_tab[init_zero];
    generatePoly[1] = 1;
    for (i=2; i<=N_long-K_long; i++)
    {
        generatePoly[i] = 1;
        for (j=i-1; j>0; j--)
            if (generatePoly[j] != 0)
                generatePoly[j] = generatePoly[j-1]^
                        alog_tab[(log_tab[generatePoly[j]]+i+init_zero-1)%N_long];
            else
                generatePoly[j] = generatePoly[j-1];
        generatePoly[0] = alog_tab[(log_tab[generatePoly[0]]+i+init_zero-1)%N_long];
    }

}

void TReedSol::createGeneratePoly()
{
    int init_zero=1;
     int i,j;

    generatePoly[0] = alog_tab[init_zero];
    generatePoly[1] = 1;
    for (i=2; i<=N_long-K_long; i++)
    {
        generatePoly[i] = 1;
        for (j=i-1; j>0; j--)
            if (generatePoly[j] != 0)
                generatePoly[j] = generatePoly[j-1]^
                        alog_tab[(log_tab[generatePoly[j]]+i+init_zero-1)%N_long];
            else
                generatePoly[j] = generatePoly[j-1];
        generatePoly[0] = alog_tab[(log_tab[generatePoly[0]]+i+init_zero-1)%N_long];
    }

    //Записываем пораждающий многочлен через log_tab:
    for (i=0; i<=N_long-K_long; i++)
        generatePolyAlog[i] = log_tab[generatePoly[i]];
}

int TReedSol::calculationSyndrome()
{
    int SyndromState=NO_SYNDROMES;
    int init_zero=1;

    //Считаем синдромы:
    for (int i=init_zero; i<N_long-K_long+init_zero; i++)
    {
        for (int j=0; j<N_long; j++)
        {
            if (log_tab[decodRecBuf[j]] != -1) syndrome[i-init_zero]^=
                    alog_tab[(log_tab[decodRecBuf[j]] + i*(N_long-1-j))%N_long];
        }
        if (syndrome[i-init_zero]!=0) SyndromState=FOUND_SYNDROMES;     //Если хотябы один синдром !=0 будет попытка декодирования
    }

    return  SyndromState;

}


int TReedSol::algorithmBerlecampMassey()
{

    // Berlecamp-Massey algorithm (Поиск кол-ва ошибок)
    int d=0;
    int i=0; lambdaPoly[0]=1; ecp_new[0]=1; corrPoly[1]=1; int numErrorsFound=0;

    while (i<N_long-K_long){
        ++i;

        // calculate discrepancy d
        d=syndrome[i-1];
        if (i>1) for (int j=1;j<=numErrorsFound;j++) if (lambdaPoly[j]!=0 && syndrome[i-1-j]!=0)
            d^=alog_tab[(log_tab[lambdaPoly[j]]+log_tab[syndrome[i-1-j]])%N_long];

        // update ecp
        if (d==0) {
            for (int j=N_long-K_long;j>0;j--)
                corrPoly[j]=corrPoly[j-1];
            corrPoly[0]=0;}


        else  {   for (int j=0;j<N_long-K_long;j++){
                if (corrPoly[j]!=0)
                    ecp_new[j]^=alog_tab[(log_tab[corrPoly[j]]+log_tab[d])%N_long];
            }

            if (2*numErrorsFound<i) {

                numErrorsFound=i-numErrorsFound;

                for (int j=0;j<N_long-K_long;j++){
                    corrPoly[j]=0;
                    if (lambdaPoly[j]!=0) corrPoly[j]=alog_tab[(log_tab[lambdaPoly[j]] -
                            log_tab[d]+N_long)%N_long];
                }
            }

            for (int j=N_long-K_long;j>0;j--)
                corrPoly[j]=corrPoly[j-1];
            corrPoly[0]=0;
            for (int j=0;j<N_long-K_long;j++)
                lambdaPoly[j]=ecp_new[j];

        }
    }

    return  numErrorsFound; //Возврат найденного числа ошибок

}

int TReedSol::algorithmCheinSerch()
{

    int cnCh=0;
    int num_err=0;

    for (int i=0; i<N_long; i++)
    {

        //По многочлену сигма (ecp) определяем положения ошибки слева на право. Таким образом в пременной
        //num_err содержится индекс места в массиве, где содержится ошибка
        for (int j=0; j<=(N_long-K_long)/2; j++)    if (lambdaPoly[j]!=0)
            errorLocation[i]^=alog_tab[(log_tab[lambdaPoly[j]]+i*j)%N_long];

        if (errorLocation[i]==0)
        {
            num_err = N_long-1-abs(i-N_long)%N_long;
            errorPosition[cnCh]=num_err;           //Запоминаем позицию ошибки
            cnCh++;
        }
    }

    return cnCh;

}

void TReedSol::algorithmForni()
{

    // 1. Create syndrome polynomial syndrome(x)
    for (int i=N_long-K_long; i>0; i--) syndrome[i]=syndrome[i-1];
    syndrome[0]=1;

    // 2. Find the expression for the product of lambdaPoly(x) and sigma(x) mod x^(2t+1)
    for (int i=0; i<=N_long-K_long; i++)
    {
        for (int j=0; j<=(N_long-K_long)/2; j++)
            if ((syndrome[i]!=0) && (lambdaPoly[j]!=0))
                omegaPoly[i+j] ^= alog_tab[(log_tab[syndrome[i]]+
                        log_tab[lambdaPoly[j]])%N_long];
    }

    //________________________________________________________________________________

    // 3. ECP formal derivative calculation (shift odd terms one position to the left)
    //Lam_prim=Lam
    for (int i=0; i<(N_long-K_long)/2; i++)
    {
        lambdaPrimPoly[2*i]=lambdaPoly[2*i+1];
        lambdaPrimPoly[2*i+1]=0;
    }

    //_________________________________________________________________________________

    // 4. Get the error values
    int cnForn=0;
    int err_val=0;
    int init_zero=1;

    for (int i=0;i<N_long;i++)
        if (errorLocation[i]==0)
        {

            // a. find (alpha^err)^(2-init_zero)
            int LSig=0; int denom=0;
            for (int j=0;j<=N_long-K_long;j++){
                if (omegaPoly[j]!=0) LSig^=
                        alog_tab[(log_tab[omegaPoly[j]]+j*i)%N_long];
                if (lambdaPrimPoly[j]!=0) denom^=
                        alog_tab[(log_tab[lambdaPrimPoly[j]]+j*i)%N_long];
            }

            //Получаем значение ошибки которая находится на месте num_err.
            //Чтобы исправить ошибку в массиве принятых данных нужно пибавить к массиву (в поизиции num_err)
            //err_val по модулю N_long. Тем самым мы исправим нашу ошибку:
            err_val=alog_tab[((N_long-i)*(2-init_zero) +
                                  log_tab[LSig] + N_long - log_tab[denom])%N_long];

            //Исправляем ошибку:
            decodRecBuf[errorPosition[cnForn]]=
                    decodRecBuf[errorPosition[cnForn]]^err_val; //Исправляем ошибку xor в поле GF
            cnForn++;
        }

}

//Кодер:
//message - вх. сообщение (длины K_long);
//codeword - кодовое слово (длины N_long);
void TReedSol::Encoder(const QList<int>*  message, QList<int>* codeword)
{

     int i,j;
    int feedback;

    memset(data_buf,0,sizeof(data_buf));
    memset(b,0,sizeof(b));
    codeword->clear();
    //Перезаписываем вх данные (если код укор. заполняем начало нулями)
    for (i=0;i<K;i++)
        data_buf[i+(N_long-N)]=message->at(i);

    //Кодирование:
    for (i=0; i<K_long; i++)
    {
        feedback = log_tab[data_buf[i]^b[N_long-K_long-1]];
        if (feedback != -1)
        {
            for (j=N_long-K_long-1; j>0; j--)
                if (generatePolyAlog[j] != -1)
                    b[j] = b[j-1]^alog_tab[(generatePolyAlog[j]+feedback)%N_long];
                else
                    b[j] = b[j-1];
            b[0] = alog_tab[(generatePolyAlog[0]+feedback)%N_long];
        }
        else
        {
            for (j=N_long-K_long-1; j>0; j--)
                b[j] = b[j-1];
            b[0] = 0;
        }
    }

    //Сист часть:
    codeword->append(*message);
//    for (i = 0; i < K; i++)
//        codeword->append(message->at(i));

    //Кодовая часть:
    for (i = N-K-1; i >= 0; i--)
        codeword->append(b[i]);
}


//Декодер:
//channelword - вх. сообщение (длины N_long);
//codeword - декодированное слово (длины K_long);
unsigned int TReedSol::Decoder(QList<int>* channelword,QList<int>* decodeword)
{
   decodeword->clear();
    memset(decodRecBuf,0,sizeof(decodRecBuf));
    int ChenState=SKIP_ALGORTHM_CHEN;
    int ForniState=SKIP_ALGORTHM_FORNI;

    clearDecodingArr();

    //Перезаписываем вх слово(если слово укор. добавляем нулей)
    int NN=qMin(N,channelword->count());
    for (int i=0;i<NN;i++)
        decodRecBuf[i+(N_long-N)]=channelword->at(i);

    int SyndromState=calculationSyndrome();

    if(SyndromState==FOUND_SYNDROMES)
    {

        int numErrorBM=algorithmBerlecampMassey();

        int degecp=getdeg(lambdaPoly,N_long-K_long);
        if (degecp==numErrorBM) ChenState=START_ALGORTHM_CHEN;

        if (ChenState==START_ALGORTHM_CHEN)
        {

            int cnCh=algorithmCheinSerch();

            if (degecp==cnCh) ForniState=START_ALGORTHM_FORNI;

            if (ForniState==START_ALGORTHM_FORNI)
            {
                algorithmForni();

                getDecodArr(decodeword);
                return (quint32)numErrorBM;

            }

            else
            {
                getDecodArr(decodeword);
                return 0x7FFFFFFF;
            }
        }

        else
        {
            getDecodArr(decodeword);
            return 0x7FFFFFFF;
        }

    }

    else
    {
        getDecodArr(decodeword);
        return (quint32) 0;

    }
}
