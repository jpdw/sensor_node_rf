/*
 * radio.h
 *
 *  Created on: 9 Jan 2015
 *      Author: wilkinsj
 */

#ifndef RADIO_H_
#define RADIO_H_

extern void cb_check_radio(void); // ISR
extern void radio_info(void);
extern void radio_details(void);
extern void radio_loop();  // every 2 seconds
extern void radio_setup(void);
extern bool radio_is_running(void);
extern unsigned long radio_payload_time(void);
extern float remote_temperature_value(void);
extern bool remote_has_temperature(void);
extern bool role_is_receiver(void);


extern bool radio_data_ready;   /* indicates radio data in payload from ISR */


struct AckPayload
{
  unsigned char command;
  unsigned char data[8];
};
extern AckPayload ack_payload;

#endif /* RADIO_H_ */
