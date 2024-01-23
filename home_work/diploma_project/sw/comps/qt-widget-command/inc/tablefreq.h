#ifndef TABLEFREQ_H
#define TABLEFREQ_H

#include <QObject>
#include <QWidget>
#include <QItemDelegate>
#include <QTableWidget>
#include <QLineEdit>
#include <QSpinBox>

// можно вводить только числа от 1 до 9999999999
// Использование делегата QLineEdit и регулярного выражения для ограничения ввода
class FreqListDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    FreqListDelegate(QObject *parent = 0): QItemDelegate(parent) { }
    //-------------------------------------------------------------------
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option,
        const QModelIndex &index) const
    {
        Q_UNUSED (option);
        Q_UNUSED (index);
        //QSpinBox *editor = new QSpinBox(parent);
        QLineEdit *editor = new QLineEdit(parent);
        //QRegExp regExp("[1-9][0-9]{0,9}");
        QPalette pl;
        editor->setAutoFillBackground(true);
        pl.setBrush(QPalette::Text, QColor(Qt::white));
        pl.setBrush(QPalette::Window, QColor(Qt::blue));
        pl.setBrush(QPalette::Base, QColor("#0089cd"));      //("#678db2"));
        pl.setBrush(QPalette::Highlight, QColor("#0089cd")); //("#678db2"));
        pl.setBrush(QPalette::HighlightedText, QColor(Qt::white));
        editor->setPalette(pl);
        editor->setBackgroundRole(QPalette::Base);

       // editor->setValidator(new QRegExpValidator(regExp, parent));
        return editor;
    }
    //-------------------------------------------------------------------
    void setEditorData(QWidget *editor, const QModelIndex &index) const
    {
        QString text = index.model()->data(index, Qt::EditRole).toString();
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
       // QSpinBox *ed = static_cast <QSpinBox*>(editor);
       // ed->setValue(text.toInt());
        lineEdit->setText(text);
        emit changeData();
    }
    //-------------------------------------------------------------------
    void setModelData(QWidget *editor, QAbstractItemModel *model,
        const QModelIndex &index) const
    {
        QLineEdit *lineEdit = static_cast<QLineEdit*>(editor);
        QString text = lineEdit->text();
        model->setData(index, text, Qt::EditRole);
    }
   //-------------------------------------------------------------------
    void updateEditorGeometry(QWidget *editor,
        const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
       Q_UNUSED (index);
        editor->setGeometry(option.rect);

    }
signals:
  void changeData() const;     // изменилась запись в одной из строки списка частот
};

//-----------------------------------------------------------------------------------------------
class TableFreq :public QTableWidget
{
    Q_OBJECT
public:
    TableFreq(QWidget *parent = nullptr);

    int  getFreqCount();                     // запрос на количество частот в таблице
public slots:

    void slotAddFreq();                     // доб. нового элемента в список частот
    void slotDelFreq();                     // удал. элемента в список частот
    void slotChangeData();
private:   


signals:
    void freqsChange();                     // изменение списка частот

};

#endif // TABLEFREQ_H
