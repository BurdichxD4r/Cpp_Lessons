#include "dte_label.h"
#include <QVBoxLayout>
//----------------------------------------------------------------------------
DTE_label::DTE_label(QString port, QPixmap &pic, QString addr, QWidget *parent)
{
   Q_UNUSED(parent);

   m_port = new QLabel("port: " + port);
   m_addr = new QLabel("adrs: " + addr);
   m_pic = new QLabel();
   m_pic->setPixmap(pic);  

   QVBoxLayout * vbox = new QVBoxLayout();

   QPalette pl;

   pl.setBrush(QPalette::Active,QPalette::WindowText, QColor(Qt::blue));
   pl.setBrush(QPalette::Inactive,QPalette::WindowText, QColor(Qt::blue));
   pl.setBrush(QPalette::Disabled,QPalette::WindowText, QColor(Qt::blue));

   m_port->setFixedHeight(14);
   m_port->setPalette(pl);
   m_addr->setFixedHeight(14);
   m_addr->setPalette(pl);
   m_pic->setBuddy(m_port);
   m_pic->setBuddy(m_addr);

   vbox->addStretch();
   vbox->addWidget(m_port);
   vbox->addWidget(m_pic);
   vbox->addWidget(m_addr);
   vbox->addStretch();

   this->setLayout(vbox);
   //  this->setMaximumHeight(55);
   //   this->setFixedHeight(55);

   // this->setMaximumWidth(55);
   // this->setFixedWidth(55);
}
//-----------------------------------------------------------------------------
