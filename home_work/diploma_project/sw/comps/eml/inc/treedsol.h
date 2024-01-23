#ifndef TREEDSOL_H
#define TREEDSOL_H

#include <QObject>

#define GF_SIZE  2048    // максимальный размер поля Галуа (2^m)
#define RS_2T  110        // максимальная длина кода Рида-Соломона (2t)

typedef enum Error_Codes_e
{
    MDM_RET_OK,
    MDM_RET_CANNOT_ALLOCATE_MEMORY,
    MDM_RET_IMPROPER_MODE
} Error_Codes;

enum SyndromSate
{
    NO_SYNDROMES=0,
    FOUND_SYNDROMES=1
};

enum ChenState
{
    SKIP_ALGORTHM_CHEN=0,
    START_ALGORTHM_CHEN=1
};

enum ForniState
{
    SKIP_ALGORTHM_FORNI=0,
    START_ALGORTHM_FORNI=1
};

enum GF
{
    GF_8=8,
    GF_16=16,
    GF_32=32,
    GF_64=64,
    GF_128=128,
    GF_256=256,
    GF_512=512,
    GF_1024=1024,
    GF_2048=2048
};

class TReedSol : public QObject
{
    Q_OBJECT
public:
    explicit TReedSol(QObject *parent = nullptr);

    int		RSCodec_init(int mode, int n, int k );
    void Encoder(const QList<int> *message, QList<int> *codeword);
    unsigned int Decoder(QList<int> *channelword, QList<int> *decodeword);

private:
    int		mode;                //Кол-во элементов поля (m=log2(mode)-степень расширения поля)
    int		N;                      //Длина вых. кодового слова (мб. укороченного)
    int		K;                      //Длина вх.  данных (мб. укороченных)

    int     N_long;                 //Длина вых. кодового слова (неукороченного!)
    int     K_long;                 //Длина вх.  данных (неукороченных!)
    int     poly_m;                 //Примитивный полином в 10-ом виде

    int    data_buf[GF_SIZE];          //Буфер для перезаписи вх данных в кодер
    int     decodRecBuf[GF_SIZE];       //Буфер для перезаписи вх данных в декодере

    int     alog_tab[GF_SIZE];           //Элементы поля (mode)
    int     log_tab[GF_SIZE];            //Cтепени элементов поля (mode)

    int    generatePoly[RS_2T+1];       //Порождающий полином (N_long-K_long+1)
    int     generatePolyAlog[RS_2T+1];   //Поражд. полинома записанный через индексы (N_long-K_long+1)
    int     b[RS_2T];                  //Кодовая часть слова           (N_long-K_long)

    int		syndrome[RS_2T+1];           // Синдромы syndrome (нужно чистить)         (N_long-K_long+1)
    int		corrPoly[RS_2T+1];           //Корректирующий многочлен (corr) (нужно чистить)    (N_long-K_long+1)
    int		omegaPoly[GF_SIZE];          // lam_sig полином (syndrome(x)*lambdaPoly X^n-1 (нужно чистить)  N_long
    int		lambdaPoly[RS_2T];         // Лямбда полином  sigma многочлен (ecp) (нужно чистить) (N_long-N_long-K_long)
    int		lambdaPrimPoly[GF_SIZE];     //Перезаписанная Lamda на 3-ем этапе декодирования (нужно чистить) (N_long)
    int     ecp_new[RS_2T];            //Lambda_new (N_long-K_long)
    int     errorLocation[GF_SIZE];      // store error locations (N_long)
    int     errorPosition [GF_SIZE];     //Буфферный массив позиции оишбки (N_long)

    void clearDecodingArr(); //static
    int getdeg(int arr[],int length);
    void getDecodArr(QList<int> *decodeword);
    void generateGF();
    void createGeneratePoly();
    int calculationSyndrome();
    int algorithmBerlecampMassey();
    int algorithmCheinSerch();
    void algorithmForni();

signals:

};

#endif // TREEDSOL_H
