#include "colorgrid.h"

//-------------------------------------------------------------------------
ColorGrid::ColorGrid(QWidget * parent, int row_count, int col_count)
    : QTableWidget(parent), m_row(row_count), m_col(col_count)
{

  initGrid();

  connect(this, &ColorGrid::cellClicked, this, &ColorGrid::clickCellSlot);
  connect(this, &ColorGrid::cellEntered, this, &ColorGrid::enterCellSlot);
  connect(this, &ColorGrid::cellPressed, this, &ColorGrid::pressCellSlot);

}
//--------------------------------------------------------------------------
void ColorGrid::enterCellSlot(int row, int col)
{
    int num  = row * m_col + col;
    m_curr_color = intToColorEx(m_colorCells.at(num));

 } 
//--------------------------------------------------------------------------
void ColorGrid::pressCellSlot()
{
   QPalette pl;
   pl.setColor(QPalette::Highlight, m_curr_color);
   pl.setColor(QPalette::Text, COLOR_TEXT_CELL_LT);
   setPalette(pl);

}
//--------------------------------------------------------------------------
void ColorGrid::clickCellSlot(int row, int col)
{
   int num  = row * m_col + col;

   emit numberCellSig(num);
   emit colorCellSig(m_colorCells.at(num));
}
//--------------------------------------------------------------------------
void ColorGrid::itemSetting(QTableWidgetItem & item, int numCell, int clCell)
{
    QBrush br = intToColorEx(clCell);
    item.setBackground(br.color());

    if (clCell < 0 ) item.setForeground(COLOR_TEXT_CELL_LT);
    else if (clCell >= 0) item.setForeground(COLOR_TEXT_CELL_DC);

    if(m_activCells.at(numCell)){
       QFont qf;
       qf.setFamily(qf.defaultFamily());   // устанавливаем шрифт по умолчанию

       item.setText(QString::number(numCell));
       item.setTextAlignment(Qt::AlignCenter);
       item.setFont(qf);

       changeFontSize();  // корректируем размер шрифта в разисимости от размера ячейки

      }
}
//--------------------------------------------------------------------------
void ColorGrid::changeFontSize()
{
  // получаем первичные настройки
  const QString &text = "12345";

  QFont ftCell = this->font();        // текущий шрифт ячейки
  int size = ftCell.pointSize();      // размер текущего шрифта
  QFontMetrics fontMetrics(ftCell);
  QSize szAr = fontMetrics.size(Qt::TextSingleLine, text);  // размер надписи текущим шрифтом

  // решаем уменьшаем или увеличиваем размер шрифта
  int step = ((szAr.height() > this->rowHeight(1)) || (szAr.width() > this->columnWidth(1)) ) ? -1 : 1;
  // итерируем пока текст не будет соответствовать размеру ячейки
  for (;;)
   {
    ftCell.setPointSize(size + step);

    QFontMetrics fontMetrics(ftCell);
    szAr = fontMetrics.size(Qt::TextSingleLine, text);
    if (size <= 1)
    {
    //  cout << "Font cannot be made smaller!" << endl;
      break;
       }
    if (step < 0) {  // если шаг выбран -1
      size += step;
          // если надпись укладывается в ячейку
      if ((szAr.height() < this->rowHeight(1)) || (szAr.width() < this->columnWidth(1)))
          break;
    }
    else {       // если шаг выбран 1
      if ((szAr.height() > this->rowHeight(1)) || (szAr.width() > this->columnWidth(1)))
          break;
      size += step;
    }
  }
  // применяем результат полученный в очередной итерации
  ftCell.setPointSize(size);

  setFont(ftCell);   // устанавливаем полученный шрифт для всего объекта

}
//--------------------------------------------------------------------------
void ColorGrid::colorChngSlot(int num, const int color)
{
   // пересчет сквозной нумерации ячеек к двумерной (ряд, колонка)
    int row = num / m_col;
    int col = num % m_col;
    int num_all = m_col * m_row;      

    if(num <= num_all){
        if(color > MAIN_COLOR_CELL || color < -MAIN_COLOR_CELL)
            QMessageBox::warning(this, "Ошибка"," Переданное значение цвета не соответствует заданной шкале");
        else{
             m_colorCells.replace(num, color);   // меняем значение находящееся в массиве цветов на переданное значение
             // отображаем переданные изменения в виджете
             QTableWidgetItem *item = new QTableWidgetItem(*(takeItem(row, col)));
             itemSetting(*item, num, color);// m_colorCells.at(num));
             setItem(row, col, item);
        }
    }
    else
       QMessageBox::warning(this, "Ошибка"," Переданные значения больше размеров таблицы");
}
//--------------------------------------------------------------------------
void ColorGrid::checkChngSlot(int num)
{
    int row = num / m_col;
    int col = num % m_col;

    bool check(false);

    if(num <= m_col * m_row){           
       check = m_activCells.at(num);
       if(!check) // если ячейка не была в активном состоянии
           m_availCells.replace(num, CELL_AVAIL_PARAM);
       else
           m_availCells.replace(num, -1);
       m_activCells.replace(num, !check);    

       QTableWidgetItem *item = new QTableWidgetItem(*(takeItem(row, col)));
       QTableWidgetItem *item2 = new QTableWidgetItem();

       itemSetting(*item2, num, m_colorCells.at(num));
       setItem(row, col, item2);
       delete item;
      }
    else
         QMessageBox::warning(this, "Ошибка"," Переданное значение больше размеров таблицы");
}
//------------------------------------------------------------------------------------------
// обработка события уменьшения параметра доступности ячейки с текущим номером
void   ColorGrid::availCellDescSlot(int num)
{
   int avail(0);
   bool check(false);

   if(num <= m_col * m_row){
       avail = m_availCells.at(num);
       check = m_activCells.at(num);
       if(avail > 0){
           --avail;
          m_availCells.replace(num, avail);
          if(avail == 0){    // если параметр доступа у ячейки с num стал = 0 , мы ее отключаем (меняем активность)
              if(check)
               checkChngSlot(num);
           }
       }
      }
    else
         QMessageBox::warning(this, "Ошибка"," Переданное значение больше размеров таблицы");
}
//------------------------------------------------------------------------------------------
// обработка события обращение к ячейки с номером извне (обнавляет парамерт доступности до CELL_AVAIL_PARAM)
void   ColorGrid::reFreshAvailCellSlot(int num)
{
    int row = num / m_col;
    int col = num % m_col;

    bool check(false);
    int avail(CELL_AVAIL_PARAM);

    if(num <= m_col * m_row){
       m_availCells.replace(num, avail);  // обновняем значение параметра доступа ячейки к которой пришло обращение
       check = m_activCells.at(num);
       if(!check) {// если на момент обращения активность ячейки = false
          m_activCells.replace(num, !check);     // меняем ее
          QTableWidgetItem *item = new QTableWidgetItem(*(takeItem(row, col)));
          QTableWidgetItem *item2 = new QTableWidgetItem();
          itemSetting(*item2, num, m_colorCells.at(num));
          setItem(row, col, item2);
          delete item;
       }
       // иначе оставляем все как есть (ячейку с активностью = true)
      }
    else
         QMessageBox::warning(this, "Ошибка"," Переданное значение больше размеров таблицы");
}
//-------------------------------------------------------------------------
void   ColorGrid::setMinWidthCell(int min_width)
{
    m_min_width_cell = min_width;
}
//-------------------------------------------------------------------------
void   ColorGrid::setMinHightCell(int min_hight)
{
    m_min_hight_cell = min_hight;
}
//-------------------------------------------------------------------------
void ColorGrid::initGrid()
{
    setColumnCount(m_col);
    setRowCount(m_row);

    QStringList vHeader, hHeader;    

    // инициализация основных параметров табл. - массива цветов, массива активности каждой ячейки
    for(int k = 0; k < m_row * m_col; ++k){
       m_activCells.append(false);
       m_availCells.append(-1);
       m_colorCells.append(MAIN_COLOR_CELL);
    }
      setFocusPolicy(Qt::StrongFocus);
  //  setFocusPolicy(Qt::NoFocus);

    QHeaderView *hhd = new QHeaderView(Qt::Horizontal);
    QHeaderView *hvd = new QHeaderView(Qt::Vertical);

    QFont fn;
    fn.setFamily(fn.defaultFamily());    
    hhd->setMinimumSectionSize(m_min_width_cell);
    hhd->setFont(fn);
    hvd->setMinimumSectionSize(m_min_hight_cell);
    hvd->setFont(fn);
    setHorizontalHeader(hhd);
    setVerticalHeader(hvd);

    m_curr_color = MAIN_COLOR_CELL; 
                   // составляем шапку (верт., гориз. таблицы)
    for (int i = 0; i < m_row; ++i)
       //hHeader << QString::number(i);
        hHeader << QString::number(i * m_col);

    for(int j = 0; j < m_col; ++j)
      // vHeader << QString(QChar(j + 65));  // 65 = 'A'
        vHeader << QString::number(j);    

    setVerticalHeaderLabels(hHeader);
    setHorizontalHeaderLabels(vHeader);
    setSelectionMode(QAbstractItemView::NoSelection);
    //setSelectionMode(QAbstractItemView::SingleSelection);
    EditTriggers trigers(QAbstractItemView::NoEditTriggers);
    setEditTriggers(trigers);                           //  значение выбранной ячейки не редактируется
    setTabKeyNavigation(true);                          // перемещение по ячейкам табл. - возможно с пом. клавиатуры

    int number = 0;
    for (int i = 0; i < m_row; ++i){
       for (int j = 0; j < m_col; ++j)
         {
           QTableWidgetItem *item = new QTableWidgetItem();
           itemSetting(*item, number, MAIN_COLOR_CELL);
           setItem(i, j, item);
           ++number;
          }
    }
    setMouseTracking(true);         // отслеживание движение мыши по виджету

    changeFontSize();  // настройка размера шрифта

}
//--------------------------------------------------------------------------
void   ColorGrid::setColorCell(int num, int color)
{
    if(num <= m_row * m_col){
        if (color > 50 || color < -50)
            QMessageBox::warning(this, "Ошибка"," Переданное значение цвета не соответствует заданной шкале");
        else
            m_colorCells.replace(num, color);
    }
    else
        QMessageBox::warning(this, "Ошибка"," Переданные значения выходят за допустимый диапазон");
}
//--------------------------------------------------------------------------
void   ColorGrid::setCheckCell(int num, bool ch )
{
   if(num <= m_row * m_col ){
      m_activCells.replace(num, ch);
   }
   else
       QMessageBox::warning(this, "Ошибка"," Переданное значение больше размеров таблицы");
}
//--------------------------------------------------------------------------
int ColorGrid::getColorCell(int num)
{
    if(num <= m_row * m_col){
        return m_colorCells.at(num);
    }
    else
        QMessageBox::warning(this, "Ошибка"," Переданное значение больше размеров таблицы");
    return false;
  }
//--------------------------------------------------------------------------
bool   ColorGrid::getCheckedCell(int num)
//int   ColorGrid::getCheckedCell(int num)
{
    if(num <= m_row * m_col){
       return m_activCells.at(num);
    }
    else
        QMessageBox::warning(this, "Ошибка"," Переданное значение больше размеров таблицы");
    return false;
}
//--------------------------------------------------------------------------
double ColorGrid::normalMix(int mix_int)
{
    double res = 0.0;
    if (mix_int < 0)
       res =( mix_int + MAIN_COLOR_CELL)/ (1.0 * NUM_COLOR_POINTS);
    else if (mix_int >= 0)
        res = mix_int / (1.0 * NUM_COLOR_POINTS);
    return  res;
}
//--------------------------------------------------------------------------
QColor ColorGrid::inversRGBComp(QColor cl)
{
    //Convert color from 0..255 to 0..1
    double r = cl.redF();
    double g = cl.greenF();
    double b = cl.blueF();

        //Inverse Red, Green, and Blue
    if (r > 0.04045) r = pow((r+0.055)/1.055, 2.4);
    else r = r / 12.92;

    if (g > 0.04045) g = pow((g+0.055)/1.055, 2.4);
    else g = g / 12.92;

    if (b > 0.04045) b = pow((b+0.055)/1.055, 2.4);
    else b = b / 12.92;

        //return new color. Convert 0..1 back into 0..255
    QColor res;
    res.setRedF(r);
    res.setGreenF(g);
    res.setBlueF(b);
    return res;
}
//--------------------------------------------------------------------------
QColor ColorGrid::strightRGBComp(QColor cl)
{
       double r = cl.redF();
       double g = cl.greenF();
       double b = cl.blueF();

       //Apply companding to Red, Green, and Blue
       if (r > 0.0031308) r = 1.055*pow(r, 1/2.4)-0.055;
          else r = r * 12.92;
       if (g > 0.0031308) g = 1.055*pow(g, 1/2.4)-0.055;
           else g = g * 12.92;
       if (b > 0.0031308) b = 1.055*pow(b, 1/2.4)-0.055;
           else b = b * 12.92;

       //return new color. Convert 0..1 back into 0..255
       QColor res;
       res.setRedF(r);
       res.setGreenF(g);
       res.setBlueF(b);

       return res;
}
//--------------------------------------------------------------------------
double ColorGrid::brightCalc(QColor cl)
{
    double gamma = 0.43;
    return pow(cl.redF() + cl.greenF() + cl.blueF(), gamma);
}
//--------------------------------------------------------------------------
double ColorGrid::linerInterpol(double y1, double y2, double x )
{
    double y = 0.0;
    y = y1 + (y2 - y1)/(1.0 - 0.0)*(x - 0.0);
    return y;
}
//--------------------------------------------------------------------------
QColor ColorGrid::intToColorEx(int color_int)
{
    double mix_color = normalMix(color_int); //[0..1]

    QColor cl1 = QColor::fromRgbF(0., 0., 1., 1.);   // чисто синий цвет
    QColor cl2 = QColor::fromRgbF(1., 0., 0., 1.);   // чисто красный цвет
    QColor cl3 =  QColor::fromRgbF(0., 1., 0., 1.);   // чисто зеленый цвет

    // 1. invers RGB companding    
    QColor cl1_ = inversRGBComp(cl1);
    QColor cl2_ = inversRGBComp(cl2);
    QColor cl3_ = inversRGBComp(cl3);

    // 2. linerInterpolation  r,g,b [0..1]
    double r(0.0), g(0.0), b(0.0);
    if(color_int < 0){  // заданный цвет в сине-красной области
       r = linerInterpol(cl1_.redF(), cl2_.redF(), mix_color);
       g = linerInterpol(cl1_.greenF(), cl2_.greenF(), mix_color);
       b = linerInterpol(cl1_.blueF(), cl2_.blueF(),  mix_color);
    }
    else if(color_int >= 0){  // заданный цвет в красно-зеленой области
       r = linerInterpol(cl2_.redF(), cl3_.redF(), mix_color );
       g = linerInterpol(cl2_.greenF(), cl3_.greenF(), mix_color );
       b = linerInterpol(cl2_.blueF(), cl3_.blueF(),  mix_color );
    }

    // 3. brightness calculation
    double br_(0.0);
    double br1 = brightCalc(cl1_);
    double br2 = brightCalc(cl2_);
    double br3 = brightCalc(cl3_);

    if(color_int < 0)
       br_ = linerInterpol(br1, br2, mix_color);
    else if(color_int >= 0)
       br_ = linerInterpol(br2, br3, mix_color);

    const double gamma = 0.43;
    double intens = pow(br_, 1/gamma);
    double factor = intens / (r + g + b);

    // factor appling to 2. r,g,b [0..1]
    if ((r + g + b) != 0.0) {
        r = (r * factor);
        g = (g * factor);
        b = (b * factor);
    }

    //Apply strighRGB compaunding
    QColor result = strightRGBComp(QColor::fromRgbF(r, g, b));
    return result;
}
//--------------------------------------------------------------------------
QColor ColorGrid::intToColor(int color_int)
{
    int r(0);   // красный
    int g(0);   // зеленый
    int b(0);   // синий
    int a(255); // прозрачность

    if (color_int < 0){     // оттенок меняется от синего к красному
        g = 0;
        b = abs(color_int * 5) + 5;
        r = 255 - b;
    }
    else if(color_int > 0){  // оттенок меняется от красного к зеленому
        b = 0;
        r = 255 - (color_int * 5);
        g = 255 - r;
    }
    else if(color_int == 0){  // чисто красный цвет
        g = 0;
        b = 0;
        r = 255;
    }
    return QColor::fromRgb(r, g, b, a);
}
//--------------------------------------------------------------------------
void ColorGrid::keyPressEvent(QKeyEvent *event) {

  int row_cur = currentRow();
  int col_cur = currentColumn();
  int num  = row_cur * m_col + (col_cur );  // сквозная нумерация ячеек начиная с 0

  if (event->key() == Qt::Key_Return) {  // клавиша ENTER обрабатывается как Qt::Key_Return
      emit numberCellSig(num);
      emit colorCellSig(m_colorCells.at(num));
   }
  if (event->key() == Qt::Key_Down){
      QTableWidget::keyPressEvent(event);
  }
  if (event->key() == Qt::Key_Up){
      QTableWidget::keyPressEvent(event);
  }
  if (event->key() == Qt::Key_Left){
      QTableWidget::keyPressEvent(event);
  }
  if (event->key() == Qt::Key_Right){
      QTableWidget::keyPressEvent(event);
  }

}
//-------------------------------------------------------------------------------------
void ColorGrid::resizeEvent(QResizeEvent * event)
{
   Q_UNUSED(event)

    int width = this->width() - (this->verticalHeader()->width()+3);
     for(int i = 0; i <m_col ; ++i){
      float dt = 1./m_col;
      this->setColumnWidth(i, width *dt); // 1/col
    }

    int height = this->height() - (this->horizontalHeader()->height()+3);
     for(int i = 0; i < m_row + 1; ++i){
         float dt = 1./m_row;
         this->setRowHeight(i, height *dt);  // 1/row
     }

    changeFontSize();  // настраиваем размер шрифта приложения при изменении размера панели

 }
//--------------------------------------------------------------------------------------
