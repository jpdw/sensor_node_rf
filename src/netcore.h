/*
 * netcore.h
 *
 *  Created on: 28 Oct 2011
 *      Author: wilkinsj
 */

#ifndef NETCORE_H_
#define NETCORE_H_

//#include "EtherShield.h"

extern uint8_t netcore_setup();
extern uint8_t netcore_loop();
extern void DHCP_Test();
extern void netcore_ifconfig();
extern void test_send();
extern void do_dns();
extern void xap_send_hbeat();

void send_hbeat();
//void send_temperature(float* temps,uint8_t size);
void send_temperature(owd_t* onewire_devices,uint8_t size);

#endif /* NETCORE_H_ */
