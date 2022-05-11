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
    void paintEvent(QPaintEvent *) override;    // updates drawing elements on window

signals:
    void canvasChanged(QList<QList<QLine>> changedCanvas);      // signal
    void sendPackets(QList<Serial::packet> changedPackets);
    void sendPacket(Serial::packet changedPacket);

public slots:
    void selectTool(QAction* tool);     // updates the selected tool after a toolbar action
    void canvasReceived(QList<QList<QLine>> newCanvas);         // used to receive drawing elements between windows internally
    void packetsReceived(QList<Serial::packet> newPackets);     // used to receive a series of packets between send and receive windows
    void packetReceived(Serial* serial);              // used tp receive a single packet of drawing elements

private:
    QString toolType;           // option selected on the window toolbar
    QList<QList<QLine>> lines;  // list of groups of drawing elements
    QList<Serial::packet> serialize(QList<QLine> lines);    // serialization of drawing elements into bytes
    void deserialize(Serial::packet p);                     // deserialization of drawing elements from bytes

};

#endif // CANVAS_H
