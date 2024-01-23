//-----------------------------------------------------------------------
// Компонент ColorGrid используется в качестве визуализации панели-таблицы,
// ячейки которой могут отображать состояние активности (check-box) и иметь
// заданную окраску  в диапазоне (синий - красный - зеленый).
// Цвет ячейки может быть передан компоненту извне, используя слот :
//  void   colorChngSlot(int,  const int);
// активность выбранной ячейки можно изменять используя слот:
// void   checkChngSlot(int);


#ifndef COLORGRID_H
#define COLORGRID_H

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QVector>
#include <QMessageBox>
#include <QKeyEvent>
#include <QHeaderView>
#include <math.h>


const int MAIN_COLOR_CELL = 50;   // основной цвет определяем как зелёный

const QColor COLOR_TEXT_CELL_LT = Qt::yellow;   // цвет  ячеек (-50 - 0)
const QColor COLOR_TEXT_CELL_DC = Qt::blue;     // цвет  ячеек (0 - 50)

const int NUM_COLOR_POINTS = 50;   // число цветовых точек (оттенков) в градиенте

//const int MAIN_COLOR_CELL_2 = 10;
//const int ANOTHER_COLOR_CELL = 0;
//const int ANOTHER_COLOR_CELL_2 = -50;

const int CELL_AVAIL_PARAM = 96; // параметр доступности ячейки


class ColorGrid : public QTableWidget
{
    Q_OBJECT

public:
    ColorGrid(QWidget *parent = 0, int row =16, int col = 16);
             // функции доступа к папаметрам (цвет, активность) ячейки табл.
    void   setColorCell(int num, int color);
    void   setCheckCell(int num, bool ch);
    int    getColorCell(int num);
    bool   getCheckedCell(int num);
    void   setMinWidthCell(int min_width);
    void   setMinHightCell(int min_hight);

private:
    QVector <bool>   m_activCells;    // параметр активности
    QVector <int>    m_availCells;    // параметр доступности
    QVector <int>    m_colorCells;    // параметр цвета
    int              m_row;           // число рядов в табл.
    int              m_col;           // число колонок в табл.
    QColor           m_curr_color;    // основной цвет элемента над которым находится мышь
    int              m_min_width_cell{10}; // мин размер ячейки
    int              m_min_hight_cell{5}; // мин размер ячейки

    void   initGrid();                // настройка внешнего вида табл. Вызывается при создании
    QColor intToColor(int color_int);           // функция преобразоваиния цвета (целое число -> тип QColor)
    QColor intToColorEx(int color_int);         // усложненная функция преобразования цветов
    void   itemSetting(QTableWidgetItem & item, int numCell, int clCell);

    double normalMix(int mix_int);     // нормировка искомого оттенка к интервалу [0,1]
    double linerInterpol(double y1, double y2, double x );  // линейная интерполяция
    QColor inversRGBComp(QColor cl);    // обратное RGB преобразование
    QColor strightRGBComp(QColor cl);   // прямое RGB преобразование
    double brightCalc(QColor cl);       // вычисление яркости
    void   changeFontSize();            // изменение размера шрифта в зависимости от размера ячейки панели

public slots:
    void   colorChngSlot(int,  const int); // слот изменения  цвета ячейки с переданным номером
    void   checkChngSlot(int);             // слот изменения состояния активности ячейки с переданным номером
    void   enterCellSlot(int, int);
    void   pressCellSlot();
    void   clickCellSlot(int, int);        // слот связанный с сигналом - клик мышью по ячейки виджета
    void   availCellDescSlot(int);         // обработка события уменьшения параметра доступности ячейки с текущим номером
    void   reFreshAvailCellSlot(int);      // обработка события обращение к ячейки с номером извне
                                           // (обнавляет парамерт доступности до CELL_AVAIL_PARAM)

protected:
    void   keyPressEvent(QKeyEvent * e) override;
    void   resizeEvent(QResizeEvent *e) override; // добавить в исходный код 23.01.22

signals:
    void   numberCellSig(int);
    void   colorCellSig(int);
};

#endif // COLORGRID_H
