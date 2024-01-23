#ifndef TEXTMESS_H
#define TEXTMESS_H

#include <QObject>
#include <QWidget>
#include <QTextEdit>


class TextMess: public QTextEdit
{
    Q_OBJECT
public:
    TextMess(QWidget *parent = nullptr);    

public slots:
    void slotShowMess(QString &mess, QColor cl); 

};

#endif // TEXTMESS_H
