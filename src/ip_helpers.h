/*
 * tcpip_helpers.h
 *
 *  Created on: 8 Jan 2012
 *      Author: wilkinsj
 */

#ifndef IP_HELPERS_H_
#define IP_HELPERS_H_


//void iphelpers_set_broadcast(uint8_t *buff_bcast, uint8_t *ip_addr, uint8_t *mask);
void iphelpers_set_mymask(uint8_t *buff_mymask);
void iphelpers_set_mymask_E(byte address);
void iphelpers_get_ip_from_E(byte address, uint8_t *buff_dst);
uint8_t* iphelpers_ethercard_udpOffset();
void iphelpers_print_octets(uint8_t *buf,uint8_t qty,uint8_t base=DEC,bool nl=true);


#endif /* IP_HELPERS_H_ */
