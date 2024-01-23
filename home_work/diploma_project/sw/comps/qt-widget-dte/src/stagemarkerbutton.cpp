#include "stagemarkerbutton.h"

//---------------------------------------------------------------------------------
StageMarkerButton::StageMarkerButton(MarkTypeEnum mark_type, QString title,
                                     QWidget *parent)
                  :QLabel(parent), m_mark_type(mark_type)
{
   this->setText(title);      // устанавливаем заголовок
                       // переводим кнопку в выкл. состояние
   this->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
   this->setFixedHeight(60);
   this->setFixedWidth(60);
  // int sz = parent->width()/9 ;
 //  this->setFixedWidth(sz);
   this->setWordWrap(true);
   this->setFrameShape(Box);
   this->setFrameShadow(Raised);
   this->setAutoFillBackground(true);
   this->setAlignment((Qt::AlignHCenter | Qt::AlignVCenter));

   setOff();
   }
//-----------------------------------------------------------------------------------
void    StageMarkerButton::setColor(QColor cl_t, QColor cl_b)        // установка цвета текста
{
    m_color_text = cl_t;
    m_color_back = cl_b;
    QPalette pl;
    pl.setBrush(QPalette::Active, QPalette::WindowText, QColor(m_color_text));
    pl.setBrush(QPalette::Disabled, QPalette::WindowText, QColor(m_color_text));
    pl.setBrush(QPalette::Inactive, QPalette::WindowText, QColor(m_color_text));

    pl.setBrush(QPalette::Active,QPalette::Window, QColor(m_color_back));
    pl.setBrush(QPalette::Disabled, QPalette::Window, QColor(m_color_back));
    pl.setBrush(QPalette::Inactive, QPalette::Window, QColor(m_color_back));
    this->setPalette(pl);
   }

//-----------------------------------------------------------------------------------
void    StageMarkerButton::setOn(int bt_width)      // вкл.
{
    QColor cl_ = QColor::fromRgb(0xff, 0xb1, 0x9d);
   // m_on = true;
    switch(m_mark_type){
    case  WARING : setColor(Qt::darkRed, Qt::yellow);
        break;
    case  DAMAGE : setColor(Qt::darkRed, cl_);
         break;
    case  CORRECT : setColor(Qt::darkGreen, Qt::green);
        break;
    case  INFORM : setColor(Qt::darkBlue, Qt::cyan);
        break;
    default:
        break;
    }   
   // this->setFixedWidth(bt_width);
    this->setMaximumWidth(bt_width);

}
//-----------------------------------------------------------------------------------
void    StageMarkerButton::setOff()      // выкл.
{
    int S = 0xdc; //

    QColor cl_ = QColor::fromRgb(S, S, S);

    setColor(Qt::darkGray, cl_);
    this->setFixedWidth(0);
    this->setMaximumWidth(0);
}

//-----------------------------------------------------------------------------------
void StageMarkerButton::setTitle(QString title)
{
    setText(title);
}
//-----------------------------------------------------------------------------------
QString StageMarkerButton::getTitle()
{
    return text();
}
//-----------------------------------------------------------------------------------
