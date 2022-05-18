#ifndef CANVAS_H
#define CANVAS_H

#include <QObject>
#include <QWidget>
#include <QString>
#include <QLine>
#include <QDebug>
#include <QMouseEvent>
#include <QPoint>
#include <QAction>
#include <QList>
#include <QPainter>
#include <QPaintEvent>
#include <QPen>
#include <QColor>

#include "serial.hpp"

class canvas : public QWidget
{
    Q_OBJECT
public:
    explicit canvas(QWidget *parent = 0);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);

protected:
    void paintEvent(QPaintEvent *) override; // updates drawing elements on window

signals:
    void sendPacket(Serial::packet changedPacket); // emitted when a new packet is ready to be sent

public slots:
    void selectTool(QAction* tool);      // updates the selected tool after a toolbar action
    void selectColor(QAction* color);    // updates the selected color after a toolbar action
    void packetReceived(Serial* serial); // used tp receive packets of drawing elements

private:
    struct LineGroup
    {
        QColor color;
        QBrush brush;
        QList<QLine> lines;
    };
    
    QList<LineGroup> lines; // list of groups of drawing elements
    LineGroup currentLines; // group of lines currently being drawn
    QString toolType;       // option selected on the window toolbar
    
    QList<Serial::packet> serialize();  // serialization of current drawing tool into packets
    void deserialize(Serial::packet p); // deserialization of drawing elements from packet
};

#endif // CANVAS_H
