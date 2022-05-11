#ifndef SERIAL_TEST_HPP
#define SERIAL_TEST_HPP

#include "serial.hpp"
#include <iostream>
#include <QObject>

extern std::string convert_packet(Serial::packet p);

class SerialTest : public QObject
{
    Q_OBJECT

public slots:
    void packet_received(Serial *serial)
    {
        while (serial->available())
            std::cout << "Addr " << serial << ": " << convert_packet(serial->read()) << std::endl;
    }
};

#endif /* SERIAL_TEST_HPP */
