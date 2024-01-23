#include "tconsole.h"
#include "eml.h"
#include <QtGui>
#include <QtWidgets>
#include <QFile>

//#define		DO_LOG
#ifdef DO_LOG
#include <stdio.h>
#endif

// Переопределение операторов
QDataStream& operator << (QDataStream &out, const WAV_HEADER &any)
{
    // Установить порядок следования байт
    out.setByteOrder(QDataStream::LittleEndian);
    // RIFF chunk
    out.writeRawData(any.chunkId,4);
    out << quint32(any.chunkSize);
    out.writeRawData(any.format,4);
    out.writeRawData(any.subchunk1Id,4);
    // Format description chunk
    out << quint32(any.subchunk1Size);
    out << quint16(any.audioFormat);
    out << quint16(any.numChannels);
    out << quint32(any.sampleRate);
    out << quint32(any.byteRate);
    out << quint16(any.blockAlign);
    out << quint16(any.bitsPerSample);
    // Data chunk
    out.writeRawData(any.subchunk2Id,4);
    out << quint32(any.subchunk2Size);
    // Выход
    return out;
};

QDataStream& operator >> (QDataStream &in, WAV_HEADER &any)
{
    // Установить порядок следования байт
    in.setByteOrder(QDataStream::LittleEndian);
    // RIFF chunk
    any.chunkId.resize(4);
    in.readRawData(any.chunkId.data(),4);
    in >> any.chunkSize;
    any.format.resize(4);
    in.readRawData(any.format.data(),4);
    any.subchunk1Id.resize(4);
    in.readRawData(any.subchunk1Id.data(),4);
    // Format description chunk
    in >> any.subchunk1Size;
    in >> any.audioFormat;
    in >> any.numChannels;
    in >> any.sampleRate;
    in >> any.byteRate;
    in >> any.blockAlign;
    in >> any.bitsPerSample;
    // Data chunk
    any.subchunk2Id.resize(4);
    in.readRawData(any.subchunk2Id.data(),4);
    in >> any.subchunk2Size;
    // Выход
    return in;
}

TDataFile::TDataFile(QWidget *parent)
    : QWidget(parent)
{
    for (int ii=0;ii<3;ii++) {
        departf[ii] = new QList<int_complex>();
    }
    chanExitf = new QVector<int_complex>();

    saveFile = new QFile();
    incomingFile = new QFile();
    tableShow = new QTextEdit(this);

    tableShow->document()->setMaximumBlockCount(300);
    //teChat->setMaximumHeight(100);

    currentShow = new QLineEdit(this);
    QSizePolicy sz = QSizePolicy();
    sz.setHorizontalPolicy(QSizePolicy::Expanding);
    currentShow->setSizePolicy(sz);

    pbSave = new QPushButton(this);
    pbSave->setText(tr("Сохранить"));
    pbSave->setToolTip(tr("Сохраняет  файл в подкаталог BER каталога приложения"));

    pbOpen = new QPushButton(this);
    pbOpen->setText(tr("Открыть"));
    pbOpen->setToolTip(tr("Открыть файл с данными для демодулятора"));
    cbFName = new QComboBox(this);
    cbFName->setEditable(true);

    cbfType = new QComboBox(this);
    cbfType->addItem("WAV");
    cbfType->addItem("BIN");
    cbfType->setToolTip(tr("Тип файла"));

    cbModulateToFile = new QCheckBox(this);
    cbModulateToFile->setText("Запись");
    //cbModulateToFile->setEnabled(false);
    cbModulateToFile->setToolTip("Включить/выключить запись \nвыходного сигнала модема в файл");
    cbDemodulateFromFile = new QCheckBox(this);
    cbDemodulateFromFile->setEnabled(false);
    cbDemodulateFromFile->setText("Чтение");
    cbDemodulateFromFile->setToolTip("Сигнал на вход демодулятора \nпоступает из выбранного файла");

    cbEndlessFile = new QCheckBox(this);
    cbEndlessFile->setText("По кругу");
    cbEndlessFile->setToolTip("По концу файла переходим на его начало");

    QGridLayout *layout3 = new QGridLayout;
    layout3->addWidget(pbOpen,               0,0,1,1);
    layout3->addWidget(cbFName,             0,1,1,4);
    layout3->addWidget(cbModulateToFile,           1,0,1,1);
    layout3->addWidget(cbfType,                        1,1,1,1);
    layout3->addWidget(cbDemodulateFromFile,  1,3,1,1);
    layout3->addWidget(cbEndlessFile,  1,4,1,1);

    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(pbSave);

    QVBoxLayout *layout2 = new QVBoxLayout;
    layout2->addWidget(currentShow);
    layout2->addWidget(tableShow);
    layout2->addLayout(layout1);
    layout2->addLayout(layout3);
    layout2->addStretch();

    setLayout(layout2);
    this->adjustSize();

    tableShow->setLineWrapMode(QTextEdit::NoWrap);
    connect(pbSave, &QPushButton::pressed, this, &TDataFile::saveCalculatedTable);
    connect(cbModulateToFile, &QCheckBox::stateChanged,this, &TDataFile::setSavingMode);
    connect(cbDemodulateFromFile, &QCheckBox::stateChanged,this, &TDataFile::setInComingMode);
    connect(pbOpen,&QPushButton::pressed, this, &TDataFile::openFile);
    connect(cbFName, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                               this, &TDataFile::reloadRecentFile);
    connect(cbfType, QOverload<const QString &>::of(&QComboBox::currentIndexChanged),
                               this, &TDataFile::fTypeChange);

    QString fname = QString("console_log%1.txt").arg(1);
    #ifdef	DO_LOG
        cong = newLogger(fname.toUtf8().data(),1,1);
    #else
        cong = newLogger(fname.toUtf8().data(),1,0);
    #endif
     strToFile(cong,1,tr("class TDataFile").toUtf8().data(),1);
}

TDataFile::~TDataFile()
{
    if (saveFile->isOpen()){
        writeWavHeader();
        saveFile->close();
    }
    deleteLogger(cong);
}

void TDataFile::openFile()
{
    QString selectedFilter;
    QString fileName = QFileDialog::getOpenFileName(this,
                                                    tr("QFileDialog::getOpenFileName()"),
                                                    sendFilesDirDefault,
                                                    tr("All Files (*);;Text Files (*.txt)"),
                                                    &selectedFilter
                                                    );
    if (!fileName.isEmpty()){
        cbFName->addItem(fileName);
        cbFName->setCurrentIndex(cbFName->count() - 1);
    }
}

void TDataFile::setCurrentFileName(const QString& fileName)
{
    //qDebug() << "setCurrentFileName="<<fileName;
    int npos = fileName.lastIndexOf('/');//QDir::separator());
    if(npos>0) {
        sendFilesDirDefault =fileName.left(npos);
        shortFileName = fileName.right(fileName.length() - npos-1);
        fullFileName = fileName;
        cbFName->setCurrentText(shortFileName );
        cbFName->setToolTip(fileName);
        cbDemodulateFromFile->setEnabled(true);
        cbDemodulateFromFile->setChecked(false);
    }
}

void TDataFile::reloadRecentFile(const QString& fileName)
{
    //readFile(fileName);
    setCurrentFileName(fileName);
    qDebug() << "reloadRecentFile "<< fileName;
}

void TDataFile::fTypeChange(const QString& fType)
{
    qDebug() << "fType= "<< fType;
    typeFile = fType;
}

void TDataFile::showBerMeasure(bool mode,Modem_stream* ber)
{
    QString str="";
    if (!mode){
        if (!(ber->vartext).isEmpty()){
            str = QString(" %1  SNR=%2 recv:%3 err:%4 byte_err:%5" ).arg(ber->vartext).arg(ber->snr).arg(ber->recv).
                    arg(ber->error).arg(ber->byte_error);
            currentShow->setText(str);
            //qDebug() << ber->vartext << ber->recv << ber->error<<ber->snr;
            if (growTableShowLine( ber)) {
                tableShow->append(variantLine);
            }
        }
    }
}

bool TDataFile::growTableShowLine(Modem_stream* ber)
{
    QString str;
    bool ok;
    int number = 0;
    float err = 0;
    bool completed = false;

    if (ber->snr == 0){
        variantLine.clear();
        if (ber->varnum == 0) tableShow->clear();
        str = ber->vartext.left(3);
        int num = str.toInt(&ok,10);
        if (ok) number = num;  // номер варианта, выделенный из ber->vartext
        str = QString("%1").arg(number,2,10,QLatin1Char('0'));
        variantLine.append(str);
    }
    err = (static_cast<float>(ber->error)*100)/ber->recv;
    str = QString("%1").arg(err,8,'e',2);
    variantLine.append('\t'+str);

    if (ber->snr == snr_index_max){
        completed = true;
    }
    return completed;
}

void TDataFile::saveCalculatedTable()
{
    if (tableShow->toPlainText().isEmpty()) return;
    //имя файла типа 2021-12-02_11:06:00_bit_errors.txt
    QString fname,str,itog,sdate,stm_end;

    QDateTime dtBegin = QDateTime::currentDateTime();
    fname = dtBegin.toString("yyyy-MM-dd_hh-mm-ss_");
    fname.append(snrMetric+"_errs.txt");

    QString dirName = QCoreApplication::applicationDirPath()+"/BER";

    QDir fDir = QDir(dirName);
    if (!fDir.exists()) fDir.mkdir(dirName);
    fname = dirName +"/"+fname;
    qDebug() << fname;

    QFile file(fname);
    file.open(QIODevice::WriteOnly);
    QTextStream out(&file);
    out << tableShow->toPlainText();
    file.close();
}

void TDataFile::setSnrMetric(const QString& str)
{
    qDebug() << "setSnrMetric "<<str;
    snrMetric = str;
}

void TDataFile::setSavingMode(bool state)
{
    if (state){
        QString fname,str,dirName;
        QDateTime dtBegin = QDateTime();
        dtBegin = QDateTime::currentDateTime();
        fname = dtBegin.toString("yyyy-MM-dd_hh-mm-ss_");
        str.setNum(variant);
        fname.append(str);
        dirName.prepend(QCoreApplication::applicationDirPath());

        if(typeFile == "WAV") {
            dirName.append("/WAV");
            fname.append(".wav");
        }else{
            dirName.append("/BIN");
            fname.append(".bin");
        }

        QDir fDir = QDir(dirName);
        if (!fDir.exists()) fDir.mkdir(dirName);
        fname = dirName +"/"+fname;
        qDebug() << fname;

        saveFile->setFileName(fname);
        if (!saveFile->open(QFile::WriteOnly)){
            return;
        }
        if(typeFile == "WAV") saveFile->seek(44);
        dataStream = new QDataStream(saveFile);
        dataStream->setByteOrder(QDataStream::LittleEndian);
        emit writeMode(true);
    }else{
            if (saveFile->isOpen()){
                writeWavHeader();
                delete dataStream;
                saveFile->close();
            }
            emit writeMode(false);
    }
}

void TDataFile::writeSlotToFile(short_complex* inChan,int len)
{
    if (saveFile->isOpen()){
        for (int ii=0;ii<len;ii++) {
             *dataStream << (*(inChan+ii)).real()  << (*(inChan+ii)).imag();
        }
    }else{
        if (!cbModulateToFile->isEnabled()) cbModulateToFile->setEnabled(true);
    }
}

void TDataFile::setInComingMode(bool state)
{
    if (state){
        incomingFile->setFileName(fullFileName);
        if (!incomingFile->open(QFile::ReadOnly)){
            emit readyReadFile(false);
            return;
        }
        incomingStream = new QDataStream(incomingFile);
        incomingStream->setByteOrder(QDataStream::LittleEndian);

        if (typeFile == "WAV") {
            *incomingStream >>  fileOutWavHdr;
            qDebug() << "fileOutWavHdr.numChannels="<< fileOutWavHdr.numChannels;
            incomingFile->seek(44);
        }else{
            fileOutWavHdr.numChannels = 2;
        }

        emit readyReadFile(true);
    }else{
            emit readyReadFile(false);
            if (incomingFile->isOpen()){
                delete incomingStream;
                incomingFile->close();
            }
    }
}

void TDataFile::readSlotFromFile(short_complex* slot, int len)
{
    short xreal,ximag;
    if (incomingFile->isOpen()){
        for (int ii=0;ii<len;ii++) {
             *incomingStream >> xreal>>ximag;
             *(slot+ii) = short_complex(xreal,ximag);
        }
    }
}

void TDataFile::buildInputSignal(int slotLen)
{
    //qDebug() << "buildInputSignal() slotlen="<< slotLen;
    static int index=0;
    departf[index]->clear();
    short xreal,ximag;
    QList <int_complex> cycle;
    int_complex sig;
    for (int ii=0;ii<slotLen;ii++) {
        cycle.clear();
        for (int jj=0;jj<fileOutWavHdr.numChannels/2;jj++) {
            *incomingStream >> xreal>>ximag;
            sig = int_complex(xreal, ximag);
            cycle.append(sig);

            if (incomingFile->atEnd()) {
                chnum = 0;
                if (cbEndlessFile->isChecked())
                    incomingFile->seek(0);
                else
                    cbDemodulateFromFile->setChecked(false);
            }
        }
        departf[index]->append(cycle.at(chnum));
    }
    chnum = (chnum+1)%(fileOutWavHdr.numChannels/2);

    intToFile(cong,0,tr(" ").toUtf8().data(),index,0,10,' ',0);
    intToFile(cong,0,tr(" ").toUtf8().data(),departf[index]->first().real(),0,16,' ',0);
    intToFile(cong,0,tr(" ").toUtf8().data(),departf[index]->first().imag(),0,16,' ',0);
    intToFile(cong,0,tr(" ").toUtf8().data(),departf[index]->last().real(),0,16,' ',0);
    intToFile(cong,0,tr(" ").toUtf8().data(),departf[index]->last().imag(),0,16,' ',0);
    intToFile(cong,0,tr(" ").toUtf8().data(),departf[index]->count(),0,10,' ',1);
    index++;
    index %= 3;

    QVector<int_complex> departs;
    int index1=index;
    for (int ii = 0;ii < 3;ii++) {
        departs.append(departf[index1]->toVector());
        index1++;
        index1 %= 3;
    }
    *chanExitf = departs.mid(slotLen/2, 2*slotLen);
    emit chan_exit_sendf(chanExitf);
}

// Запись заголовка
void TDataFile::writeWavHeader()
{
    if(typeFile == "WAV")
    {
        // Заполнить структуру заголовка WAV-файла
        // Фрагмент RIFF
        fileOutWavHdr.chunkId       = "RIFF";
        fileOutWavHdr.chunkSize     = saveFile->size() - 8;
        // Фрагмент описания формата
        fileOutWavHdr.format        = "WAVE";
        fileOutWavHdr.subchunk1Id   = "fmt ";
        fileOutWavHdr.subchunk1Size = subchunk1Size;
        fileOutWavHdr.audioFormat   = audioFormat;
        fileOutWavHdr.numChannels   =numChannels;
        fileOutWavHdr.sampleRate    = sampleRate;
        fileOutWavHdr.byteRate      = sampleRate  * numChannels * bitsPerSample / 8 ;
        fileOutWavHdr.blockAlign    = numChannels * bitsPerSample / 8;
        fileOutWavHdr.bitsPerSample = bitsPerSample;
        // Фрагмент DATA
        fileOutWavHdr.subchunk2Id   = "data";
        fileOutWavHdr.subchunk2Size = saveFile->size() - 44;

        // Записать заголовок
        saveFile->seek(0);
        *dataStream << fileOutWavHdr;
    }

}

void  TDataFile::fsetOptions(SLOT_VARIANT *vars)
{
   sampleRate = vars->freq_discr;
   variant = vars->num;
}


