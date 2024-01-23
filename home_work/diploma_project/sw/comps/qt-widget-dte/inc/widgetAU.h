#ifndef WIDGETAU_H
#define WIDGETAU_H

#include <QWidget>
#include "widgetClient.h"

namespace Ui {
class WidgetAU;
}

class WidgetAU : public WidgetClient
{
    Q_OBJECT

public:
    explicit WidgetAU(QWidget *parent = nullptr);
    ~WidgetAU();

private:
    Ui::WidgetAU *ui;
};

#endif // WIDGETAU_H
