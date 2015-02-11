/*
 * radio.cpp
 *
 *  Created on: 9 Jan 2015
 *      Author: wilkinsj
 */

// Additional includes for RF24
#include "nRF24L01.h"
#include "RF24.h"
#include "printf.h"
#include "avr/eeprom.h"
#include "eeprom_map.h"

// Get hardware pin definition for target board
#include "target.h"

#include "role.h"

extern float onewire_mostrecentvalue(int);
extern void cb_check_radio(void);

const char* role_friendly_name[] = { "invalid", "Sender", "Receiver"};  // The debug-friendly names of those roles


// external flag - whether to output debug info
extern bool mode_debug;

bool radio_data_ready;

// *     The other pin we are using is DIG2 (ATmega pin 4) for INT0 in the code lower down
// Set up nRF24L01+ radio on SPI bus plus EN/SS pins as defined for target board
RF24 radio(HW_SPI_EN_RF24,HW_RF24_DIRN_PIN);



// Start of more RF24 code

// Demonstrates another method of setting up the addresses
byte address[][5] = { 0xCC,0xCE,0xCC,0xCE,0xCC , 0xCE,0xCC,0xCE,0xCC,0xCE};

// Role management


static uint32_t message_count = 0;

struct Payload
{
  unsigned long time;     // arduino microsecond counter
  int num_colors;         // colour density factor in hsb-fadearound
  int animation_delay;    // delay factor  in hsb-fadearound
  float temperature;      // latest measured temperature
};
Payload payload;


/****************************************************************************
 *  RF24 Wireless code here
 ****************************************************************************/
// Copied from pingpair_irq "setup()"
// - removed lines that are already dealt with in my setup (start serial etc)


void radio_setup(){

    printf_begin();

    // Setup and configure rf radio
    radio.begin();
    // Get radio channel from eeprom if valid (or set eeprom to default)
    uint8_t rf_channel = eeprom_read_byte((unsigned char*)EVM_RF_CHNNL);
    if (rf_channel <=127){
        radio.setChannel(rf_channel);
    }else{
        eeprom_write_byte((unsigned char*)EVM_RF_CHNNL,0x4c);
    }

    //radio.setPALevel(RF24_PA_LOW);
    radio.enableAckPayload();           // We will be using the Ack Payload feature, so please enable it
    radio.enableDynamicPayloads();      // Ack payloads are dynamic payloads
                                        // Open pipes to other node for communication
    if ( role == role_sender ) {                      // This simple sketch opens a pipe on a single address for these two nodes to
        radio.openWritingPipe(address[0]);             // communicate back and forth.  One listens on it, the other talks to it.
        radio.openReadingPipe(1,address[1]);
    }else{
        radio.openWritingPipe(address[1]);
        radio.openReadingPipe(1,address[0]);
        radio.startListening();
        radio.writeAckPayload( 1, &message_count, sizeof(message_count) );  // Add an ack packet for the next time around.  This is a simple
        ++message_count;
    }

    delay(50);
    // Attach interrupt handler to interrupt #0
    attachInterrupt(INT_REF_RF24, cb_check_radio, LOW);

    // initialise payload
    payload.time = 0;
    payload.temperature = -999; // indicates empty (note - FIX THIS)

    // initialise flag to signal received data (used by receiver)
    radio_data_ready = false;

}


/*
 * radio is running
 *
 * Return true we have a functioning radio
 */
bool radio_is_running(){
    return(true);
}

unsigned long radio_payload_time(){
    return(payload.time);
}

bool role_is_receiver(void){
    if(role == role_receiver){
        return true;
    }else{
        return false;
    }
}
// return a 'true' if there is 'anything' in payload other than the init value
bool remote_has_temperature(void){
    return( (payload.temperature != -999) );
}
float remote_temperature_value(void){
    return(payload.temperature);
}

// Copied from pingpair_irq check_radio
// This is the pseudo ISR that is defined as the callback in the Arduino
// interrupt call in radio_setup

void cb_check_radio(void){

    bool tx,fail,rx;
    radio.whatHappened(tx,fail,rx);                     // What happened?

    if(mode_debug == true) {
        if ( tx ) {                                         // Have we successfully transmitted?
            if ( role == role_sender ){   printf("Send:OK\n\r"); }
            if ( role == role_receiver ){ printf("Ack Payload:Sent\n\r"); }
        }

        if ( fail ) {                                       // Have we failed to transmit?
            if ( role == role_sender ){   printf("Send:Failed\n\r");  }
            if ( role == role_receiver ){ printf("Ack Payload:Failed\n\r");  }
        }
    }

    if ( rx || radio.available()){                      // Did we receive a message?

        if ( role == role_sender ) {                      // If we're the sender, we've received an ack payload
            radio.read(&message_count,sizeof(message_count));
            if(mode_debug){
                printf("Ack:%lu\n\r",message_count);
            }
        }


        if ( role == role_receiver ) {                    // If we're the receiver, we've received a time message
            radio.read( &payload, sizeof(Payload) );
            if(mode_debug){
                printf("Got payload %lu, ~ %d\n\r", payload.time, ((int)payload.temperature));
            }

            // Send back the *sender* an ack payload containing a simple counter
            radio.writeAckPayload( 1, &message_count, sizeof(message_count) );  // Add an ack packet for the next time around.  This is a simple
            ++message_count;                                // packet counter

            // signal to the main loop that we have new data
            radio_data_ready = true;

        }
    }
}

// Copied from pingpair_irq loop()
// - adjusted to be called as radio_loop from a scheduler slot
//   (w 2 second repeat) so as to avoid it causing other functions to halt
//   for 2 secs all the time due to using a blocking Arduino 'delay'
void radio_loop() {


  if (role == role_sender)  {                        // Sender role.  Repeatedly send the current time
    unsigned long time = millis();                   // Take the time, and send it.
    // pack the payload struct
    payload.time = time;
    payload.animation_delay = 0; // unused element in struct
    payload.num_colors = 0;      // unused element in struct
    payload.temperature =  onewire_mostrecentvalue(0); // latest temperature

    if(mode_debug){
        printf("Now sending %lu, ~%d\n\r",time, (int8_t)payload.temperature);
    }
    radio.startWrite( &payload, sizeof(Payload) ,0);
  }
}


/*
 * Provide serial output info on radio status & behaviour
 *
 * For the moment this is mainly the basic radio details provided by the
 * RF24/TMRh24 library with a few extras
 */
void radio_info(void){
    printf("Role: %s\n\r",role_friendly_name[role]);
}

void radio_details(void){
    printf("RF since boot: %lu\n\r",message_count);
    radio.printDetails();
}

/****************************************************************************
 *  RF24 Wireless code END
 ****************************************************************************/


