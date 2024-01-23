#include "textcmd.h"

//------------------------------------------------------------------
TextCMD::TextCMD(const QString &text, QColor cl_text, QWidget *parent)
     : /*QTextEdit*/QLabel(text, parent), m_text_cl(cl_text)
 {
     m_mess = text;  // сообщение целиком а не только команда   
     setFocusPolicy(Qt::StrongFocus);

     QPalette pl;
     pl.setColor(QPalette::Text, m_text_cl);
     setPalette(pl);

     setFixedHeight(this->fontInfo().pixelSize() + 4);
     setAlignment(Qt::AlignLeft);
 }

//-----------------------------------------------------------------
void TextCMD::focusInEvent(QFocusEvent *event)
{
     QPalette pl;

     if(event->gotFocus()){        
         pl.setBrush(QPalette::Text, m_text_cl);
         QFont fn;
         fn.setBold(true);
         this->setFont(fn);
         this->setPalette(pl);
         emit sigWholeMess(m_mess, m_text_cl);
     }
}

//-----------------------------------------------------------------
void TextCMD::focusOutEvent(QFocusEvent *event)
{
     QPalette pl;

     if(event->lostFocus()){
         pl.setBrush(QPalette::Text, m_text_cl);
         QFont fn;
         fn.setBold(false);
         this->setFont(fn);
         this->setPalette(pl);
     }
}

//--------------------------------------------------------------------
