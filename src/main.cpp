/*
 * main.cpp
 *
 *  Created on: 3 Jan 2015
 *      Author: wilkinsj
 */


#include "target.h"

// Mandatory includes
#include <Arduino.h>
#include "SPI.h"
#include "HardwareSerial.h"
#include "U8glib.h"
#include "console.h"
#include "scheduler.h"
#include "onewirecore.h"
#include "memoryfree.h"
#include "radio.h"

extern void radio_loop();




// End of RF24 code


void flash_led(void);

// control level of serial output
bool debug_mode;

unsigned long lt;

#define INFO_DISP_SCHED_WIDE 5

U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI


/*
 * showinfo
 * - show various system information
 *   - current millis (the closest the device has to realtime)
 *   - eeprom map version
 *   - freememory
 *   - schedulers enabled
 *   - 1-wire devices
 */
void showinfo(void){
    uint8_t i;

    Serial.println(F("SYSINFO::\n\n\rBuild: " __DATE__ " " __TIME__));

    radio_info();

    /*
    DateTime now = RTC.now();

    // Show current time
    Serial.println("Time:");
    Serial.print(now.year(), DEC);
    Serial.print('/');
    Serial.print(now.month(), DEC);
    Serial.print('/');
    Serial.print(now.day(), DEC);
    Serial.print(' ');
    Serial.print(now.hour(), DEC);
    Serial.print(':');
    Serial.print(now.minute(), DEC);
    Serial.print(':');
    Serial.print(now.second(), DEC);
    Serial.print("; since 2000 = ");
    Serial.print(now.get());
    Serial.print("s = ");
    Serial.print(now.get() / 86400L);
    Serial.println("d");
    */

	// current millis
	Serial.print(F("Device millis "));
	Serial.print(millis());

	// last loop execution
	Serial.print(F("  Loop u "));
	Serial.println(lt);

	/*
	// eeprom map version
	i=EEPROM.read(evm_map_ver);
	Serial.print(F("Memory:\n\rEEPROM ver "));
	Serial.print(i,DEC);
	*/

	// freememory
    Serial.print(F("RAM "));
    Serial.println(free());


	// schedulers enabled
	Serial.println(F("Schedulers:"));
	i=0;
	do{
		Serial.print(F(" S"));
		Serial.print(i,DEC);
		Serial.print(F(" "));
		if(scheduler_isenabled(i)){
			Serial.print(F("E"));
		}else{
			Serial.print(F("D"));
		}

		if((i+1) % INFO_DISP_SCHED_WIDE  == 0){
			Serial.println("");
		}else{
			Serial.print("  ");
		}
	} while (++i<FIXED_SCHEDULED_SLOTS);
	if((i)% INFO_DISP_SCHED_WIDE !=0){
		Serial.println("");
	}

  // 1-wire devices
  onewire_addresses();

  // Radio-related info
  radio_details();

}



static char tmp[7]="123456";
static char tmp2[7]="      ";
static char tmp3[11]="          ";
static char tmp4[7]="      ";
void draw() {
    // display title at top of screen
    u8g.setPrintPos(0, 12);
    u8g.print(F("Latest:"));

    // display display loop count in upper right
    u8g.setPrintPos(80,12);
    u8g.print(tmp);

    // display local temperature in lower right
    u8g.setPrintPos(0,46);
    u8g.print(F("Here:"));
    u8g.setPrintPos(80,46);
    u8g.print(tmp2);

    // display a received time value upper-mid right
    if(radio_is_running() == true){
        u8g.setPrintPos(48,28);
        u8g.print(tmp3);
    }

    // display a received temperature value lower-mid right
    if(remote_has_temperature() == true){
        u8g.setPrintPos(0,64);
        u8g.print(F("There:"));
        u8g.setPrintPos(80,64);
        u8g.print(tmp4);
    }
}

void print_test(void){
    static unsigned long start;
    static unsigned long last;

    //char * t, * t2, *t3, *t4;
    //t=dtostrf(last,6,0,tmp);
    dtostrf(last,6,0,tmp);
    //t2=dtostrf(onewire_mostrecentvalue(0),6,2,tmp2);
    dtostrf(onewire_mostrecentvalue(0),6,2,tmp2);
    if(radio_is_running() == true){
        //t3=dtostrf(radio_payload_time(),10,0,tmp3);
        dtostrf(radio_payload_time(),10,0,tmp3);
    }
    if(remote_has_temperature() == true){
        //t4=dtostrf(remote_temperature_value(),6,2,tmp4);
        dtostrf(remote_temperature_value(),6,2,tmp4);
    }

    // we don't intend to change the font to we can do it once here
    // rather than inside the picture loop
    u8g.setFont(u8g_font_unifontr);

    start = micros();

    // clear the radio data ready
    // - this might have a race condition... might need to be done differently
    radio_data_ready = false;

    u8g.firstPage();
    do {
        draw();
    } while( u8g.nextPage() );

    last = micros() - start;

}

void d2(void){
    u8g.setFont(u8g_font_unifontr);
    u8g.setPrintPos(0,14);
    u8g.print(F("Build: "));
    u8g.setPrintPos(0,30);
    u8g.print(__DATE__ " " __TIME__);
}
void display_message_bootup(void){
    u8g.firstPage();
    do {
      d2();
    } while( u8g.nextPage() );
}

/*
 * callback triggered to request temperature readings
 */
void cb_request_temp_retrieval(){
    onewire_request_temps();
}

/*
 * callback triggered when temp requests should be ready to retrieve
 */
void cb_temp_requests_ready(){
    //float temps[3];
    //onewire_read_temps(temps);
    //send_temperature(temps,onewire_device_count());
    onewire_read_temps();
    //send_temperature(onewire_devices,onewire_device_count());
}

/* Refresh OLED display
 *
 */
void cb_display_refresh(){
    print_test();
}
/*
 * Watchdog to trigger every 1 minute
 */
void cb_1mwd(){
    // check stash free & reset if needed
    //if(Stash::freeCount()<5){
    //    Stash::initMap(56);
    //}
}

void flash_led(){
    static bool led_state;
    led_state=!led_state;
    digitalWrite(HW_STATUS_LED_PIN, led_state);
}


/*
 * Generic place to test stuff
 */
void test(){
}

/*
 * Debug function
 *
 * Toggle debug mode: true/false
 */
void debug(){
    debug_mode = !debug_mode;
}


void show_sensors(){
    onewire_debug();
}

void setup(){

    display_message_bootup();
    Serial.begin(115200);

    // disable debug mode by default -- use debg to enable
    debug_mode = false;

    pinMode(HW_STATUS_LED_PIN, OUTPUT);


    /*
    // initialise eeprom
    uint8_t ver=EEPROM.read(evm_map_ver);
    if (ver==255){
        Serial.println(F("Bad eeprom version, resetting"));
        //factory_reset();
        resetFunc();
    }
    */

    // run various module setups
    //netcore_setup();

    //radio_setup();


    onewire_setup();
    scheduler_setup();
    console_setup();

    // initialise & start the console
    //console_register("icfg",&netcore_ifconfig);
    //console_register("fact",&factory_reset);
    //console_register("web",&cb_request_temp_retrieval);
    console_register("info",&showinfo);
    console_register("debg",&debug);
    console_register("sens",&show_sensors);
    //console_register("print",&print_test);
    //console_register("xaph",&xap_send_hbeat);
    console_register("call",&radio_loop);
    console_register("run",&scheduler_run);
    console_register("stop",&scheduler_stop);

    console_start();

    radio_setup();

    showinfo();

    // start certain scheduler jobs
    scheduler_run();

}


void loop(){
unsigned long last;

	/*
	 * Run the loop
	 *
	 */

    do {
        // give various modules an execution slot
        //netcore_loop();
        scheduler_loop();
        //radio_loop();

        // handle console
        console_handle_received_command();	// process command received completely
        console_handle_serial_reception();	// process new chars received on serial

        lt=micros()-last; // millis for last run
        last=micros();
    } while(1);

}


/*
 * Standard main() function
 *
 * Run the init() [ inits arduino stuff] then application setuo() and loop()
 * Normally loop will never exit so the for() is slightly redundant but for the moment it has
 * been kept in case there is a usefullness if being able to exit loop() to re-initialise etc,.
 */
//See http://www.tty1.net/blog/2008-04-29-avr-gcc-optimisations_en.html
// for this optimisation:
//void main() __attribute__ ((noreturn))
// just adding -ffreestanding to the C & CPP flags seems to have saved 2 bytes of progmem!
int main(void)
{
	init();
	setup();
	// Atmel recommended optimal never-ending loop (see above url)
	for (;;)
		loop();
	return 0;
}


/* Function added here to get the code to compile outside of older Arduino IDE e.g eclipse
 * however, adding this code increase the finished Release by approx 276 bytes (presumably as
 * the build will pull in analogRead and other dependancies
 */
/*
void dummy(){
	uint8_t a=analogRead(1);
}
*/

