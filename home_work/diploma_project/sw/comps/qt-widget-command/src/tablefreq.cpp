#include "tablefreq.h"
#include <QMenu>
#include <QHeaderView>
#include <QSpinBox>

//--------------------------------------------------------------------------------
TableFreq::TableFreq(QWidget *parent)
{  
    Q_UNUSED (parent);
    this->setRowCount(0);
    this->setColumnCount(2);
    this->setSelectionMode(QAbstractItemView::SingleSelection);
    this->setSelectionBehavior(QAbstractItemView::SelectRows);
    this->setTabKeyNavigation(true);
}
//---------------------------------------------------------------------------------
void TableFreq::slotChangeData()
{
    emit freqsChange();
}
//--------------------------------------------------------------------------------
void TableFreq::slotAddFreq()
{
   int n = this->rowCount();

   this->setRowCount(n+1);

   FreqListDelegate *freqListDelegate = new FreqListDelegate(this);   
   this->setItemDelegateForRow(n,freqListDelegate);

   connect(freqListDelegate, SIGNAL(changeData()), this, SLOT(slotChangeData()));

   QTableWidgetItem *twi = new QTableWidgetItem();

   this->setItem(n,0,twi);

   emit freqsChange();      // список частот возможно изменился
}

//--------------------------------------------------------------------------------------------
void TableFreq::slotDelFreq()
{
    int n = this->currentRow();

    this->removeRow(n);

    n = this->rowCount();
    for (int i = 0; i < n; ++i){
       QTableWidgetItem *item = new QTableWidgetItem();
       item->setText(QString::number(i));
       this->horizontalHeader()->setStyleSheet("QHeaderView::section { background-color: lightGray}");
       this->setVerticalHeaderItem(i,item);
    }
    //slotUseFreqList(true);
    emit freqsChange();      // список частот возможно изменился
}
//-----------------------------------------------------------------------------------------------
int TableFreq::getFreqCount()
{
   int num{0};
    for (int i = 0; i < this->rowCount(); ++i){
        QString str = this->item(i,0)->text();
        if(str != "") ++num;
    }
    if (num == 0) num = 1;    // если в списке частот нет ни одной записи, тогда num=1;
    return num;
}
//-----------------------------------------------------------------------------------------------

