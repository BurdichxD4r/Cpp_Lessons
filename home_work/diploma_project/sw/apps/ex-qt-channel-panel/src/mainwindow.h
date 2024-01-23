#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "colorgrid.h"
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QSpinBox>
#include <QLabel>
#include <QTime>
#include <QTimer>

QT_BEGIN_NAMESPACE

//namespace Ui { class MainWindow; }

QT_END_NAMESPACE

const int ROW_NUM = 16; //8;
const int COL_NUM = 16;

const int COLOR_MIN = -50;
const int COLOR_MAX = 50;
const int TIMER_PARAM = 1000;   // период работы таймера (мс)

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private slots:
    void   activeChange();
    void   colorChange();
    void   emulGrad();
    void   timeAlarmSlot();
    void   timerOnSlot();    // включение таймера

private:
    QFrame      *mainFrame;
    ColorGrid   *m_grid;
    QWidget     *m_panelGrid;
    QGridLayout *m_gridbox;

    QPushButton *m_activBut;
    QPushButton *m_emulBut;
    QPushButton *m_timerBut;

    QPushButton *m_colorBut;
    QSpinBox    *m_colorSp;
    QSpinBox    *m_cellSp;
    int          m_cell_num = 0;    // количество ячеек в таблице (сквозная нумерация от 0 до n)
    QTimer       *m_timer;

signals:
    void   activeChSig(int);
    void   colorChSig(int, int);
    void   gridTimeOutSig(int);
    void   gridAvailDoSig(int);   // обращение к ячейке

    //Ui::MainWindow *ui;
 };
#endif // MAINWINDOW_H
