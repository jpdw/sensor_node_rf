/*
 * netcore.cpp
 *
 *  Created on: 27 Oct 2011
 *      Author: wilkinsj
 *
 *  Module provides core networking functions needed by the parent app
 *  - designed to build a module or library to be linked into other apps eventually
 *  - if running on Nanode, will use installed MAC
 *  - will do DHCP on initialisation
 *  - provide a queryable interface
 *  - limitations
 *    - will not perform dhcp renewals
 *    - does not handle lack of MAC on non-Nanode boards
 *    - has no capability for static addressing
 *    - has to be included (compiled) in client app - is not a library (yet)
 */


#include "target.h"
#include "eeprom_map.h"
#include <Arduino.h>
#include <EtherCard.h>
//#include "NanodeMAC.h"
#include "ip_helpers.h"

#include "memoryfree.h"
#include "strings.h"
#include "onewirecore.h"
#include "netcore.h"

typedef uint8_t DeviceAddress[8];
extern owd_t * onewire_devices;
/*
 * Define constants for netcore module
 */
#define DHCPLED 6		// digital o/p? (6 on nanode, 9 on arduino?)
#define BUFFER_SIZE 400 // ethercard buffer size constant (used to set Ethernet::buffer); has varied between 350 & 500

/*
 * Define variables used by module
 */
byte Ethernet::buffer[BUFFER_SIZE];	// ethercard module's buffer
static uint8_t fNetcore_0;			// module flag byte
// b7 =                          b3 = DHCP Enabled
// b6 =                          b2 = network module state b2
// b5 =                          b1 = network module state b1
// b4 =                          b0 = network module state b0
Stash stash;

/*==========================================================================
 * xAP
 *==========================================================================
 */

/*
 * xap_send_hbeat
 *
 * Send simple xAP format heartbeat over UDP periodically
 */
void xap_send_hbeat(){
	// Create ram-based strings for the two identifiers referenced
	// in the template.  These need to be passed to stash::prepare as ram-based
	// strings as the template token is $s.  So if the IDs *should* come from
	// Flash or EEPROM, they'll need to be memcpy'd to local ram vars first (here)
	const char * xAP_UID = "FF.1101:0000";
	const char * xAP_source = "be-tai.nanode.test";
	byte sd = stash.create();
	stash.print(F("60"));
	stash.save();
	Stash::prepare(xap_template,xAP_UID,xAP_source,sd);
	//byte ret=ether.udpSend(49152,ether.mybcast,3639);
}

/*=========================================================================
 * Web based calls
 *=========================================================================
 */

/*
 * Send a web-based heartbeat to the same server as the temp submits
 */
void send_hbeat(){
	byte sd = stash.create();
	stash.print(F("free="));
	stash.print(free());
	stash.print(F("&uptime="));
	stash.print(millis());
	stash.print(F("&stashfree="));
	stash.print(Stash::freeCount());
	stash.save();

	Stash::prepare(post_header,
		uri_hbeat,dns_host,content_type_urlencoded,stash.size(),sd);

	// send the packet - this also releases all stash buffers once done
	byte ret=ether.tcpSend();

}

void send_temperature(owd_t* onewire_devices,uint8_t size){
	static uint16_t seq=0;
	float a;
	int ia;
	char * str;
	uint8_t factor=100;

	byte sd = stash.create();
	stash.print(F("seq="));
	stash.print(seq);
	stash.print(F("&cnt="));
	stash.print(size);

	/*
	 * Build the content of the post request
	 * Containing:
	 * - index (ix)
	 * - sensor id (id)
	 * - value (vl)
	 * - factor [of value] (fc)
	 */
	for(uint8_t i=0;i<size;i++){
		a=factor*(onewire_devices[i].most_recent);
		//a=factor*(*(temps+i));
		ia=(int)a;
		str=(char*)onewire_devices[i].address;
		stash.print(F("&ix")); /* index */
		stash.print(i);
		stash.print(F("="));
		for (uint8_t j = 0; j < 8; j++){
		    // zero pad the address if necessary
		    if (onewire_devices[i].address[j] < 16) stash.print("0");
		    stash.print(onewire_devices[i].address[j], HEX);
		}
		stash.print(F("%3B"));
		stash.print(ia);	/* value */
		stash.print(F("%3B"));
		stash.print(factor); /* factor */
	}
	stash.save();

	Stash::prepare(post_header,
		uri_webapi,dns_host,content_type_urlencoded,stash.size(),sd);

	// send the packet - this also releases all stash buffers once done
	byte ret=ether.tcpSend();

	// Increment application sequence count
	seq++;
}

void netcore_ifconfig(){
	// Display the dhcp configuration:
	Serial.println(F("Network Configuration"));
	Serial.print(F(" Network Address : "));
	iphelpers_print_octets(ether.mymac,6,HEX,true);
	Serial.print(F(" DHCP Enabled    : "));
	if(fNetcore_0 & 0b00001000){
		Serial.println(F("Yes"));
	}else{
		Serial.println(F("No"));
	}
	Serial.print(F(" IPv4 Address    : "));
	iphelpers_print_octets(ether.myip,4,DEC,true);
	//Serial.print(F(" Subnet Netmask  : "));
	//iphelpers_print_octets(ether.mymask,4,DEC,true);
	Serial.print(F(" Detault Gateway : "));
	iphelpers_print_octets(ether.gwip,4,DEC,true);
	Serial.print(F(" DNS Server      : "));
	iphelpers_print_octets(ether.dnsip,4,DEC,true);
	//Serial.print(F(" (B/Cast Addr    : "));
	//iphelpers_print_octets(ether.mybcast,4,DEC,true);
}

uint8_t netcore_configure_static(){
	// Start static configuration based on EEPROM values
	iphelpers_get_ip_from_E(EVM_STATIC_IP1, ether.myip);
	// This might cause issues... the setStatic call does something
	// in addition to just putting the GWIP into the ether object
	iphelpers_get_ip_from_E(EVM_STATIC_GWIP1, ether.gwip);
	//setGwIp(gw_ip);
	//iphelpers_get_ip_from_E(EVM_STATIC_DNSIP1, ether.dnsip);
	iphelpers_set_mymask_E(EVM_STATIC_MASK1);
	// Call the function in EtherCard to set the broadcast based on IP & mask;

	// the following was a function I think I added to derive the broadcast address
	// from the IP and netmask
	//ether.setBroadcast();
	return(true);
}

uint8_t netcore_dns_preresolve(){
	// DNS Setup
	ether.packetLoop(ether.packetReceive());

	Serial.print(F("DNS "));

	// before even trying to do a DNS resolve, check we know a DNS server...!
	if(ether.dnsip[0]==0){
		// no dns server
		Serial.println(F("no server"));
		return(false);
	}

	Serial.print("wait 10s.... ");
	long to=millis()+10000;
	do{
		ether.packetLoop(ether.packetReceive());
	} while (millis()<to);
	Serial.println("done");

	if (!ether.dnsLookup(dns_host)){
		Serial.println(F("failed"));
		return(false);
	}else{
		Serial.println(F("succeeded"));
		return(true);
	}
}

/*
 * Setup the ethernet interface hardware & software registers
 *
 * Return true if OK, false if unsuccessful
 */
uint8_t netcore_setup(){
	/*
	 * Initialise the ethernet interface
	 */

	// initialise module flag field
	fNetcore_0=0;

	// Get MAC address from the underside of the board
	uint8_t mymac[6] = { 0B00000010,0x01,0x01,0x00,0x00,0x01 }; // def=02:00:00:00:00:01
	//NanodeMAC mac( mymac );

	// Configure nanode hardware
	pinMode( DHCPLED, OUTPUT);
	digitalWrite( DHCPLED, LOW);
	pinMode( 8, OUTPUT);
	digitalWrite( 8, LOW);

	// initialize enc28j60
	if (ether.begin(sizeof Ethernet::buffer, mymac) == 0) {
		fNetcore_0=6 ; // 0b110 rotate towards msb by 5 bits
		Serial.println(F("Failed to access Ethernet controller"));
		// unable to init netcore further, so return(false);
		return(false);
	}

	// determine if board should be in static or dhcp mode
	// and set a module-level local to store this for later reference
//	uint8_t evm_netflags=EEPROM.read(EVM_NETFLAGS);
	uint8_t bitfield=(1 << EVM_NETFLAGS_BF_DHCP);
	if((evm_netflags & bitfield )){
		Serial.print(F("DHCP"));
		// set state bits in flag byte to indicate DHCP attempt state required
		fNetcore_0=(fNetcore_0 & 248) | 1; // 0b001 rotate towards msb by 5 bits
		bitSet(fNetcore_0,3);	// set bit 3 in indicate DHCP
	}else{
		Serial.print(F("Static"));
		// set state bits in flag byte to indicate static IP state required
		fNetcore_0=(fNetcore_0 & 248) | 2; // 0b010 rotate towards msb by 5 bits
		bitClear(fNetcore_0,3);	// reset bit 3 in indicate static
	}
	Serial.println(F(" Mode"));

	//// Display mac address
	//iphelpers_print_octets(mymac,6,HEX);

	// return success
	return(true);
}


/*
 * netcore_loop
 *
 * - short non-blocking function for running network tasks
 * - will be called by the main loop
 * - local flag variable contains module's state in b2, b1, b0
 * if used to maintain network module's state
 *   fNetcore_0: [210]
 *    - 000 (0) = not initialised (about to init HW)
 *    - 001 (1) = DHCP attempt (acc counts failures)
 *    - 010 (2) = Static attempt
 *    - 011 (3) = DNS attempt
 *    - 100 (4) = No DNS or DNS failed - unable to proceed
 *    - 101 (5) = (spare)
 *    - 110 (6) = Unspecified error (hardware init issue)
 *    - 111 (7) = Operational
 *
 *
 * DHCP States from net.h
 * DHCP_STATE_INIT 0
 * DHCP_STATE_DISCOVER 1
 * DHCP_STATE_OFFER 2
 * DHCP_STATE_REQUEST 3
 * DHCP_STATE_ACK 4
 * DHCP_STATE_OK 5
 * DHCP_STATE_RENEW 6
 */

uint8_t netcore_loop(){
	static uint8_t loop_acc_a=0;

	switch(fNetcore_0 & 7){ // 7 is bitfield to mask off all OTHER THAN b2,b1,b0
		case 1:{ // dhcp config required
			Serial.println(F("Starting DHCP..."));
			if (!ether.dhcpSetup()) {
				Serial.println(F("  ..DHCP failed"));
				// increment a failure counter
				loop_acc_a++;
				// if we've failed too many times, set to static
				if(loop_acc_a>=2){
					// switch flags to static mode
					fNetcore_0=(fNetcore_0 & 248) | 2;
				}
			}else{
				Serial.println(F("  ..completed"));
				//ether.setBroadcast();
				//iphelpers_set_broadcast(broadcast, ether.myip, ether.mymask);
				netcore_ifconfig();
				// set dns as net stage
				fNetcore_0 =(fNetcore_0 & 248) | 3 ;
			}
			break;
		}
		case 2:{ // static config required
			if(netcore_configure_static()){
				//iphelpers_set_broadcast(broadcast, ether.myip, ether.mymask);
				netcore_ifconfig();
				fNetcore_0 =(fNetcore_0 & 248) |  3;
			}else{
				fNetcore_0 =(fNetcore_0 & 248) |  6;
			}
			break;
		}
		case 3:{ // DNS required
			if(netcore_dns_preresolve()){
				fNetcore_0 =(fNetcore_0 & 248) |  7;
			}else{
				fNetcore_0 = (fNetcore_0 & 248) | 4;
			}
			break;
		}
		case 7:{
			// normal operation - handle network traffic from here...
			word len = ether.packetReceive();
			word pos = ether.packetLoop(len);

			if(pos!=0){

				// display the buffer
				word p=pos;
				do{
					Serial.print((char)Ethernet::buffer[p++]);
				}while(Ethernet::buffer[p]!=0 && p<BUFFER_SIZE);

				// Is this a GET?
				if (strncmp("GET ",(char *)&(EtherCard::buffer[pos]),4)==0){
					Serial.println("GET request");

					if (strncmp("/api/",(char *)&(EtherCard::buffer[pos+4]),5)==0){
						Serial.println("Supported path");


					}
				}

				// Is this a PUT?
				if (strncmp("POST",(char *)&(EtherCard::buffer[pos]),4)==0){
					Serial.println("POST not yet supported");
				}
			}

			break;
		}
	}
	return(1);
}

