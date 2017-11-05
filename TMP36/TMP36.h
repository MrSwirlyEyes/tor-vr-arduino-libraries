/*
TMP36.h - Library to obtain values from a TMP36Z Temperature Sensor
Created by Colin "MrSwirlyEyes"
For Project in a Box, University of California, San Diego.
*/

#ifndef TMP36_h
#define TMP36_h

#include <Arduino.h>

class TMP36 {
	public:
        TMP36(byte _pin,float _vcc);

        void read();
        void demux_read(int voltage);
        float get_temperature_C();
        float get_temperature_F();

    private:
        byte _pin;    
        float _vcc;
        float _temperature_C;
        float _temperature_F;
};

#endif