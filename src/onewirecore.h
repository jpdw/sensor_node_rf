/*
 * onewirecore.h
 *
 *  Created on: 8 Nov 2011
 *      Author: wilkinsj
 */

#ifndef ONEWIRECORE_H_
#define ONEWIRECORE_H_

struct owd_t {
	uint8_t address[8];		/* 8 byte address */
	uint8_t str_id;			/* # of name in eeprom */
	float most_recent;		/* most recent reading */
};

extern void onewire_setup();
extern uint8_t onewire_device_count();
extern void onewire_request_temps();
//extern void onewire_read_temps(float *);
extern void onewire_read_temps();
extern void onewire_addresses();

extern owd_t * onewire_devices;

extern void onewire_debug();
extern float onewire_mostrecentvalue(int);

#endif /* ONEWIRECORE_H_ */
