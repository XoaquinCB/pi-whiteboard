#include "window.h"
#include "ui_window.h"
#include <QToolButton>
#include <QMenu>

Window::Window(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::Window)
{
    ui->setupUi(this);
    ui->mainToolBar->addAction("pen");
    ui->mainToolBar->addAction("line");         // draw line button added to tool bar
    ui->mainToolBar->addAction("rectangle");    // replace with icons
    ui->mainToolBar->addAction("clear");

    QToolButton *colourButton = new QToolButton(this);
    colourButton->setText("colour");
    colourButton->setPopupMode(QToolButton::InstantPopup);
    QMenu *colourMenu = new QMenu(colourButton);
    colourMenu->addAction(new QAction("red", this));
    colourMenu->addAction(new QAction("black", this));
    colourMenu->addAction(new QAction("blue", this));
    colourButton->setMenu(colourMenu);
    ui->mainToolBar->addWidget(colourButton);

    connect(colourButton, &QToolButton::triggered, ui->centralWidget, &canvas::selectColor);
    connect(ui->mainToolBar, &QToolBar::actionTriggered, ui->centralWidget, &canvas::selectTool);   // connects tool bar button to slot
}

Window::~Window()
{
    delete ui;
}
