#ifndef WINDOW_H
#define WINDOW_H

#include "canvas.h"
#include "ui_window.h"

#include <QMainWindow>
#include <QMouseEvent>
#include <QToolBar>
#include <QDebug>
#include <QPoint>
#include <QLine>

namespace Ui {
class Window;
}

class Window : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Window(QWidget *parent = 0);
    //void mouseMoveEvent(QMouseEvent *event);
    ~Window();
    Ui::Window *ui; // includes canvas as main widget in definition
    //canvas *mainCanvas;
    
private slots:


private:
};

#endif // WINDOW_H
