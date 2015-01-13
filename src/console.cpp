/*
 * console.cpp
 *
 *  Created on: 29 Oct 2011
 *      Author: wilkinsj
 *
 * control console class
 *
 * this is revised since the power_control code in order to attempt to
 * make the code & data impact of this class much reduced from the original
 * main changes expected
 * - console commands will be stored in a struct/list rather than array of objects
 * - lack of Arduino String class
 * -
 */

// includes
#include "target.h"
#include <Arduino.h>
#include "HardwareSerial.h"
#include "console.h"

// ?? move elsewhere??
void(* resetFunc)(void) = 0; //declare reset function @ address 0
void reboot(){ resetFunc();} //function to call the above... hacky, can prob be improved

/*
 * Primatives for functions not needed outside of this module
 */
void console_cmd_help();

/*
 * Define constants
 */
#define CONSOLE_CMD_STRMAX 4	// define the max length of a console command
#define FIXED_COMMAND_SLOTS 12	// number of console command slots (max)
#define CONSOLE_RX_BUFR 20		// byte size of console incoming command buffer

/*
 * Define a structure for each individual command that will be available.  This has been kept in the .cpp
 * file instead of the .h deliberataly, so that modules which use the console (include the .h) don't have
 * to have - and dont need - knowledge of the underlying struct
 */
struct command_t {
	char command[CONSOLE_CMD_STRMAX+1];			// fixed command string (can be less if null terminated)
	void (*callback)();							// address to call (if cmdid == 0)
};

/*
 * Module variables (global scope)
 */
command_t commands[FIXED_COMMAND_SLOTS];
unsigned char console_rxinc[CONSOLE_RX_BUFR];	// buffer for incoming command line
uint8_t console_rxinc_pos;   					// position (zero based) for *next* write
boolean console_rx_complete;					// signals a completed command (handle_received_command needs to be called)
uint8_t console_commands=0;						// number of defined commands

/*
 * console_setup
 * - initial setup of module, will be called during setup()
 * - register commands that are "built in" to the console module
 */
void console_setup(){
	console_register("help",&console_cmd_help);
	console_register("?",&console_cmd_help);
	console_register("rebt",&reboot);
}

/*
 * console_register
 * - allows new commands/functions to be added to the console
 * - cannot grow beyond FIXED_COMMAND_SLOTS but this fn does not validate... beware!
 */
void console_register(const char* command, void(*cbfunction)()){
	strncpy(commands[console_commands].command,command,CONSOLE_CMD_STRMAX);
	commands[console_commands].command[4]='\0';
	commands[console_commands].callback=cbfunction;
	console_commands++;
}


void console_handle_serial_reception(){
	char latest=0;
	boolean exit_now=false;

	// check for anything in the serial buffer
	while ((Serial.available() && !exit_now)){
		// copy everything in buffer upto end/newline
		latest=(char)Serial.read();

		switch((uint8_t)latest){
			case 13:{
				console_rxinc[console_rxinc_pos]='\0';	// null terminate
				Serial.print(F("\r\n"));
				exit_now=true;
				break;
			}
			case 127:{
				//Serial.print("{bs}");
				if(console_rxinc_pos){
					console_rxinc[--console_rxinc_pos]='\0';

					// echo character back
					Serial.print((char)8);

				}else{
					Serial.print(F("\a"));	// BELL
				}
				break;
			}
			default:{
				// determine if this is special character of some sort
				if(latest<32){
					Serial.println((int)latest);
				}

				// standard character - store in incoming buffer
				console_rxinc[console_rxinc_pos++]=latest;

				// echo character back
				Serial.print(latest);
				break;
			}
		}
	} //while

	// determine how to handle a completed command line
	if(latest=='\r'){
		// raise semaphore
		console_rx_complete=true;
	}
}

void console_handle_received_command(){
	boolean bExit=false;

	if(!console_rx_complete){
		return;
	}

	// handle blank entry (ie just pressed return)
	if(console_rxinc_pos){

		// iterate through the commands[] till we find a match
		uint8_t i=0;
		while(i<FIXED_COMMAND_SLOTS && !bExit){
			if(commands[i].command[0]!=0){
				if(!strncmp((const char*)console_rxinc,commands[i].command,CONSOLE_CMD_STRMAX)){
					// match
					bExit=true;
				}else{
					i++;
				}
			}else{
				i++;
			}
		}

		if(bExit){
			// lets try to run the callback :-)
			commands[i].callback();
		}else{
			// command had been entered but was not recognised
			Serial.println(F("Unknown (? for help)"));
		}
	}

	// reset flag & buffer
	console_rx_complete=false;
	console_rxinc_pos=0;

	// display prompt for next command entry
	console_send_prompt();
}

void console_start(){
	console_send_prompt();
}

void console_send_prompt(){
	Serial.print(F("=>"));
}

void console_cmd_help(){
	uint8_t i=0;
	Serial.println(F("Commands available:"));
	do {
		if(commands[i].command[0]!=0){
			Serial.println(commands[i].command);
		}
		i++;
	}while(i<FIXED_COMMAND_SLOTS);
}
