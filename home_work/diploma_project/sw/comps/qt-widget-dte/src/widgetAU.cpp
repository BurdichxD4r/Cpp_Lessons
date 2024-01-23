#include "widgetAU.h"
#include "ui_widgetAU.h"
//------------------------------------------------------
WidgetAU::WidgetAU(QWidget *parent) :
    WidgetClient(parent),
    ui(new Ui::WidgetAU)
{
    ui->setupUi(this);
}
//-------------------------------------------------------
WidgetAU::~WidgetAU()
{
    delete ui;
}
//--------------------------------------------------------
