#ifndef SERIAL_HPP
#define SERIAL_HPP

#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
    
class Serial
{
public:
    typedef std::vector<unsigned char> packet;
    static const int bitrate = 1000;
    
    const int pin_scl, pin_sda;
    
    /**
     * Constructor, specifying the SCL and SDA pins.
     */
    Serial(int pin_scl, int pin_sda);
    
    /**
     * Calls the `stop` function and then cleans up the instance.
     */
    ~Serial();
    
    /**
     * Returns the next packet in the receive buffer, or an empty packet if the
     * buffer is empty.
     */
    packet read();
    
    /**
     * Same as 'read' but doesn't remove the packet from the buffer.
     */
    packet peek();
    
    /**
     * Checks if a packet is valid and puts it on the transmit buffer if it is.
     * Returns whether the packet is valid.
     */
    bool write(packet bytes);
    
    /**
     * Returns the number of packets available in the receive buffer.
     */
    std::size_t available();
    
    /**
     * Returns the number of packets remaining in the transmit buffer.
     */
    std::size_t remaining();
    
    /**
     * Blocks until data is available in the receive buffer, or until the timeout.
     * Use a negative timeout if you want to wait indefinitely.
     */
    bool wait_available(long timeout_micros = -1);
    
    /**
     * Blocks until any ongoing transmission is finished and then stops any further transmissions or receptions.
     * Can safely be called multiple times. Note that once stopped, the instance cannot be restarted.
     */
    void stop();
    
    /**
     * Returns whether the instance has been stopped.
     */
    bool stopped();
    
private:
    std::mutex mtx;
    std::condition_variable_any stop_condition;
    std::condition_variable_any available_condition;
    
    bool finish = false;
    bool is_stopped = false;
    unsigned int thread_count = 0;
    
    std::queue<packet> tx_buffer, rx_buffer;
    
    unsigned int byte_pos, bit_pos;
    bool tx_bit_val;
    unsigned char rx_byte;
    packet rx_packet;
    
    enum { IDLE, TX, RX } state = IDLE;
    
    void pin_thread_process();
    
    void isr_scl_rise();
    void isr_scl_fall();
    void isr_sda_rise();
    void isr_sda_fall();
    
    void trigger_tx();
    void clock_pulse();
};

#endif /* SERIAL_HPP */
