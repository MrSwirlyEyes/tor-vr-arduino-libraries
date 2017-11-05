/*
TMP36.cpp - Library to obtain values from a TMP36Z Temperature Sensor
Created by Colin "MrSwirlyEyes"
For Project in a Box, University of California, San Diego.
*/

#include "CD74HC4067.h"



// CONSTRUCTOR

CD74HC4067::CD74HC4067(byte _s0,byte _s1,byte _s2,byte _s3,byte _sig_pin) : _s0(_s0),_s1(_s1),_s2(_s2),_s3(_s3),_sig_pin(_sig_pin) {
	this->_ctrl_pins_arr = (CTRL_PINS) {this->_s0,this->_s1,this->_s2,this->_s3};

	for(int i = 0 ; i < NUM_SELECTS ; i++) {
		pinMode(this->_ctrl_pins_arr._ctrl_pins[i],OUTPUT);
		digitalWrite(this->_ctrl_pins_arr._ctrl_pins[i],LOW);	
	}
	
    analogReference(EXTERNAL);
}



// PUBLIC FUNCTIONS

int CD74HC4067::read_channel(byte ch) {
	for(int i = 0 ; i < NUM_SELECTS ; i++)
		digitalWrite(this->_ctrl_pins_arr._ctrl_pins[i],this->_mux_channel[ch][i]);

	return analogRead(this->_sig_pin);
}