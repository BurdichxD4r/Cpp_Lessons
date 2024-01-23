#ifndef TCONSOLE_H
#define TCONSOLE_H

#include <QWidget>
#include "slot_params.h"
#include "clogger.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QLineEdit;
class QPushButton;
class QComboBox;
class QTextEdit;
class QString;
QT_END_NAMESPACE


// Заголовок WAV файла
struct WAV_HEADER
{
    // Фрагмент RIFF
    QByteArray chunkId;    // Начало RIFF-цепочки. Символы “RIFF” в ASCII кодировке (0x52494646 в big-endian представлении).
    quint32 chunkSize;     // Размер цепочки RIFF, исключены поля chunkId и chunkSize (РАЗМЕР_ФАЙЛА - 8). Заполняется при закрытии файла
    // Фрагмент описания формата
    QByteArray format;     // Содержит символы “WAVE” (0x57415645 в big-endian представлении)
    QByteArray subchunk1Id;// Содержит символы “fmt “(0x666d7420 в big - endian представлении)
    quint32 subchunk1Size; // 16 для формата PCM.
    quint16 audioFormat;   // Аудио формат. Для PCM = 1 (то есть, линейное квантование).
    quint16 numChannels;   // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    quint32 sampleRate;    // Частота дискретизации.
    quint32 byteRate;      // Количество байт, переданных за секунду воспроизведения.
    quint16 blockAlign;    // Количество байт для одного сэмпла, включая все каналы.
    quint16 bitsPerSample; // Количество бит в сэмпле. Так называемая “глубина” или точность звучания. 8 бит, 16 бит и т.д.
    // Фрагмент DATA
    QByteArray subchunk2Id;// Содержит символы “data” (0x64617461 в big-endian представлении)
    quint32 subchunk2Size; // Количество байт в области данных. Заполняется при закрытии файла
};
// Переопределение операторов
QDataStream& operator << (QDataStream &out, const WAV_HEADER &any);
QDataStream& operator >> (QDataStream &in,        WAV_HEADER &any);

class TDataFile : public QWidget
{
    Q_OBJECT

public:
    TDataFile(QWidget *parent = nullptr);
    ~TDataFile();

private:

    QString variantLine;
    QTextEdit *tableShow;
    QLineEdit *currentShow;
    QPushButton* pbSave;
    QString snrMetric = "EbN0";

    QCheckBox *cbModulateToFile;  // запись сигнала с модулятора в файл
    QCheckBox *cbDemodulateFromFile;  // на вход демодулятора подавать данные из файла
    QCheckBox *cbEndlessFile;  // по концу файла переходим на его начало
    QPushButton *pbOpen; // открыть файл
    QComboBox *cbFName; // список файлов для воспроизведения
    QComboBox *cbfType; // тип файла(WAV,RAW)

    QFile* saveFile;
    QDataStream* dataStream;
    CLogger cong;

    QFile* incomingFile;
    QDataStream* incomingStream;

    QString sendFilesDirDefault="";   // каталог для диалога выбора файла
    QString shortFileName;  // имя файла без каталога
    QString fullFileName;  // полное имя файла

    short_complex inModem[SLOT_LEN_MAX];
    QList<int_complex>* departf[3];  // три последовательных слота на выходе канала
    QVector<int_complex>* chanExitf;  // объединение depart[3] в поток
    // Структура заголовка файлов записи/чтения
    WAV_HEADER fileOutWavHdr;
    int chnum=0;

    //Это оставшийся размер подцепочки (для PCM 16)
    quint32 subchunk1Size = 16;
    // Аудио формат. Для PCM = 1 (то есть, линейное квантование)
    quint16 audioFormat = 1;
    // Количество каналов. Моно = 1, Стерео = 2 и т.д.
    quint16 numChannels = 2;
    // Частота дискретизации
    quint32 sampleRate = 8000;
    // Количество байт, переданных за секунду воспроизведения
    quint32 byteRate;
    // Количество байт для одного сэмпла, включая все каналы
    quint16 blockAlign;
    // Количество бит в сэмпле. Так называемая “глубина” или точность звучания. 8 бит, 16 бит и т.д.
    quint16 bitsPerSample = 16;
    // Тип файла (WAV, или BIN)
    QString typeFile = "WAV";
    int variant;  // номер сигнальной конструкции при записи файла

    bool growTableShowLine(Modem_stream* ber);
    void writeWavHeader();

private slots:
    void saveCalculatedTable();
    void setSavingMode(bool state);
    void setInComingMode(bool state);

    void openFile();
    void setCurrentFileName(const QString& fileName);
    void reloadRecentFile(const QString &fileName);
    void fTypeChange(const QString& fType);

public slots:
    void showBerMeasure(bool mode, Modem_stream* ber);
    void setSnrMetric(const QString& str);
    void writeSlotToFile(short_complex* inChan,int len);
    void readSlotFromFile(short_complex* slot, int len);
    void buildInputSignal(int slotLen);
    void  fsetOptions(SLOT_VARIANT *vars);

signals:
    void readyReadFile(bool state);
    void writeMode(bool state);
    void chan_exit_sendf(QVector<int_complex>*);


};

#endif // TCONSOLE_H
