#include "tablemess.h"
#include "textcmd.h"
#include <QHBoxLayout>
#include <QApplication>
#include <QVector>
#include <QDir>
#include <QMessageBox>
#include <QTextStream>

//------------------------------------------------------------------
TableMess::TableMess(QWidget * parent, int row_count, int col_count)
     : QTableWidget(parent), m_row(0), m_col(2)
 {
    Q_UNUSED(row_count);
    Q_UNUSED(col_count);

    initTable();
 }
//-------------------------------------------------------------
// настройка внешнего вида таблицы - журнала сообщений
void TableMess::initTable()
{
  /*  QString fl_path;
    QString APP_ABS_PATH = QCoreApplication::applicationDirPath();
    QString APP_ABS_NAME = QCoreApplication::applicationName();
    QString LOG_PATH  = "/LOG/";               // дир. с файлами конфигур.
    QString log_dir = APP_ABS_PATH + LOG_PATH;
    QDateTime cdt = QDateTime::currentDateTime();
    QString s = cdt.toLocalTime().toString("yy_MM_dd_hh_mm");
    QString LOG_FL_DIR = s + "/";              // имя папки для log файлов


    m_log_file_dir_name = APP_ABS_PATH + LOG_PATH + LOG_FL_DIR;
    m_log_file_name = APP_ABS_PATH + LOG_PATH + LOG_FL_DIR;

    fl_path = getFullLogFileName(APP_ABS_PATH + LOG_PATH + LOG_FL_DIR);

    if (!checkDirExist(log_dir))     // если LOG дир. не существ.
          if(!makeDir(log_dir))      // пытаемся ее создать
             {}

    QString     fl_dir = APP_ABS_PATH + LOG_PATH + LOG_FL_DIR;

    if (!checkDirExist(fl_dir))     // если текущей дир. не существ.
          if(!makeDir(fl_dir))      // пытаемся ее создать
             {}

     m_log_file = new QFile(fl_path);

     if (!m_log_file->exists()){
         if(!m_log_file->open(QIODevice::ReadWrite|QIODevice::Append)){
            QMessageBox::critical(0, "Ошибка","Файл : "
                                                 "" + fl_dir  + fl_path +
                                                 " не может быть создан ! "
                                                 "Программа будет закрыта.");
          }
         else { // если файл создан               
             logFileWriteHeader(); // добавим в него заголовок
          }
     }
     */
    setColumnCount(m_col);
    setRowCount(m_row);

    QStringList vHeader, hHeader;
    setFocusPolicy(Qt::StrongFocus);
    int S = 0xff; // белый
    int D = 0xdc; // светло-светло серый

    QPalette pl;
    QColor cl_ = QColor::fromRgb(S, S, S);
    QColor cl2 = QColor::fromRgb(D, D, D);
    pl.setBrush(QPalette::Highlight, cl_);
    pl.setBrush(QPalette::Button, cl2);
    pl.setBrush(QPalette::HighlightedText, Qt::black);
    this->setPalette(pl);

    horizontalHeader()->setStyleSheet("::section {background-color:#f0f0f0}");

                   // составляем шапку (верт., гориз. таблицы)
    hHeader << "Время" <<"Команда";

    for(int j = 0; j < m_row; ++j)
       vHeader << QString::number(j);

    setHorizontalHeaderLabels(hHeader);

    setAlternatingRowColors(false);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//Qt::ScrollBarAsNeeded);

    horizontalHeader()->setDefaultSectionSize(100);
    horizontalHeader()->setStretchLastSection(true);
    verticalHeader()->setDefaultSectionSize(20);
    setSelectionMode(QTableWidget::SingleSelection);
    setSelectionBehavior(QTableWidget::SelectRows);
    setWordWrap(true);
    horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

    horizontalHeader()->setMaximumSectionSize(300);
    verticalHeader()->setStretchLastSection(false);
    verticalHeader()->setHidden(true);
    resizeColumnsToContents();
    resizeRowsToContents();

   // EditTriggers trigers(QAbstractItemView::NoEditTriggers);
   // setEditTriggers(trigers);                           //  значение выбранной ячейки не редактируется
   // setTabKeyNavigation(true);                          // перемещение по ячейкам табл. - возможно с пом. клавиатуры

    // синхронизация навигации по таблицы с положением скролбара
    connect(this, &TableMess::currentCellChanged, this, &TableMess::slotScrollBarSynchro);    
   // connect(this, &TableMess::destroyed, this,&TableMess::slotSaveLog);
}
//-----------------------------------------------------------------
void TableMess::slotSetSaveParam(bool save)
{
    m_log_save = save;
}
//-----------------------------------------------------------------
TableMess::~TableMess()
{
  /*  if(m_log_file->isOpen()){ // если текущий файл сессии открыт
       m_log_file->close();   // закрываем его
       delete m_log_file;     // удаляем указатель
    }

    if(!m_log_save){
        if (checkDirExist(m_log_file_dir_name))     // если текущая дир. существ.
              remDir(m_log_file_dir_name);
    }
    */
}
//------------------------------------------------------------------
// сохранение созданных за сессию лог файлов при закрытии приложения
// (задается пользователем)
void TableMess::slotSaveLog()
{
  /*  if(!m_log_save){
        if (checkDirExist(m_log_file_dir_name))     // если текущая дир. существ.
              remDir(m_log_file_dir_name);
    }
    */
}
//-----------------------------------------------------------------
void TableMess::slotSizeLog(double log_size)
{
   Q_UNUSED(log_size);
   // m_log_file_max_size = static_cast<qint64>((log_size *1024.)*1024.);
}
//=================================================================

// отображение сообщения целиком в окне просмотра ниже табл.
void TableMess::slotOpenWholeMess(QString & whole_mess, QColor cl)
{
    // сигнал отправляем в WidgetMess для отображения сообщения в нижнем окне
   emit sigWholeMessToRead(whole_mess, cl);
}
//-----------------------------------------------------------------
//=================================================================
// синхронизация скрол бара с выделеным элементом в табл.
void TableMess::slotScrollBarSynchro(int row, int column)
{
   Q_UNUSED(column);
   emit sigActiveRow(row);
}
//-------------------------------------------------------------------
//===================================================================
//---------------------Режим ЗАПИСИ(приём сообщений)-----------------
//-------------------------------------------------------------------
// добавление нового сообщения в лог файл и в табл.
void TableMess::slotAddMsg(int tx,  QDateTime   tm, QString &type_com,
                             const QString &mess, const QColor  &cl)
{
     MessageS msg;
     msg.tx = tx;
     msg.tm = tm.toString("HH:mm:ss.zzz");
     msg.type_cmd = type_com;
     msg.mess = mess;
     msg.color_name = cl.name();

  /* if(!isCreateNewLogFileInSession()){ // если новый лог файл создаванет НЕ НУЖНО
     //logFileWriting(msg);
     ++m_vec_size;                     // размер вектора
   }
   else{                               // новый лог файл создавать НУЖНО (текущий файл достиг кретического размера)
       ++m_log_file_num;
      if(m_log_file->isOpen())         // если текущий файл открыт
         m_log_file->close();          // закрываем его
      delete m_log_file;               // удаляем указатель на лог.файл для записи
      createNewLogFileInSession(getFullLogFileName(m_log_file_name)); // создаем новый лог. файл
      //logFileWriting(msg);
   }
   logFileWriting(msg);     // записываем в старый или в новый файл сообщение
   */
   addMsgToWidget(msg);     // отображаем записанное сообщение в табл. журанла
             // !!! текущий файл для записи сообщений остается открытым до закрытия приложения или до создания нового файла.
}
//------------------------------------------------------------------
// проверка перед записью сообщ. в лог. файл не переполнен ли файл
// и не требуется ли создать новый лог. файл в данной сессии
bool TableMess::isCreateNewLogFileInSession()
{
    bool res = false;
   // QString full_name = getFullLogFileName(m_log_file_name);
 /*   qint64 size = getLogFileSize();          // получаем размер текущего лог файла
    if (size >= m_log_file_max_size)         // сравниваем его с размером файла, заданным пользователем
       res = true;                           // принимаем решение о создании нового файла
       */
   return res;
}
//------------------------------------------------------------------
// создание еще одного нового лог. файла в данной сессии
void TableMess::createNewLogFileInSession(QString full_name)
{
    Q_UNUSED(full_name);
   /*  m_log_file = new QFile(full_name);
     m_log_file->open(QIODevice::ReadWrite|QIODevice::Append);
     logFileWriteHeader();
     m_vec_size = 0;
     */
}
//-----------------------------------------------------------------
// запись в текущий лог файл заголовка
void TableMess::logFileWriteHeader()
{
  /*  m_look_up = false;
   if (m_log_file->isOpen()){    //QIODevice::ReadWrite|QIODevice::Append)) {
       m_log_file->write(" TX/RX * Time * CMDName * ColorName * Message *\n");
       m_log_file->flush();
     }
     */
}
//------------------------------------------------------------------
// добавление нового сообщения в таблицу(журнал)
void TableMess:: addMsgToWidget(MessageS msg)
{
    QTableWidgetItem *prwi_tm = 0;
          // добаляем время прихода сообщения до приложения
    prwi_tm = new QTableWidgetItem(msg.tm);

    QColor cl(msg.color_name);
    TextCMD  * te = new TextCMD(msg.mess, cl, this);       // поле делегат для отображения команды в табл (текст. поле)
    connect(te, &TextCMD::sigWholeMess, this, &TableMess::slotOpenWholeMess);

               // заносим только значения поля "команда"(type)
    te->setText(msg.type_cmd);  //list.at(1));  // в списке  [0] - id, [1] - команда, ...

    this->insertRow(0);

    setItem(0, 0, prwi_tm);  // помещаем в журнал время

    setCellWidget(0, 1, te);  // помещаем в журнал делегат со значением команды
    int row = rowCount();  //rowCount()-1;
    setCurrentCell(row, 1);     // устанавливаем текущий элемент журнала - последний записанный

    if(row >= getVisibleMessNumber())
      this->removeRow(getVisibleMessNumber()-1);
}
//----------------------------------------------------------------
// расчет кол-ва видимых записей сообщений в табл.
// (зависит от видимого размера таблицы в данный момент)
int TableMess::getVisibleMessNumber()
{
   int hgh_size = height();
   int hgh_row = this->rowHeight(0);
   int count = (hgh_size / (hgh_row + 1));
   if (count < 3)
       count = 3;
   return count;
}
//-----------------------------------------------------------------
// запись в текущий лог файл пришедшего / отправляемого сообщения

void TableMess::logFileWriting(MessageS &msg)
{
    Q_UNUSED(msg);
 /*   m_look_up = false;
    QString tx_rx{""};

    if(msg.type_cmd!=""){  // если запись не пустая
     if (m_log_file->isOpen()){    //QIODevice::ReadWrite|QIODevice::Append)) {
         if (msg.tx == 1){
            tx_rx = "TX";
         }
         else
            tx_rx = "RX";
         m_log_file->write(tx_rx.toUtf8() +" * "+msg.tm.toUtf8()
                           +" * "+ msg.type_cmd.toUtf8()+" * " + msg.color_name.toUtf8()
                           +" * "+msg.mess.toUtf8()+"\n");
         m_log_file->flush();
     }
    }
    */
}
//------------------------------------------------------------------
//===================================================================

//---------------------Режим ПРОСМОТРА журнала за сессию ------------
//------------------------------------------------------------------
// задействование скрол бара для просмотра данных лог файла // нажимаем на стрелку (внизу/вверху)на скрол баре
void TableMess::slotLookUpJounal(int action)
// action - задает вниз(1) или вверх(2) пользователь перемещает скрол бар
{
        if(!m_look_up){           // только собираемся просматривать данные
          loadDataForLookUp();    // загружаем данные для просмотра из лог. файла
          m_step = 0;
          setCurrentCell(m_step,1);  // переводим указатель на последнее записанное сообщение
                                     //(оно будет находится сверху табл.)
          m_look_up = true;          // данные загружены и можно приступать к просмотру
        }
       else {   // данные уже загружены, теперь можно перемещаться по ним и просматривать
           if(action == 1 && m_step != rowCount())  // перемещаемся вниз
              ++m_step;
           else if(action == 2 && m_step!=0)        // перемещаемся вверх
              --m_step;
          setCurrentCell(m_step,1);
       }
}
//------------------------------------------------------------------
// задействование скрол бара для просмотра данных лог файла
// перемещаем ползунок (вниз/вверх) скролл бара до определенной позиции
void TableMess::slotLookUpJounalTo(int position)
{
       if(!m_look_up){     // только собираемся просматривать данные
          loadDataForLookUp();
          m_step = 0;
          setCurrentCell(m_step,1);
          m_look_up = true;
       }
       else { // данные уже загружены, теперь их просматривать
              // перемещая ползунок в определенную позицию
          m_step = position;
          setCurrentCell(m_step,1);
       }
}
//-------------------------------------------------------------------
// загрузка лог файла с диска для просмотра данных
void TableMess::loadDataForLookUp()
{
  //  setRowCount(0);             // очищаем табл. журнала
 //   readLogFile(m_log_file);    // считываем в вектор сообщений текущий файл он уже открыт
   /* for(int i = m_log_file_num - 1; i > -1; i--){
        QString full_name = m_log_file_dir_name + QString::number(i);
        QFile *log_file = new QFile(full_name);
        log_file->open(QIODevice::ReadWrite|QIODevice::Append);
        readLogFile(log_file);
        log_file->close();
        delete log_file;
  //  }*/
}
//------------------------------------------------------------------
void TableMess::readLogFile(QFile *file_to_read)
{
    Q_UNUSED(file_to_read);
  /*  if (file_to_read->isOpen()){    //QIODevice::ReadWrite|QIODevice::Append)) { // если текущий файл - открыт

      file_to_read->seek(0);        // ищем начало файла
      //qDebug() << m_log_file->readAll();
      QTextStream fl_strm(file_to_read);  // создаем текстовый поток
      QString data = fl_strm.readAll(); // считываем содержимое файла в поток

      QStringList list;
      list = data.split('\n');  // содержимое считанное из файла разбиваем на строки

      for(int i = 0; i < list.count()-1; ++i){ // обходим все строки
          if(list.at(i)!=""){                                // если запись не пустая
             m_vec_msg.append(parseMsgString(list.at(i)));   // помещаем ее в вектор сообщений предварительно разобрав ее на поля структуры MassageS
             lookUpMsgToWidget(parseMsgString(list.at(i)));  // помещаем сообщение в таблицу
          }
       }
    }
    */
}

//------------------------------------------------------------------
// разбор сообщения считанного из лог.файла в виде строки: "tx*tm*type_cmd[:status]*color_name*mess\n"
// на отдельные поля структуры MessageS
MessageS TableMess::parseMsgString(QString msg_string)
{
    MessageS msg_struct;
    int pos{0};

    pos = msg_string.indexOf('*');
    int tx = (msg_string.left(pos)).toInt();
    msg_string.remove(0,pos+1);

    pos = msg_string.indexOf('*');
    QString str_tm = (msg_string.left(pos));
    msg_string.remove(0,pos+1);

    pos = msg_string.indexOf('*');
    QString str_cmd = (msg_string.left(pos));
    msg_string.remove(0,pos+1);

    pos = msg_string.indexOf('*');
    QString str_color = (msg_string.left(pos));
    msg_string.remove(0,pos+1);

    QString str_msg = msg_string;

    msg_struct.tx = tx;
    msg_struct.tm = str_tm;
    msg_struct.type_cmd = str_cmd;
    msg_struct.color_name = str_color;
    msg_struct.mess = str_msg;

    return msg_struct;
}
//------------------------------------------------------------------
// занесение сообщения из структуры в табл. журнала
void TableMess:: lookUpMsgToWidget(MessageS msg)
{
    QTableWidgetItem *prwi_tm = 0;
          // добаляем время прихода сообщения до приложения
    prwi_tm = new QTableWidgetItem(msg.tm);

    QColor cl(msg.color_name);
    TextCMD  * te = new TextCMD(msg.mess, cl, this);       // поле делегат для отображения команды в табл (текст. поле)
    connect(te, &TextCMD::sigWholeMess, this, &TableMess::slotOpenWholeMess);

               // заносим только значения поля "команда"(type)
    te->setText(msg.type_cmd);  //list.at(1));  // в списке  [0] - id, [1] - команда, ...

    this->insertRow(0);

    setItem(0, 0, prwi_tm);  // помещаем в журнал время

    setCellWidget(0, 1, te);  // помещаем в журнал делегат со значением команды
}

//===================================================================
//----------------------общие функции для всех режимов работы:(ЗАПИСИ, ПРОСМОТРА)
//-----------------------------------------------------------------
// формирования полного имени файла : путь + имя
QString TableMess::getFullLogFileName(QString fl_name)
{
    Q_UNUSED(fl_name);
  QString  full_name;// = fl_name + QString::number(m_log_file_num);
  return full_name;
}
//-----------------------------------------------------------------
// получение размера текущего лог файла
qint64 TableMess::getLogFileSize()
{
   return m_log_file->size();
}
//------------------------------------------------------------------
// получение кол-во записей в векторе
qlonglong TableMess::getVecSize()
{
    return  m_vec_size-1;
}
//-------------------------------------------------------------------
// проверка существования директории с заданным имененм
bool TableMess:: checkDirExist(QString &dir_name_)
{
    QDir dir(dir_name_);
    if (!dir.exists()){
     //   QMessageBox::critical(0,"Ошибка","Директория:  "+
     //                         dir_name_+
     //                         "  НЕ существует. Мы попробуем её создать." );
        return false;
    }
    else
        return true;
}
//---------------------------------------------------------------------
// создание директории с заданным имененм
bool TableMess:: makeDir(QString &dir_name_)
{
    QDir dir(dir_name_);
    if (!dir.mkdir(dir_name_)){
        //QMessageBox::critical(0,"Ошибка","Директория:  "+
       //                       dir_name_+
      //                        "  НЕ может быть создана.Проверте права доступа." );
        return false;
    }
    else
        return true;
}
//------------------------------------------------------------------
// удаление дир. с заданным имененм
bool TableMess::remDir(QString &dir_name_)
{
    QDir dir(dir_name_);
    if(m_log_file->isOpen()){
       m_log_file->close();
       delete m_log_file;
    }
    if (!dir.removeRecursively()){   //.mkdir(dir_name_)){
        //QMessageBox::critical(0,"Ошибка","Директория:  "+
       //                       dir_name_+
      //                        "  НЕ может быть создана.Проверте права доступа." );
        return false;
    }
    else
        return true;
}
//----------------------------------------------------------------
// переопределенные события мыши - УБИРАТЬ нельзя!!!
void   TableMess::mousePressEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}
//----------------------------------------------------------------
void  TableMess::mouseDoubleClickEvent(QMouseEvent *e)
{
    Q_UNUSED(e);
}
//----------------------------------------------------------------

