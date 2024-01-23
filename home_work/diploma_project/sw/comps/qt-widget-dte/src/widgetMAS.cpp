#include "widgetMAS.h"
#include "ui_widgetMAS.h"
//------------------------------------------------------------------------
WidgetMAS::WidgetMAS(QWidget *parent) :
    WidgetClient(parent),
    ui(new Ui::WidgetMAS)
{
    ui->setupUi(this);
}
//-------------------------------------------------------------------------
WidgetMAS::~WidgetMAS()
{
    delete ui;
}
//---------------------------------------------------------------------------
