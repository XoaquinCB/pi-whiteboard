#include "serial.hpp"

int main()
{
    Serial serial(3, 21, 1000);
    
    Serial::packet p;
    p.push_back(10);
    p.push_back(53);
    p.push_back(125);
    serial.write(p);
    
    return 0;
}
