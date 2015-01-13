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

extern float onewire_mostrecentvalue(int);
extern void cb_check_radio(void);


// this external flag indicates whether or not to spew to serial
extern bool debug_mode;

bool radio_data_ready;

/* === RF Hardware connections == 9, 10
 *     Like many of the examples, I'm using Arduino 9, 10
 *     (ATmega 15 & 16 respectively) as well as SPI MISO/MOSI/SCK
 *
 *     The other pin we are using is DIG2 (ATmega pin 4) for INT0 in the code lower down
 *
 *     Comment mentioning 7 & 8 is copied from TMRh20 example ... doesn't look right!
 */
// Hardware configuration
RF24 radio(9,10);          // Set up nRF24L01 radio on SPI bus plus pins 7 & 8


/* === Role pin in original source == 7.  I'm using it already so in the code
 *     here this is now EIGHT 8.  This is ATmega328P pin 14 and is currently
 *     unused on all 3 of my target boards
 */
const short role_pin = 8;  // sets the role of this unit in hardware.  Connect to GND to be the 'pong' receiver
                           // Leave open to be the 'ping' transmitter

// Start of more RF24 code

// Demonstrates another method of setting up the addresses
byte address[][5] = { 0xCC,0xCE,0xCC,0xCE,0xCC , 0xCE,0xCC,0xCE,0xCC,0xCE};

// Role management

// Set up role.  This sketch uses the same software for all the nodes in this
// system.  Doing so greatly simplifies testing.  The hardware itself specifies
// which node it is.
// This is done through the role_pin
typedef enum { role_sender = 1, role_receiver } role_e;                 // The various roles supported by this sketch
const char* role_friendly_name[] = { "invalid", "Sender", "Receiver"};  // The debug-friendly names of those roles
role_e role;                                                            // The role of the current running sketch

static uint32_t message_count = 0;

struct Payload
{
  unsigned long time;     // arduino microsecond counter
  int num_colors;         // colour density factor in hsb-fadearound
  int animation_delay;    // delay factor  in hsb-fadearound
  float temperature;      // latest measured temperature
  //bool ready;           // set when new value received; cleared on use
};
Payload payload;


/****************************************************************************
 *  RF24 Wireless code here
 ****************************************************************************/
// Copied from pingpair_irq "setup()"
// - removed lines that are already dealt with in my setup (start serial etc)

void radio_setup(){

    printf_begin();


    pinMode(role_pin, INPUT);                        // set up the role pin
    digitalWrite(role_pin,HIGH);                     // Change this to LOW/HIGH instead of using an external pin
    delay(20);                                       // Just to get a solid reading on the role pin

    if ( digitalRead(role_pin) )                    // read the address pin, establish our role
        role = role_sender;
    else
        role = role_receiver;

    printf_begin();

    // Setup and configure rf radio
    radio.begin();
    //radio.setPALevel(RF24_PA_LOW);
    radio.enableAckPayload();                         // We will be using the Ack Payload feature, so please enable it
    radio.enableDynamicPayloads();                    // Ack payloads are dynamic payloads
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
    attachInterrupt(0, cb_check_radio, LOW);             // Attach interrupt handler to interrupt #0 (using pin 2) on BOTH the sender and receiver

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

    if(debug_mode == true) {
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
            if(debug_mode){
                printf("Ack:%lu\n\r",message_count);
            }
        }


        if ( role == role_receiver ) {                    // If we're the receiver, we've received a time message
            radio.read( &payload, sizeof(Payload) );
            if(debug_mode){
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

    if(debug_mode){
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
    printf("ROLE: %s\n\r",role_friendly_name[role]);
}

void radio_details(void){
    printf("RF count since last boot: %lu\n\r",message_count);
    radio.printDetails();
}

/****************************************************************************
 *  RF24 Wireless code END
 ****************************************************************************/


