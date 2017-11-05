/*
TMP36.h - Library to obtain values from a TMP36Z Temperature Sensor
Created by Colin "MrSwirlyEyes"
For Project in a Box, University of California, San Diego.
*/

#ifndef CD74HC4067_h
#define CD74HC4067_h

#include <Arduino.h>

#define NUM_SELECTS 4
#define NUM_CHANNELS 16

typedef struct {
    byte _ctrl_pins[NUM_SELECTS];
} CTRL_PINS;

class CD74HC4067 {
	public:
        CD74HC4067(byte _s0,byte _s1,byte _s2,byte _s3,byte _sig_pin);

        int read_channel(byte ch);

    private:
        byte _s0;
        byte _s1;
        byte _s2;
        byte _s3;
        byte _sig_pin;

        CTRL_PINS _ctrl_pins_arr;

        const bool _mux_channel[NUM_CHANNELS][NUM_SELECTS] = {
                                        {0,0,0,0}, //channel 0
                                        {1,0,0,0}, //channel 1
                                        {0,1,0,0}, //channel 2
                                        {1,1,0,0}, //channel 3
                                        {0,0,1,0}, //channel 4
                                        {1,0,1,0}, //channel 5
                                        {0,1,1,0}, //channel 6
                                        {1,1,1,0}, //channel 7
                                        {0,0,0,1}, //channel 8
                                        {1,0,0,1}, //channel 9
                                        {0,1,0,1}, //channel 10
                                        {1,1,0,1}, //channel 11
                                        {0,0,1,1}, //channel 12
                                        {1,0,1,1}, //channel 13
                                        {0,1,1,1}, //channel 14
                                        {1,1,1,1}  //channel 15
                                      };
};

#endif