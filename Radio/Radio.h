#ifndef Radio_h
#define Radio_h

// Required to use Arduino methods
#if ARDUINO >= 100
    #include "Arduino.h"
#else
    #include "WProgram.h"
#endif

// Some constants
#define RX_LED 34
#define TX_LED 35
#define RF_BUFFER_SIZE 127

class Radio {
    public:
        Radio(int bChannel);
        
        void rfFlush();
        void rfPrint(String toPrint);
        void rfWrite(uint8_t b);
        void rfWrite(uint8_t* b, uint8_t length);
        unsigned int rfAvailable();
        char rfRead();
        char rfRead(uint8_t* buf, uint8_t len);

        struct ringBuffer {
            unsigned char buffer[RF_BUFFER_SIZE];
            volatile unsigned int head;
            volatile unsigned int tail;
        } radioRXBuffer;

        uint8_t rssiRaw;

    private:
        int channel;

        uint8_t rfBegin();
        void setExternRadio();
}; 

#endif