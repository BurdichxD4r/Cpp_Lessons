#ifndef TSTREAM_H
#define TSTREAM_H

#include <QWidget>
#include "slot_params.h"

QT_BEGIN_NAMESPACE
class QCheckBox;
class QGroupBox;
class QPushButton;
class QComboBox;
class QTextEdit;
class QString;
class QProgressBar;
QT_END_NAMESPACE

class TStream : public QWidget
{
    Q_OBJECT

public:
    TStream(QWidget *parent = nullptr);
    ~TStream();

private:

    QPushButton *pbClear; // очистка статистики
    QTextEdit *bitsShow;
    QTextEdit *dataShow;

    QGroupBox *dataTransfering;
    QGroupBox *bitRating;

    QGroupBox *createBitsTransfer();
    QGroupBox *createDataTransfer();
    void textDefs();

private slots:

public slots:
    void reflectData(QString title,  int* codes, int len);
    void reflectMessage(QString title,  QList<int> *mess);
    void clearData();
    void showBer(Modem_stream* ber);


signals:
    void clearFromStreams();

protected:
    void changeEvent(QEvent *event);

};

#endif // TSTREAM_H
