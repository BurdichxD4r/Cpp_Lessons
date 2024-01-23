#include "tstream.h"
#include <QtGui>
#include <QtWidgets>

TStream::TStream(QWidget *parent)
    : QWidget(parent)
{

    pbClear = new QPushButton(this);
    pbClear->setText(tr("Очистка"));
    QHBoxLayout *layout1 = new QHBoxLayout;
    layout1->addStretch();
    layout1->addWidget(pbClear);
    layout1->setContentsMargins(0,0,0,0);

    QVBoxLayout *bitrates = new QVBoxLayout;
    bitrates->addLayout(layout1);
    bitrates->addWidget(createBitsTransfer());
    bitrates->addWidget(createDataTransfer());
    //bitrates->addStretch();
    setLayout(bitrates);

    this->adjustSize();
    connect(pbClear, &QPushButton::pressed, this, &TStream::clearData);
    textDefs();
}

TStream::~TStream()
{

}

QGroupBox *TStream::createBitsTransfer()
{
    bitRating = new QGroupBox(tr("Bit Error Rate"));
    bitRating->setMaximumHeight(85);
    bitsShow = new QTextEdit();
    bitsShow->setReadOnly(true);
    bitsShow->setMaximumHeight(75);

    QHBoxLayout *hbox = new QHBoxLayout;

    hbox->addWidget(bitsShow);
    hbox->setContentsMargins(0,0,0,0);
    bitRating->setLayout(hbox);

    return bitRating;
}

QGroupBox *TStream::createDataTransfer()
{
    dataTransfering = new QGroupBox(tr("Data Transfer"));

    dataShow = new QTextEdit();
    dataShow->document()->setMaximumBlockCount(300);
    dataShow->setReadOnly(true);

    QHBoxLayout *hbox = new QHBoxLayout;

    hbox->addWidget(dataShow);
    hbox->setContentsMargins(0,0,0,0);
    dataTransfering->setLayout(hbox);
    return dataTransfering;
}

void TStream::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange) {
        textDefs();
    } else
        QWidget::changeEvent(event);
}

void TStream::textDefs()
{
    pbClear->setText(tr("Очистка"));
    pbClear->setToolTip(tr("Очистка информационных табло"));
    bitRating->setTitle(tr("Отображение BER"));
    dataTransfering->setTitle(tr("Передача данных"));
}

void TStream::reflectData(QString title,  int* codes, int len)
{
    QString str,str1;
    for (int ii=0;ii<len;ii++) {
        str1 = QString(" %1").arg(*(codes+ii));
        str.append(str1);
    }
    str.prepend(title);
    dataShow->append(str);
}

void TStream::reflectMessage(QString title,  QList<int>* mess)
{
    QString str,str1;
    str.append(title+"\n");
    for (int ii=0;ii<mess->count();ii++) {
        quint8 cod = (int)(mess->at(ii));
        str1 = QString(" %1").arg(cod);
        str.append(str1);
    }
    dataShow->append(str);
}

void TStream::clearData()
{
    emit clearFromStreams();
    bitsShow->clear();
    dataShow->clear();
}

void TStream::showBer(Modem_stream* ber)
{
    QString str;
    bitsShow->clear();
    bitsShow->append(ber->vartext);
    str = QString("Отправлено \t%1 бит \t%2 байт" ).arg(ber->send).arg(ber->send/8);
    bitsShow->append(str);
    str = QString("Получено \t%1 бит \t%2 байт" ).arg(ber->recv).arg(ber->recv/8);
    bitsShow->append(str);
    str = QString("Ошибок \t%1 бит \t%2 байт" ).arg(ber->error).arg(ber->byte_error);
    bitsShow->append(str);
}

