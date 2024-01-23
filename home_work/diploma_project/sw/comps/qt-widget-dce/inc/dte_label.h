#ifndef DTE_LABEL_H
#define DTE_LABEL_H

#include <QObject>
#include <QWidget>
#include <QLabel>

class DTE_label: public QWidget
{
    Q_OBJECT

    QLabel *m_port;
    QLabel *m_addr;
    QLabel *m_pic;
public:
    DTE_label(QString port, QPixmap &pic, QString addr, QWidget *parent = nullptr);

};

#endif // DTE_LABEL_H
