/*
 * ip_helpers.cpp
 *
 *  Created on: 8 Jan 2012
 *      Author: wilkinsj
 */


#include "target.h"

// Mandatory includes
#if ARDUINO >= 100
	#include <Arduino.h>
#else
	#include <wiring.h>
#endif

#include <EtherCard.h>

#include "eeprom_map.h"
#include "ip_helpers.h"


/*
 * set_broadcast
 *
 * fill a 4-octet buffer with the broadcast address
 * derived from the given ip address and subnet mask
 */
/*
void iphelpers_set_broadcast(uint8_t *buff_bcast, uint8_t *ip_addr, uint8_t *mask){
	for(uint8_t p=0;p<4;p++){
		buff_bcast[p]=(ip_addr[p] & mask[p]) | ~ mask[p];
	}

	// glue as a temporary measure till broadcast code above moves to ethercard
	memcpy(EtherCard::mybcast,buff_bcast,4);
}
*/

void iphelpers_get_ip_from_E(byte address, uint8_t *buff_dst){
	for( uint8_t i=0; i<4; i++ ) {
		//buff_dst[i]=EEPROM.read(address++);
	}
}

/*
 * set_mymask & set_mymask_E
 *
 * fill a 4-octet buffer in the ethercard with passed-in
 * subnet mask - whilst this is copied in from DHCP responses
 * there for does not seem to be a direct funtion to do this for
 * statically assigned (and the examples dont seem to do it either)
 *
 */
void iphelpers_set_mymask(uint8_t *buff_mymask){
	//ether.copyIp(ether.mymask,buff_mymask);
}
// as above but reading from eeprom memory
void iphelpers_set_mymask_E(byte address){
	uint8_t buff_mymask[4];
	iphelpers_get_ip_from_E(address,&buff_mymask[0]);

	// following line removed by JW 03/01/2015 to get this to compile
	//ether.copyIp(ether.mymask,buff_mymask);
}


/*
 * print_octets
 *
 * short function to print a MAC or IP Address either as hex or decimal
 */
void iphelpers_print_octets(uint8_t *buf,uint8_t qty,uint8_t base,bool nl){
	char delimiter=':';
	if(base==DEC){
		delimiter='.';
	}
	for( uint8_t i=0; i<qty; i++ ) {
		Serial.print( buf[i], base );
		if(i<(qty-1))
		{
			Serial.print(delimiter);
		}
	}
	if(nl){
		Serial.println();
	}
}
