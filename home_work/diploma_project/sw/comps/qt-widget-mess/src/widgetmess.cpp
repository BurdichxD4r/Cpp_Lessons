#include "widgetmess.h"
#include <QGridLayout>
#include <QSplitter>
#include <QLabel>

//------------------------------------------------------------------
WidgetMess::WidgetMess(QWidget * parent)
     : QWidget(parent)
 {
    initWidget();
    m_sp_logsize->setValue(0.05);
    m_chB_logsave->setChecked(false);
 }
//-----------------------------------------------------------------
WidgetMess:: ~WidgetMess()
{
     delete m_table;
}
//-----------------------------------------------------------------
void WidgetMess::initWidget()
{
    m_table = new TableMess();
    m_text_mess = new TextMess();
    m_scroll  = new QScrollBar(Qt::Vertical);

    //m_chB_format = new QCheckBox("Форматировать сообщения");
    //m_chB_visible = new QCheckBox("Видимость журнала");
    m_chB_logsave = new QCheckBox("Сохранить лог. файл");
    QLabel *label = new QLabel("Размер лог. файла: (Мб)");
    m_sp_logsize = new QDoubleSpinBox();

  //  m_scroll->setSliderDown(true);   
    QSplitter *spl = new QSplitter(Qt::Vertical);

    QVBoxLayout * vvbox = new QVBoxLayout();
    QHBoxLayout * hbox = new QHBoxLayout();
    QHBoxLayout * hhbox = new QHBoxLayout();
    QVBoxLayout * vbox = new QVBoxLayout();

    spl->addWidget(m_table);
    spl->addWidget(m_text_mess); 

   // hbox1->addWidget(m_chB_format);
   // hbox1->addWidget(m_chB_visible);
    vbox->addWidget(spl);
    hhbox->addWidget(label);
    hhbox->addWidget(m_sp_logsize);

    vvbox->addLayout(vbox);
    vvbox->addWidget(m_chB_logsave);
    vvbox->addLayout(hhbox);

    //hbox->addWidget(m_scroll);
    hbox->addLayout(vvbox);
    this->setLayout(hbox);
    m_chB_logsave->setEnabled(false);

    connect(this, &WidgetMess::sigAddNewMsg, m_table, &TableMess::slotAddMsg);
    connect (m_table, &TableMess::sigWholeMessToRead, m_text_mess, &TextMess::slotShowMess);
    connect(m_table, &TableMess::sigActiveRow, this, &WidgetMess::slotScrollBarSynchro);
    connect(m_scroll, &QScrollBar::sliderMoved, this, &WidgetMess::slotLookUpJournalTo);
    connect(m_scroll, &QScrollBar::actionTriggered, this, &WidgetMess::slotLookUpJournal);    
    connect(m_sp_logsize,  SIGNAL(valueChanged(double)), m_table, SLOT(slotSizeLog(double)));

    connect(this, &WidgetMess::sigJournalLookUp, m_table, &TableMess::slotLookUpJounal);
    connect(this, &WidgetMess::sigJournalLookUpTo, m_table, &TableMess::slotLookUpJounalTo);    
    connect(m_chB_logsave,  &QCheckBox::clicked, m_table, &TableMess::slotSetSaveParam);
}
//------------------------------------------------------------------
void WidgetMess::slotAddMsg(int tx,  QDateTime   tm, QString &type_com,
                             const QString &mess, const QColor  &cl)
{
   if(mess.count() > 5)  //                        
     emit sigAddNewMsg(tx, tm,type_com, mess, cl);
}
//--------------------------------------------------------------------
void WidgetMess::slotLookUpJournal(int action)
{
 int n = m_table->getVecSize();
 m_scroll->setMaximum(n);
 m_scroll->setSingleStep(1);
 emit sigJournalLookUp(action);
}
//--------------------------------------------------------------------
void WidgetMess::slotLookUpJournalTo(int position)
{
 int n = m_table->getVecSize();
 m_scroll->setMaximum(n);
 m_scroll->setValue(position);
 emit sigJournalLookUpTo(position);
}
//--------------------------------------------------------------------
void WidgetMess::slotScrollBarSynchro(int row)
{
   m_scroll->setValue(row);
}
//----------------------------------------------------------------------
