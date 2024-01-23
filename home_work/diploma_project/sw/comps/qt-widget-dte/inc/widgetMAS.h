#ifndef WIDGETMASAS_H
#define WIDGETMASAS_H

#include <QWidget>
#include "widgetClient.h"

namespace Ui {
class WidgetMAS;
}

class WidgetMAS : public WidgetClient
{
    Q_OBJECT

public:
    explicit WidgetMAS(QWidget *parent = nullptr);
    ~WidgetMAS();

private:
    Ui::WidgetMAS *ui;
};

#endif // WIDGETMASAS_H
