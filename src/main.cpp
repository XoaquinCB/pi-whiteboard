#include "serial.hpp"
#include <wiringPi.h>
#include <iostream>
#include <unistd.h>
#include <string>

std::string convert_packet(Serial::packet p)
{
    std::string str = "";
    
    for (unsigned char c : p)
        str += std::to_string((unsigned int) c) + " ";
    
    return str;
}

int main()
{
    // Pins 2 and 4, and 3 and 5 should be connected together.
    Serial serialA(2, 3);
    Serial serialB(4, 5);
    
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
    
    if (serialA.wait_available())
         std::cout << "RX (A): " << convert_packet(serialA.read()) << std::endl;
    
    if (serialB.wait_available())
         std::cout << "RX (B): " << convert_packet(serialB.read()) << std::endl;
    
    return 0;
}
