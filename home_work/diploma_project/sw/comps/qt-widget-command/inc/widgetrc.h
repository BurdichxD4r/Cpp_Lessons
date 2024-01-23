#ifndef WIDGETRC_H
#define WIDGETRC_H

#include <QWidget>
#include <QGroupBox>
#include <QRadioButton>
#include <QLineEdit>
#include <QLabel>
#include <QSpinBox>

class widgetRC :public QWidget
{
    Q_OBJECT
public:
    explicit widgetRC(QWidget *parent = nullptr);
    ~widgetRC();

   QStringList getRCfilds(); // геттер полей радиосредства
   QString     getRCIP4();   // геттер ip4
   QString     getRCName();
   int         getRC1();
   int         getRC2();
   int         getRC3();

public:
   QRadioButton *m_rb_rx;   // тип радио средства (приёмник)
   QRadioButton *m_rb_tx;   // тип радио средства (передатчик)
   QLineEdit    *m_le_name; // доп. часть имени рс( для радио направления)

   QLineEdit    *m_le_ip;     // ip-адресс радиосредства
   QSpinBox     *m_sp_tcp;    // tcp порт радиосредства
   QSpinBox     *m_sp_udp_1;  // udp 1 порт
   QSpinBox     *m_sp_udp_2;  // udp 2 порт

private slots:
   void     slotChangeRadioName();

private:
   void     initWidget();          // инициализация виджета радиосредсва
   void     tabOrderSett();

signals:
   void     changeRadioName();      // сигнал об изменении радиоИмени
};

#endif // WIDGETRC_H
