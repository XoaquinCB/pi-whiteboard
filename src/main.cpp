#include "serial.hpp"
#include "serial_test.hpp"
#include <wiringPi.h>
#include <iostream>
#include <unistd.h>
#include <string>

#include <QObject>
#include <QCoreApplication>

std::string convert_packet(Serial::packet p)
{
    std::string str = "";
    
    for (unsigned char c : p)
        str += std::to_string((unsigned int) c) + " ";
    
    return str;
}

int main(int argc, char** argv)
{
    QCoreApplication app(argc, argv);
    SerialTest test;
    
    // Pins 2 and 4, and 3 and 5 should be connected together.
    Serial serialA(2, 3);
    Serial serialB(4, 5);
    
    QObject::connect(&serialA, &Serial::packet_received, &test, &SerialTest::packet_received);
    QObject::connect(&serialB, &Serial::packet_received, &test, &SerialTest::packet_received);
    
    Serial::packet p1;
    p1.push_back(115);
    p1.push_back(53);
    p1.push_back(125);
    serialA.write(p1);
    
    Serial::packet p2;
    p2.push_back(115);
    p2.push_back(40);
    p2.push_back(84);
    serialB.write(p2);
    
    return app.exec();
}
