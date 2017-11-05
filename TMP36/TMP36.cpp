/*
TMP36.cpp - Library to obtain values from a TMP36Z Temperature Sensor
Created by Colin "MrSwirlyEyes"
For Project in a Box, University of California, San Diego.
*/

#include "TMP36.h"



// CONSTRUCTOR

TMP36::TMP36(byte _pin,float _vcc) : _pin(_pin),_vcc(_vcc),_temperature_C(0.0),_temperature_F(0.0) {
	analogReference(EXTERNAL);
}



// PUBLIC FUNCTIONS

void TMP36::read() {
	float voltage = ((analogRead(this->_pin) * this->_vcc) / 1024.0);
	this->_temperature_C = (voltage - 0.5) * 100;
	this->_temperature_F = (this->_temperature_C * (9.0 / 5.0)) + 32.0;	
}

void TMP36::demux_read(int voltage) {
	float _voltage = ((voltage * this->_vcc) / 1024.0);
	this->_temperature_C = (_voltage - 0.5) * 100;
	this->_temperature_F = (this->_temperature_C * (9.0 / 5.0)) + 32.0;	
}

float TMP36::get_temperature_C() {
	return this->_temperature_C;
}

float TMP36::get_temperature_F() {
	return this->_temperature_F;
}