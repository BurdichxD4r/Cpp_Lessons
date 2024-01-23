#ifndef TABLEMESS_H
#define TABLEMESS_H

#include <QObject>
#include <QWidget>
#include <QTableWidget>
#include <QHeaderView>
#include <QTime>
#include <QMouseEvent>
#include <QFile>

struct MessageS{
    int     tx   = 0;   //флаг вход/выход сообщения
    QString tm{""} ;    // время получения / отправки сообщения
    QString type_cmd;   // наименование команды (с полем status)
    QString mess ;      // сообщение целиком
    QString color_name; // цвет шрифта сообщения

};
class TableMess: public QTableWidget
{
    Q_OBJECT
public:
    TableMess(QWidget *parent = 0, int row = 1, int col = 3);
    ~TableMess();
    void      initTable();
    qlonglong getVecSize();     //  получить значение размера вектора данных m_vec_size
    int       getVisibleMessNumber();

public slots:
    void slotAddMsg(      int      tx,              // флаг вход/исход сообщ.
                           QDateTime   tm,              // время прихода
                           QString &type_com,       // название команды
                     const QString &mess,           // содержание сообщения
                     const QColor  &cl);            // цвет шрифта

    void slotOpenWholeMess(QString &, QColor );
    void slotLookUpJounal(int action);
    void slotLookUpJounalTo(int position);
    void slotScrollBarSynchro(int row, int column);
    void slotSaveLog();
    void slotSetSaveParam(bool save);
    void slotSizeLog(double);

private:

    int              m_step{0};
    bool             m_look_up{false}; // флаг просмотра (компонент переведен в режим просмотра - загружен файл с данными)
    int              m_row;           // число рядов в табл.
    int              m_col;           // число колонок в табл.
    QColor           m_curr_color;    // основной цвет сообщения
    qlonglong        m_vec_size{0};   // размер вектора
    QVector<MessageS> m_vec_msg;

    QFile           * m_log_file;
    int             m_log_file_num{0};  // номер лог. файла в текущей сессии
    QString         m_log_file_name{""};    // имя лог файла на уровне сессии(полное имя с дерикториями)
    QString         m_log_file_dir_name{""};    // имя папки логов в данной  сессии (от 0 до N)
    qint64          m_log_file_max_size{10240}; // макс. размер элемент. лог файла в данной сессии байт (10 Kбайт)
    bool            m_log_save{true};         // сохрание лог. файла при закрытии виджета


    bool    checkDirExist(QString &dir_name_);
    bool    makeDir(QString &dir_name_);
    bool    remDir(QString &dir_name_);
    bool    isCreateNewLogFileInSession();
    void    createNewLogFileInSession(QString full_name);
    QString getFullLogFileName(QString fl_name);
    qint64  getLogFileSize();
    void    logFileWriteHeader();
    void    logFileWriting(MessageS &msg);
    void    addMsgToWidget(MessageS msg);
    void    loadDataForLookUp();
    void    readLogFile(QFile *file_to_read);
    MessageS parseMsgString(QString msg_string);
    void    lookUpMsgToWidget(MessageS msg);

protected:
        // события мыши переопределяем для удаления ненужных последствий после нажатия
     void    mousePressEvent(QMouseEvent *event) override;
     void    mouseDoubleClickEvent(QMouseEvent *event) override;

signals:
     void    sigWholeMessToRead(QString &, QColor);   // сигнал в WidgetMess для отображения всего сообщения в нижнем текстовом поле
     void    sigCloseWidg();
     void    sigActiveRow(int);   //

};

#endif // TABLEMESS_H
