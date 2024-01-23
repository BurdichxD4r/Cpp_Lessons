#include "widgetClient.h"
#include "ui_widgetClient.h"

WidgetClient::WidgetClient(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::WidgetClient)
{
    ui->setupUi(this);
}

WidgetClient::~WidgetClient()
{
    delete ui;
}
