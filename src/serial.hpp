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
    /**
     * Datatype representing a packet of data as a list of bytes.
     */
    typedef std::vector<unsigned char> packet;
    
    /**
     * Target bitrate in Hz.
     */
    static const int bitrate = 1000;
    
    /**
     * SCL (clock) and SDA (data) pins.
     */
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
     * Blocks until data is available in the receive buffer, or until the timeout,
     * and returns the number of packets available.
     * Use a negative timeout to wait indefinitely.
     */
    std::size_t wait_available(long timeout_micros = -1);
    
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
    // Any access to variables in this class should lock this mutex.
    std::mutex mtx;
    
    // Variables for assisting with stopping the instance.
    bool finish = false;
    bool is_stopped = false;
    unsigned int thread_count = 0;
    std::condition_variable_any stop_condition;
    
    // Transmit and receive buffers.
    std::queue<packet> tx_buffer, rx_buffer;
    
    // Variables for keeping track of a transmission/reception.
    unsigned int byte_pos, bit_pos;
    bool tx_bit_val;
    unsigned char rx_byte;
    packet rx_packet;
    
    std::condition_variable_any available_condition;
    
    enum { IDLE, TX, RX } state = IDLE;
    
    // Starts a thread in charge of checking the pin values and dispatching the pin change interrupts.
    void pin_thread();
    
    // Iterrupt routines for pin changes.
    // 'mtx' must be locked before calling any of these.
    void isr_scl_rise();
    void isr_scl_fall();
    void isr_sda_rise();
    void isr_sda_fall();
    
    // Methods for triggering a transmission, and for generating a clock pulse.
    // 'mtx' must be locked before calling either of these.
    void trigger_tx();
    void clock_pulse();
};

#endif /* SERIAL_HPP */
