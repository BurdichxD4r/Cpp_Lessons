#include "mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    
{
    mainFrame = new QFrame(this);
 //   m_panelGrid = new QWidget(this);
    m_gridbox = new QGridLayout(mainFrame);

   // m_grid = new ColorGrid(m_panelGrid, ROW_NUM, COL_NUM);
    m_grid = new ColorGrid(this, ROW_NUM, COL_NUM);
    m_emulBut = new QPushButton("Демонстрация градиента", this );
    m_activBut = new QPushButton("Изменить активность", this );
    m_colorBut = new QPushButton("Изменить цвет", this);  
    m_timerBut = new QPushButton("Запустить таймер", this);

    QLabel *colorlb = new QLabel("Номер цвета", this);
    QLabel *celllb = new QLabel("Номер ячейки", this);
    colorlb->setAlignment(Qt::AlignRight);
    celllb->setAlignment(Qt::AlignRight);

    m_colorSp = new QSpinBox(this);
    m_cellSp = new QSpinBox(this);

    colorlb ->setBuddy ( m_colorSp );
    celllb ->setBuddy ( m_cellSp );

    m_cell_num = ROW_NUM * COL_NUM - 1;
    m_cellSp->setMinimum(0);
    m_cellSp->setMaximum(m_cell_num);
    m_colorSp->setMinimum(COLOR_MIN);
    m_colorSp->setMaximum(COLOR_MAX);

    m_gridbox->addWidget(m_grid, 0, 0, 1, 3);
   // m_gridbox->addWidget(m_panelGrid, 0, 0, 1, 3);
    m_gridbox->addWidget(m_activBut, 1, 0 );
    m_gridbox->addWidget(m_colorBut, 2, 0);
    m_gridbox->addWidget(m_timerBut, 3, 0);
    m_gridbox->addWidget(colorlb, 1, 1);
    m_gridbox->addWidget(celllb, 2, 1);
    m_gridbox->addWidget(m_colorSp, 1, 2);
    m_gridbox->addWidget(m_cellSp, 2, 2);
    m_gridbox->addWidget(m_emulBut, 4, 0);

    m_timer = new QTimer();
    connect(m_timer, SIGNAL(timeout()),this, SLOT(timeAlarmSlot()));

    this->setCentralWidget(mainFrame);

    setWindowTitle("Color Grid");

    connect (m_activBut, &QAbstractButton::clicked, this, &MainWindow::activeChange);
    connect(this, &MainWindow::activeChSig, m_grid, &ColorGrid::checkChngSlot );

    connect (m_colorBut, &QAbstractButton::clicked, this, &MainWindow::colorChange);
    connect(this, &MainWindow::colorChSig, m_grid, &ColorGrid::colorChngSlot );

    connect (m_timerBut, &QAbstractButton::clicked, this, &MainWindow::timerOnSlot);
    connect(this, &MainWindow::gridTimeOutSig, m_grid, &ColorGrid::availCellDescSlot);
    connect(this, &MainWindow::gridAvailDoSig, m_grid, &ColorGrid::reFreshAvailCellSlot);

    connect (m_emulBut, &QAbstractButton::clicked, this, &MainWindow::emulGrad);
    connect(m_grid, &ColorGrid::numberCellSig, m_cellSp, &QSpinBox::setValue);
    connect(m_grid, &ColorGrid::colorCellSig, m_colorSp, &QSpinBox::setValue);

}
//-----------------------------------------------------
MainWindow::~MainWindow()
{
    
}
//------------------------------------------------------
void MainWindow::timerOnSlot()
{
    m_timer->start(TIMER_PARAM);
}
//------------------------------------------------------
// обработка события таймера
void MainWindow::timeAlarmSlot()
{
     m_timer->stop();
    // сначала уменьшаем параметр досупности для всех ячеек
    for(int k = 0; k < m_cell_num; ++k){
       gridTimeOutSig(k);
    }

    // организуме доступ (с востановление параметра доступа) для ряда ячеек
    QVector<int> cell_vec{6, 7, 40, 3, 20};
    for(int i = 0; i < cell_vec.count(); ++i){
    gridAvailDoSig(cell_vec.at(i));   // обращение к ячейке

    }

    m_timer->start(TIMER_PARAM);
}
//------------------------------------------------------
void MainWindow::activeChange()
{
    emit activeChSig(m_cellSp->value());
}
//------------------------------------------------------
void MainWindow::colorChange()
{
    emit colorChSig(m_cellSp->value(), m_colorSp->value());
}
//------------------------------------------------------
void MainWindow::emulGrad()
{
    for (int i = 0; i < 101; ++i){
        emit colorChSig(i, i + COLOR_MIN);
    }
    m_timer->stop();
}
//------------------------------------------------------
