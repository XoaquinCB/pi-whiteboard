#include <QApplication>
#include <QDebug>
#include <QObject>

#include <pthread.h>
#include <wiringPi.h>

#include "serial.hpp"
#include <QList>

#include <iostream>

#include "window.h"
#include "ui_window.h"

void* worker(void* thread_id)
{
    long tid = (long)thread_id;
    // do something....
    qDebug() << "Worker thread " << tid << "started.";

    // end thread
    pthread_exit(NULL);
}

int main(int argc, char *argv[])
{
    // setup GPIO interface - uncomment when needed
    // needs to run with root via sudo in terminal.
    // wiringPiSetup();
    // pinMode (0, OUTPUT);
    
    int pin_scl = 0;
    int pin_sda = 1;
    
    if (argc == 3)
    {
        char *arg1 = argv[1];
        char *arg2 = argv[2];
        
        pin_scl = atoi(arg1);
        pin_sda = atoi(arg2);
        
        if (pin_scl == pin_sda)
        {
            std::cout << "SCL and SDA pins cannot be the same." << std::endl;
            exit(1);
        }
    }
    
    std::cout << "SCL pin: " << pin_scl << ", SDA pin: " << pin_sda << std::endl;

    // setup Qt GUI
    QApplication a(argc, argv);
    Window window;
    Serial serial(pin_scl, pin_sda);
    window.show();

    QObject::connect(window.ui->centralWidget, &canvas::sendPacket, &serial, &Serial::write);
    QObject::connect(&serial, &Serial::packet_received, window.ui->centralWidget, &canvas::packetReceived);

    // starting worker thread(s)
    int rc;
    pthread_t worker_thread;
    rc = pthread_create(&worker_thread, NULL, worker, (void*)1);
    if (rc) {
        qDebug() << "Unable to start worker thread.";
        exit(1);
    }

    // start window event loop
    qDebug() << "Starting event loop...";
    int ret = a.exec();
    qDebug() << "Event loop stopped.";

    // cleanup pthreads
    pthread_exit(NULL);

    // exit
    return ret;
}
