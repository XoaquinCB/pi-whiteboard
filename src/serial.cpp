#include "serial.hpp"

#include <wiringPi.h>
#include <unistd.h>
#include <thread>
#include <cmath>
#include <chrono>
#include <future>

#define SET_PIN_LEVEL(pin, level) if (level) { pullUpDnControl(pin, PUD_UP); pinMode(pin, INPUT); } else { digitalWrite(pin, LOW); pinMode(pin, OUTPUT); }
#define GET_PIN_LEVEL(pin)        digitalRead(pin)

Serial::Serial(int pin_scl, int pin_sda, int bitrate) : pin_scl(pin_scl), pin_sda(pin_sda), bitrate(bitrate)
{
    mtx.lock();
    thread_count++;
    mtx.unlock();
    
    std::thread thr([this]{ pin_thread_process(*this); });
    thr.detach();
}

Serial::~Serial()
{
    stop();
}

Serial::packet Serial::read()
{
    packet p(0);
    mtx.lock();
    if (rx_buffer.size())
    {
        p = rx_buffer.front();
        rx_buffer.pop();
    }
    mtx.unlock();
    return p;
}

bool Serial::write(packet bytes)
{
    if (0 < bytes.size() && bytes.size() <= 256)
    {
        mtx.lock();
        tx_buffer.push(bytes);
        trigger_tx(*this);
        mtx.unlock();
        return true;
    }
    else
    {
        return false;
    }
}

std::size_t Serial::available()
{
    mtx.lock();
    std::size_t size = rx_buffer.size();
    mtx.unlock();
    return size;
}

std::size_t Serial::remaining()
{
    mtx.lock();
    std::size_t size = tx_buffer.size();
    mtx.unlock();
    return size;
}

void Serial::stop()
{
    mtx.lock();
    
    // Set flag to tell threads to finish:
    finish = true;
    
    // Wait for all threads to finish (count == 0):
    stop_condition.wait(mtx, [this]{ return thread_count == 0; });
    
    is_stopped = true;
    mtx.unlock();
}

bool Serial::stopped()
{
    mtx.lock();
    bool stopped = is_stopped;
    mtx.unlock();
    
    return stopped;
}

void Serial::pin_thread_process(Serial &serial)
{
    serial.mtx.lock();
    
    SET_PIN_LEVEL(serial.pin_sda, 1);
    SET_PIN_LEVEL(serial.pin_scl, 1);
    
    bool last_state_sda = GET_PIN_LEVEL(serial.pin_sda);
    bool last_state_scl = GET_PIN_LEVEL(serial.pin_scl);
    
    serial.mtx.unlock();
    
    while (true)
    {
        serial.mtx.lock();
        
        if (serial.finish && serial.state == IDLE)
            break;
        
        bool curr_state_sda = GET_PIN_LEVEL(serial.pin_sda);
        bool curr_state_scl = GET_PIN_LEVEL(serial.pin_scl);
        
        if (!last_state_sda && curr_state_sda) isr_sda_rise(serial);
        if (last_state_sda && !curr_state_sda) isr_sda_fall(serial);
        if (!last_state_scl && curr_state_scl) isr_scl_rise(serial);
        if (last_state_scl && !curr_state_scl) isr_scl_fall(serial);
        
        serial.mtx.unlock();
        
        last_state_sda = curr_state_sda;
        last_state_scl = curr_state_scl;
        
        usleep(10);
    }
    
    serial.thread_count--;
    serial.mtx.unlock();
    serial.stop_condition.notify_all();
}

void Serial::isr_scl_rise(Serial &serial)
{
    bool rx_bit_val = GET_PIN_LEVEL(serial.pin_sda);
    
    if (serial.state == TX)
    {
        // Check if last byte has been transmitted:
        if (serial.byte_pos > serial.tx_buffer.front().size())
        {
            // If so, generate a stop condition and return:
            SET_PIN_LEVEL(serial.pin_sda, 1);
            return;
        }
        
        // Check if the actual pin level is what we transmitted:
        if (rx_bit_val != serial.tx_bit_val)
        {
            // If not, arbitartion was lost, switch to RX mode:
            serial.state = RX;
        }
        else
        {
            // If so, generate the next clock pulse:
            clock_pulse(serial);
        }
    }
    
    // In both RX and TX mode, push the bits on SDA into an rx_packet.
    // If in TX mode, arbitration could force the device into RX mode
    // and we don't want to miss all the data beforehand.
    
    // Add bit to byte:
    serial.rx_byte |= rx_bit_val << (serial.bit_pos);
    serial.bit_pos++;
    
    // If byte is filled, add byte to packet and start new byte:
    if (serial.bit_pos == 8)
    {
        // Ignore first byte (length byte).
        if (serial.byte_pos > 0)
            serial.rx_packet.push_back(serial.rx_byte);
        
        serial.bit_pos = 0;
        serial.byte_pos++;
        serial.rx_byte = 0;
    }
}

void Serial::isr_scl_fall(Serial &serial)
{
    // If TX mode, write next bit to SDA:
    if (serial.state == TX)
    {
        packet &tx_packet = serial.tx_buffer.front();
        unsigned char tx_byte;
        
        if (serial.byte_pos == 0)
            tx_byte = tx_packet.size(); // first byte is the length byte.
        else if (serial.byte_pos > tx_packet.size())
            tx_byte = 0; // after last byte, set up the stop bit by setting SDA low
        else
            tx_byte = tx_packet.at(serial.byte_pos - 1); // else get the data byte
        
        serial.tx_bit_val = tx_byte & (1 << serial.bit_pos);
        
        SET_PIN_LEVEL(serial.pin_sda, serial.tx_bit_val);
    }
}

void Serial::isr_sda_rise(Serial &serial)
{
    // Check if SCL is high -> stop condition:
    if (GET_PIN_LEVEL(serial.pin_scl))
    {
        if (serial.state == RX)
            serial.rx_buffer.push(serial.rx_packet);
        else if (serial.state == TX)
            serial.tx_buffer.pop();
        
        serial.state = IDLE;
        
        // Trigger the next transmission:
        trigger_tx(serial);
    }
}

void Serial::isr_sda_fall(Serial &serial)
{
    // Check if SCL is high -> start condition:
    if (GET_PIN_LEVEL(serial.pin_scl))
    {
        serial.bit_pos = 0;
        serial.rx_byte = 0;
        serial.byte_pos = 0;
        serial.rx_packet = packet();
        
        if (serial.state == IDLE)
        {
            serial.state = RX;
        }
    }
}

// Asynchronously waits for half a clock cycle and then starts a new transmission if it can.
void Serial::trigger_tx(Serial &serial)
{
    if (serial.is_stopped)
        return;
    
    serial.thread_count++;
    
    std::thread thr([&serial] () {
        // Delay for half a clock cycle:
        usleep(1000000 / serial.bitrate / 2);
        
        serial.mtx.lock();
        
        // Check the finish flag and that the state is IDLE:
        if (!serial.finish && serial.state == IDLE)
        {
            // If there's a packet to transmit, start a new transmission:
            if (serial.tx_buffer.size())
            {
                SET_PIN_LEVEL(serial.pin_sda, 0); // start condition
                serial.state = TX;
                clock_pulse(serial);
            }
        }
        
        serial.thread_count--;
        serial.mtx.unlock();
        serial.stop_condition.notify_all();
    });
    
    thr.detach();
}

// Asynchronously generates a single clock pulse: SCL low and then high.
void Serial::clock_pulse(Serial &serial)
{
    serial.thread_count++;
    
    // Set SCL low and then high with the appropriate delays, in a separate thread:
    std::thread thr([&serial] () {
        usleep(1000000 / serial.bitrate / 2);
        
        serial.mtx.lock();
        SET_PIN_LEVEL(serial.pin_scl, 0);
        serial.mtx.unlock();
        
        usleep(1000000 / serial.bitrate / 2);
        
        serial.mtx.lock();
        SET_PIN_LEVEL(serial.pin_scl, 1);
        
        serial.thread_count--;
        serial.mtx.unlock();
        serial.stop_condition.notify_all();
    });
    
    thr.detach();
}
