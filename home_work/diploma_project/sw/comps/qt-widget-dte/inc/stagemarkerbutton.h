#ifndef STAGEMARKERBUTTON_H
#define STAGEMARKERBUTTON_H

#include <QObject>
#include <QWidget>
#include <QLabel>


enum MarkTypeEnum{ // тип метки для задания состояния в котором находится объект УВС в данный момент
    WARING = 0,   // предупреждение
    DAMAGE,       // опасность
    CORRECT,      // штатная работа
    INFORM        // информация
};

class StageMarkerButton: public QLabel
{    
    Q_OBJECT
public:
    StageMarkerButton( MarkTypeEnum mark_type, QString title = "", QWidget *parent = nullptr);

    void    setOn(int bt_hight);       // вкл.
    void    setOff();      // выкл.
    void    setTitle(QString title);  // изменение записи не кнопке
    QString getTitle();        // получение значения надписи кнопки состояния

private:

    MarkTypeEnum    m_mark_type;                   // тип метки
    QColor          m_color_text{Qt::darkGray};    // цвет текста
    QColor          m_color_back{Qt::lightGray};   // цвет фона
                       // включена кнока или нет

    void    setColor(QColor cl_t,QColor cl_b);        // установка цвета текста и фона

};

#endif // STAGEMARKERBUTTON_H
