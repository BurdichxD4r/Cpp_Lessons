#ifndef TEXTCMD_H
#define TEXTCMD_H

#include <QObject>
#include <QWidget>
#include <QFocusEvent>
#include <QLabel>


class TextCMD: public QLabel
{
    Q_OBJECT
public:
    TextCMD(const QString &text, QColor cl_text, QWidget *parent = nullptr);


public slots:

private:    

     QColor m_text_cl;      // цвет текста
     QColor m_base_cl;      // основной цвет элемента
     QString m_mess;


protected:

     void   focusInEvent(QFocusEvent * event) override;
     void   focusOutEvent(QFocusEvent * event) override;

signals:
     void    sigWholeMess(QString &, QColor );
};

#endif // TEXTCMD_H
