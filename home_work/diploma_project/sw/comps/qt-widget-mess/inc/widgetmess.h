#ifndef WIDGETMESS_H
#define WIDGETMESS_H

#include <QObject>
#include <QWidget>
#include <QTime>
#include <QScrollBar>
#include <QCheckBox>
#include <QDoubleSpinBox>

#include "tablemess.h"     // журнал
#include "textmess.h"      // содержание команды


class WidgetMess: public QWidget
{
    Q_OBJECT
public:
    WidgetMess(QWidget *parent = 0);
    ~WidgetMess();
    void    initWidget();

public slots:
    void    slotAddMsg(   int     tx,               // флаг вход/исход сообщ.
                            QDateTime   tm,         // время прихода
                            QString &type_com,
                      const QString &mess,          // содержание сообщения
                      const QColor  &cl);           // цвет шрифта

    void    slotLookUpJournal(int action);                   // просмотр журнала
    void    slotLookUpJournalTo(int position);
    void    slotScrollBarSynchro(int row);    

private:

  TableMess      *m_table;           // журнал сообщений (таблица)
  TextMess       *m_text_mess;       // расшифровка сообщения (текст. поле)
  QScrollBar     *m_scroll;          // скролл для просмотра журнала
 // QCheckBox      *m_chB_format;    // чек. бокс для задания формата сообщения
//  QCheckBox      *m_chB_visible;   // чек. бокс для задания видимости компонента
  QCheckBox      *m_chB_logsave;     // чек.бокс для сохранения лог. файлов журнала
  QDoubleSpinBox *m_sp_logsize;      // размер ед. лог. файла (Мбайт)

protected: 

signals:
  void sigAddNewMsg(int, QDateTime, QString &, const QString &, const QColor  &);  // сигнал для TableMess,о добавлении новой команды в журнал
  void sigJournalLookUp(int action);  // просмотр журнала
  void sigJournalLookUpTo(int position);  // просмотр журнала
  void sigSaveLog(bool save);             // сохранять или нет лог файлы сеанса  
};

#endif // WIDGETMESS_H
