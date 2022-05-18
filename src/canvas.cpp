#include "canvas.h"

canvas::canvas(QWidget *parent) : QWidget(parent)
{

}

void canvas::selectTool(QAction* tool)
{
    qDebug() << tool->text();
    toolType = tool->text();
}

void canvas::selectColor(QAction* color)
{
    qDebug() << color->text();
    currentLines.color.setNamedColor(color->text());
}

void canvas::mouseMoveEvent(QMouseEvent *event)
{
    qDebug() << event->x() << event->y();   // position relative to window

    QPoint point2;
    QPoint point1;

    if(toolType == "pen")
    {
        if(currentLines.lines.isEmpty())
            point1 = event->pos();
        else
            point1 = currentLines.lines.last().p2();
        //qDebug() << "while";
        point2 = event->pos();
        QLine newLine(point1, point2);
        currentLines.lines.append(newLine);
        update();
    }
    if(toolType == "line")
    {
        if(currentLines.lines.isEmpty()){
           QLine line;
           currentLines.lines.append(line);
           point1 = event->pos();
        }
        else
            point1 = currentLines.lines.last().p1();
        point2 = event->pos();
        QLine newLine(point1, point2);
        currentLines.lines.replace(0, newLine);
        update();
    }
    if(toolType == "rectangle")
    {
        if(currentLines.lines.isEmpty()){
            QLine line;
            for(int i = 0; i < 4; i++)
                currentLines.lines.append(line);
            point1 = event->pos();
        }
        else
            point1 = currentLines.lines.first().p1();
        //qDebug() << "while";
        point2 = event->pos();
        QList<QLine> newLines;
        newLines.append(QLine(point1.x(), point1.y(), point1.x(), point2.y()));
        newLines.append(QLine(point1.x(), point2.y(), point2.x(), point2.y()));
        newLines.append(QLine(point2.x(), point2.y(), point2.x(), point1.y()));
        newLines.append(QLine(point2.x(), point1.y(), point1.x(), point1.y()));
        for(int i = 0; i < 4; i++)
            currentLines.lines.replace(i, newLines[i]);
        update();
    }
}

void canvas::mouseReleaseEvent(QMouseEvent *event)
{
    QList<Serial::packet> packets = serialize();
    
    for(int i = 0; i < packets.size(); i++)
    {
        qDebug() << "packetSent";
        emit sendPacket(packets[i]);
    }
    
    // QColor color = penColor;
    // colorList.append(color);
    lines.append(currentLines);
    currentLines.lines.clear();

    qDebug() << "mouse release";
}

void canvas::paintEvent(QPaintEvent*)
{
    QPainter painter;
    painter.begin(this);
    QPen pen;
    for(int i = 0; i < lines.size(); i++)
    {
        pen.setColor(lines[i].color);
        painter.setPen(pen);
        for(int j = 0; j < lines[i].lines.size(); j++)
        {
            painter.drawLine(lines[i].lines[j]);
        }
    }
    
    pen.setColor(currentLines.color);
    painter.setPen(pen);
    for(int i = 0; i < currentLines.lines.size(); i++)
    {
        painter.drawLine(currentLines.lines[i]);
    }
    painter.end();
}

void canvas::canvasReceived(QList<QList<QLine>> newCanvas)
{
    // lines = newCanvas;
    update();
}

QList<Serial::packet> canvas::serialize()
{
    QList<Serial::packet> packets;
    Serial::packet p;
    int x, y;
    
    if (toolType == "clear")
    {
        p.push_back(0);
        packets.append(p);
    }
    else if (toolType == "pen" ||
             toolType == "line" ||
             toolType == "rectangle")
    {
        for(int i = 0; i < currentLines.lines.size(); i++){

            if(p.size() % 256 == 0 )
            {
                packets.append(p);
                p = Serial::packet(0);
                
                p.push_back(1);
                p.push_back(currentLines.color.red()   & 0xFF);
                p.push_back(currentLines.color.green() & 0xFF);
                p.push_back(currentLines.color.blue()  & 0xFF);

                x = currentLines.lines[i].x1();
                y = currentLines.lines[i].y1();
                p.push_back((x >> 0) & 0xFF);
                p.push_back((x >> 8) & 0xFF);
                p.push_back((y >> 0) & 0xFF);
                p.push_back((y >> 8) & 0xFF);
            }
            int x = currentLines.lines[i].x2();
            int y = currentLines.lines[i].y2();
            p.push_back((x >> 0) & 0xFF);
            p.push_back((x >> 8) & 0xFF);
            p.push_back((y >> 0) & 0xFF);
            p.push_back((y >> 8) & 0xFF);
        }
        packets.append(p);
        packets.erase(packets.begin());
    }
    qDebug() << "serialized";
    return packets;
}

void canvas::deserialize(Serial::packet p)
{
    if (p.size() == 0)
        return;
    
    int command = p[0];
    qDebug() << command;
    if (command == 1)
    {
        if(p.size() % 4 != 0)
           return;
        LineGroup newGroup;
        for(int i = 4; i < p.size() - 4; i += 4)
        {
            int x1 = (int16_t) ((p[i+1] << 8) | p[i+0]);
            int y1 = (int16_t) ((p[i+3] << 8) | p[i+2]);
            int x2 = (int16_t) ((p[i+5] << 8) | p[i+4]);
            int y2 = (int16_t) ((p[i+7] << 8) | p[i+6]);
            QLine newLine(x1, y1, x2, y2);
            newGroup.lines.append(newLine);
        }
        newGroup.color = QColor(p[1], p[2], p[3]);
        lines.append(newGroup);
    }
    else if (command == 0)
    {
       qDebug() << "receive client cleared";
       lines.clear();
    }
    update();
}

void canvas::packetsReceived(QList<Serial::packet> newPackets)
{
    for(int i = 0; i < newPackets.size(); i++)
        deserialize(newPackets[i]);
}

void canvas::packetReceived(Serial* serial)
{
    while(serial->available())
        deserialize(serial->read());
}

void canvas::mousePressEvent(QMouseEvent *event)
{
    if (toolType == "clear")
    {
        lines.clear();
        update();
    }
}
