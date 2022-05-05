#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
    
class Serial
{
public:
    typedef std::vector<unsigned char> packet;
    
    const int pin_scl, pin_sda;
    const int bitrate;
    
    /**
     * Constructor, specifying the SCL and SDA pins, and target bitrate.
     */
    Serial(int pin_scl, int pin_sda, int bitrate = 10000);
    
    /**
     * Waits for any ongoing transmission to finish and then destroys the object.
     */
    ~Serial();
    
    /**
     * Checks if a packet is valid and puts it on the transmit buffer if it is.
     * Returns whether the packet is valid.
     */
    bool write(packet bytes);
    
    /**
     * Returns the next packet in the receive buffer, or an empty packet if the
     * buffer is empty.
     */
    packet read();
    
    /**
     * Returns the number of packets available in the receive buffer.
     */
    std::size_t available();
    
private:
    std::thread *pin_thread;
    std::recursive_mutex mtx;
    
    bool finish;
    std::queue<packet> tx_buffer, rx_buffer;
    unsigned int bit_pos;
    unsigned int byte_pos;
    bool tx_bit_val;
    unsigned char rx_byte;
    packet rx_packet;
    
    enum { IDLE, TX, RX } state;
    
    static void pin_thread_process(Serial *serial);
    
    static void isr_scl_rise(Serial &serial);
    static void isr_scl_fall(Serial &serial);
    static void isr_sda_rise(Serial &serial);
    static void isr_sda_fall(Serial &serial);
    
    static void clock_pulse(Serial &serial);
    static void trigger_tx(Serial &serial);
};

#endif /* SERIAL_HPP */
