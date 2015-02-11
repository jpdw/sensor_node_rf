/*
 * onewirecore.cpp
 *
 * Contains main 1-wire bus related functions coverin the following devices:
 * - 1-wire temperature (DS18B20)
 *
 *  Created on: 8 Nov 2011
 *      Author: wilkinsj
 */

// include hardware-specific definitions
#include "target.h"

#include <Arduino.h>
#include "HardwareSerial.h"
#include "DallasTemperature.h"
#include "OneWire.h"
#include "scheduler.h"
#include "onewirecore.h"

// Data wire is plugged into port 2 on the Arduino
#define ONE_WIRE_BUS HW_ONEWIRE_PIN

// Create global objects for onewire & dallas temperature sensors
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

owd_t * onewire_devices;	/* pointer to array of owd_t elements */

/* onewire_setup
 * - will be called during initialisation of the system
 * - function should completely initialise the 1-wire components
 */
void onewire_setup(bool wait_for_conversion){

    // start up the Dallas temperature sensor library and initiates a bus scan
    sensors.begin();
    // sets the dts library into non-blocking mode - calls for a conversion
    // will send a request to the device & then return -- we have to manage
    // a suitable delay and then retrieve the reading
	sensors.setWaitForConversion(wait_for_conversion);

	// create an array to hold address details for each device found
	uint8_t d=sensors.getDeviceCount();
	onewire_devices=(owd_t*)malloc(d*sizeof(owd_t));

	if(onewire_devices==NULL){
		Serial.println(F("WARN!"));
	}else{
		if(onewire_devices==0){
			Serial.println(F("NO 1WIRE"));
		}
	}
	// Get the 1-wire addresses of each device found on the 1-wire but
	uint8_t i=0;
	do {
		sensors.getAddress(onewire_devices[i].address, i);
	} while(++i<d);
}


uint8_t onewire_device_count(){
	return(sensors.getDeviceCount());
}

void onewire_request_temps(){
	// Send the command to get temperatures
	sensors.requestTemperatures();
	// enable the scheduler to raise an event when request should be ready to read
	scheduler_enable(SCHEDULER_ONEWIREREQ);
}

/*
 * onewire_read_temp
 * - will be called by the callback triggered by the ONEWIREREQ scheduler event
 */
//void onewire_read_temps(float *temps){
void onewire_read_temps(){
	//float t;
	for(uint8_t i=0;i<sensors.getDeviceCount();i++){
		//t=sensors.getTempCByIndex(i);
		onewire_devices[i].most_recent=sensors.getTempCByIndex(i);
		//temps[i]=t;
		//Serial.print(F("Temp="));
		//Serial.println(t);

	}
}

void onewire_debug(){
  sensors.requestTemperatures(); // Send the command to get temperatures
  Serial.println(F("DONE"));

  Serial.print(F("Temperature for Device 1 is: "));
  Serial.print(sensors.getTempCByIndex(0)); // Why "byIndex"?
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}

void onewire_addresses(){
	Serial.println(F("1Wire:"));
	for(uint8_t i=0;i<sensors.getDeviceCount();i++){
		Serial.print(i);
		Serial.print(F(" "));
		printAddress(onewire_devices[i].address);
		Serial.print(F(" "));
		Serial.println(onewire_devices[i].most_recent);
	}
}

float onewire_mostrecentvalue(int index){
    if(index >= sensors.getDeviceCount()){
        return 0;
    }else{
        return(onewire_devices[index].most_recent);
    }
}
