#ifndef WIDGETCLIENT_H
#define WIDGETCLIENT_H

#include <QWidget>

namespace Ui {
class WidgetClient;
}

class WidgetClient : public QWidget
{
    Q_OBJECT

public:
    explicit WidgetClient(QWidget *parent = nullptr);
    ~WidgetClient();

private:
    Ui::WidgetClient *ui;
};

#endif // WIDGETCLIENT_H
