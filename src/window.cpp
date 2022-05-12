#include "window.h"
#include "ui_window.h"

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    ui->mainToolBar->addAction("pen");
    ui->mainToolBar->addAction("line");         // draw line button added to tool bar
    ui->mainToolBar->addAction("rectangle");    // replace with icons
    ui->mainToolBar->addAction("clear");

    //ui->centralWidget->setMouseTracking(true);

    connect(ui->mainToolBar, &QToolBar::actionTriggered, ui->centralWidget, &canvas::selectTool);   // connects tool bar button to slot
}

Window::~Window()
{
    delete ui;
}
