#include "textmess.h"

//------------------------------------------------------------------
TextMess::TextMess(QWidget * parent)
     : QTextEdit(parent)
 {
    setReadOnly(true);
   // initTextEdit();
 }
//------------------------------------------------------------------
void TextMess::slotShowMess(QString &mess, QColor cl)
{
    setTextColor(cl);
    setText(mess);
}
//-----------------------------------------------------------------
/*void TextMess::initTextEdit()
{

}
*/
//--------------------------------------------------------------------
