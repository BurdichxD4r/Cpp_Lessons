#include "widgetrc.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGroupBox>

//--------------------------------------------------------------
widgetRC::widgetRC(QWidget *parent):
        QWidget(parent)
{
   initWidget();
   tabOrderSett();
}
//--------------------------------------------------------------
widgetRC::~widgetRC()
{
   delete m_rb_rx;
   delete m_rb_tx;
   delete m_le_name;
   delete m_le_ip;
   delete m_sp_tcp;
   delete m_sp_udp_1;
   delete m_sp_udp_2;
}
//--------------------------------------------------------------
void widgetRC::tabOrderSett()
{
    setTabOrder(this, m_rb_rx);
    setTabOrder(m_rb_rx, m_rb_tx);
    setTabOrder(m_rb_tx, m_le_ip);
    setTabOrder(m_le_ip, m_sp_tcp);
    setTabOrder(m_sp_tcp, m_sp_udp_1);
    setTabOrder(m_sp_udp_1, m_sp_udp_2);
}
//--------------------------------------------------------------
void widgetRC::initWidget()
{
    QFrame *mainFrame = new QFrame(this);
    QVBoxLayout *vBox = new QVBoxLayout(mainFrame);
    QGroupBox *grBox = new QGroupBox();

    grBox->setTitle(" Задание типа радиосредства и радионаправления: ");

    QHBoxLayout *hBox1 = new QHBoxLayout(grBox);

    m_rb_rx = new QRadioButton("приём. (RX)");
    m_rb_tx = new QRadioButton("перед. (TX)");
    m_le_name = new QLineEdit("");

    m_rb_rx->setChecked(true);
    hBox1->addWidget(m_rb_rx);
    hBox1->addWidget(m_le_name);
    hBox1->addWidget(m_rb_tx);

    QLabel *lb1 = new QLabel("IP-адрес радиосредства");
    QLabel *lb2 = new QLabel("Порт для TCP соед.");
    QLabel *lb3 = new QLabel("Порт 1 для UDP соед.");
    QLabel *lb4 = new QLabel("Порт 2 для UDP соед.");
    QList<QLabel*> lst_label;
    lst_label << lb1 << lb2 << lb3 <<lb4;

    m_le_ip = new QLineEdit();
    QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
    //QRegExp ipRegexp ("^" +ipRange +"\\." +ipRange +"\\." +ipRange +"\\." +ipRange +"$");
    //QRegExpValidator *ipValidator = new QRegExpValidator(ipRegexp, this);
    //m_le_ip->setValidator(ipValidator);
    m_le_ip->setText("1.1.1.1");
    m_sp_tcp = new QSpinBox();
    m_sp_tcp->setMaximum(1000000);
    m_sp_udp_1 = new QSpinBox();
    m_sp_udp_1->setMaximum(1000000);
    m_sp_udp_2 = new QSpinBox();
    m_sp_udp_2->setMaximum(1000000);

    QList<QWidget*> lst_widg;
    lst_widg << m_le_ip << m_sp_tcp << m_sp_udp_1 << m_sp_udp_2;

    QVector <QHBoxLayout *> vec_h_box;

    for (int i = 0; i < 4; ++i){
        QHBoxLayout *hBox_ = new QHBoxLayout();
        vec_h_box.append(hBox_);
    }

    for (int i = 0; i < 4; ++i){
       vec_h_box.at(i)->addWidget(lst_label.at(i));
       vec_h_box.at(i)->addWidget(lst_widg.at(i));
    }

    vBox->addWidget(grBox);

    for (int i =0; i < 4; ++i){
        vBox->addLayout(vec_h_box.at(i));
    }

    this->setLayout(vBox);

    connect(m_rb_rx, SIGNAL(clicked(bool)), this, SLOT(slotChangeRadioName()));
    connect(m_rb_tx, SIGNAL(clicked(bool)), this, SLOT(slotChangeRadioName()));
}
//----------------------------------------------------------------
void widgetRC::slotChangeRadioName()
{
    emit changeRadioName();
}
//-----------------------------------------------------------------
QStringList widgetRC::getRCfilds()
{
   QStringList res;
   res << m_sp_tcp->text()<< m_sp_udp_1->text()<< m_sp_udp_2->text();
   return res;
}
//----------------------------------------------------------------
int widgetRC::getRC1()
{
    return m_sp_tcp->value();
}
//----------------------------------------------------------------
int widgetRC::getRC2()
{
    return m_sp_udp_1->value();
}
//----------------------------------------------------------------
int widgetRC::getRC3()
{
    return m_sp_udp_2->value();
}
//----------------------------------------------------------------
QString widgetRC::getRCIP4()
{
   return m_le_ip->text();
}
//----------------------------------------------------------------
QString widgetRC::getRCName()
{
    QString name_dir = m_le_name->text();
    QString res{""};
    if(m_rb_rx->isChecked()){
       return res = name_dir+":RX";
    }
    else if (m_rb_tx->isChecked())
       return res = name_dir+":TX";
    else
         return "";
}
//---------------------------------------------------------------
