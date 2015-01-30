/*
 * main.cpp
 *
 *  Created on: 3 Jan 2015
 *      Author: wilkinsj
 */



// Power management
#define power_adc_enable()      (PRR &= (uint8_t)~(1 << PRADC))
#define power_adc_disable()     (PRR |= (uint8_t)(1 << PRADC))

#define power_spi_enable()      (PRR &= (uint8_t)~(1 << PRSPI))
#define power_spi_disable()     (PRR |= (uint8_t)(1 << PRSPI))

#define power_usart0_enable()   (PRR &= (uint8_t)~(1 << PRUSART0))
#define power_usart0_disable()  (PRR |= (uint8_t)(1 << PRUSART0))

#include "target.h"
#include "config.h"
#include "build_info.h"

// Mandatory includes
#include <Arduino.h>
#include "avr/io.h"
#include "avr/sleep.h"
#include "avr/wdt.h"
#include "avr/power.h"
#include "avr/eeprom.h"
#include "SPI.h"
#ifdef serial
  #include "HardwareSerial.h"
#endif
#ifdef display
  #include "oled.h"
#endif
#ifdef serial
  #include "console.h"
#endif
#include "scheduler.h"
#include "onewirecore.h"

#ifdef network
  #include "netcore.h"
#endif

#include "memoryfree.h"
#include "radio.h"
#include "eeprom_map.h"
#include "role.h"
#include "streamprint.h"

// Declare additional external functions here
extern void radio_loop();

// Declare any local functions that are references before declaration
void flash_led_blue();

// Global operating mode flags
bool mode_lowpower;       // if true, run lowpower variants of certain things
bool mode_debug;          // if true, enable additional debugging output
bool enable_oled;         // if true, enable oled output
bool enable_ethr;         // if true, enable ethernet features
bool enable_wdt_ticktock; // if true (& lowpower mode), flash led on wdt isr
bool enable_send_led;     // if true, flash led on each send

char timestamp[7] = {__BI__TIMESTAMP_STR};
char datestamp[7] = {__BI__DATESTAMP_STR};

role_e role;                                                            // The role of the current running sketch

/*
 * ISR for WDT
 *
 * Disable the WDT (it will be re-enabled later) & tick-tock if required
 */
ISR (WDT_vect)
{
    wdt_disable();  // disable watchdog
    if(enable_wdt_ticktock){
        flash_led_blue();
    }
}  // end of WDT_vect


void flash_led(void);


unsigned long lt;

#define INFO_DISP_SCHED_WIDE 5


#ifdef serial
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
    // Abort if in lowpower mode (no serial)
    if(mode_lowpower){return;}

    //
    Serialprint("\nSYSINFO:\n\rBuild: %s/%s\n\r", datestamp, timestamp);
    radio_info();

    // current millis
    Serial.print(F("Device millis "));
    Serial.print(millis());

    // last loop execution
    Serial.print(F("  Loop u "));
    Serial.println(lt);

    // eeprom map version
    uint8_t i = eeprom_read_byte((unsigned char*)EVM_MAP_VER);
    Serialprint("Memory: \n\rEEPROM ver %d; RAM %d\n\r", i, free());

    // schedulers enabled
    Serialprint("Schedulers:\r\n");
    i=0;
    do{
        Serialprint(" S%d ", i);
        if(scheduler_isenabled(i)){
            Serialprint("E");
        }else{
            Serialprint("D");
        }

        if((i+1) % INFO_DISP_SCHED_WIDE  == 0){
            Serialprint("\r\n");
        }else{
            //Serial.print("  ");
            Serialprint("  \r\n");
        }
    } while (++i<FIXED_SCHEDULED_SLOTS);
    if((i)% INFO_DISP_SCHED_WIDE !=0){
        Serialprint("\r\n");
    }

    // 1-wire devices
    onewire_addresses();

    // Radio-related info
    radio_details();

#ifdef network
    netcore_ifconfig();
#endif


}

#endif



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

#ifdef display
/* Refresh OLED display
 *
 */
void cb_display_refresh(){
    //if(!enable_oled){
    //    return;
    //}

    render_display_main();
}

#endif

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

void flash_led_blue(){
    static bool led_state;
    led_state=!led_state;
    digitalWrite(HW_PWM_RGB_BLUE, led_state);
}

/*
 * Generic place to test stuff
 */
void test(){
}

/*
 * Toggle debug mode: true/false
 */
void debug(){
    mode_debug = !mode_debug;
    Serial.print("Debug mode ");
    if(mode_debug == true){
        Serial.println(F("enabled"));
    }else{
        Serial.println(F("disabled"));
    }
}


void show_sensors(){
    onewire_debug();
}

/*
 * power_management
 *
 * Configure power-usage related options such as en/dis processor feature,
 * en/dis port modes,
 *
 * Also set boolean flags that to en/dis related code paths so as to avoid
 * executing code that would have no affect due to being related to hardware or
 * features that have been disabled.
 *
 * Function to be called for either build-time config and will en/dis as
 * appropriate
 */

void power_management(void){
    // power management

    // Disable ADC & analog comparator as these are not used by either config
    //previousADCSRA = ADCSRA;
    ADCSRA &= ~(1<<ADEN); //Disable ADC
    ACSR = (1<<ACD); //Disable the analog comparator
    power_adc_disable(); // disable adc (power.h)

    // Disable digital inputs on ADC1-5 ( 4 & 5 are I2C so need to check if this still works with Dig In disabled)
    // Leave ADC0 as that is mode pin -- at the moment !!!!
    DIDR0 = 0x3E; //Disable digital input buffers on all ADC0-ADC5 pins
#ifdef opt_onewire
    DIDR1 = (0<<AIN1D)|(1<<AIN0D); //Disable digital input buffer on AIN0 (leave AIN1 enabled for 1-wire)
#else
    DIDR1 = (1<<AIN1D)|(1<<AIN0D); //Disable digital input buffer on AIN0 & AIN1
#endif

    // timer1 and timer2 are not currently used at all => disable!
    power_timer1_disable();
    power_timer2_disable();

    // unless "display" is enabled, disable twi
#ifdef display
    power_twi_enable();
    enable_oled=true;
#else
    power_twi_disable();
    enable_oled=false;
#endif

#ifdef serial
    // ensure usart is powered up
#else
    // unpower usart
#endif

    /*
     * determine which power-down mode to use -- basically
     * defined by whether we have serial or not
     *
     * - if build has serial included the lowest we can go is "IDLE" (keep usart running)
     * - otherwise
     *   - if this is base_mode, INT0 is needed to interrupt on receipt of radio traffic
     *   - otherwise
     *     - sleep max (timer still runs, triggers sensor refresh & send)
     */

#ifdef serial
    set_sleep_mode (SLEEP_MODE_IDLE); // prepare for powerdown
#else
#ifdef base_node
    set_sleep_mode (SLEEP_MODE_ADC); // prepare for powerdown
#else
    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
#endif
#endif


#ifdef lowpower
    // clear various "reset" flags
    MCUSR = 0;
    // allow changes, disable reset, enable Watchdog interrupt
    WDTCSR = bit (WDCE) | bit (WDE);
    // set interval (see datasheet p55)
    WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // 128K cycles = approx 1 second
    wdt_reset();  // start watchdog timer

    //sleep mode set above as appropriate:
    //set_sleep_mode (SLEEP_MODE_PWR_DOWN); // prepare for powerdown
    //sleep_enable();
#endif


}


void setup(){

    set_role_mode();

    // set default operating flags
    mode_debug = false;     // debug disabled (debg to enable)
    enable_send_led = false; // flash on send

#ifdef lowpower
    mode_lowpower = true;  // lowpower mode by default
#else
    mode_lowpower = false; // lowpower disabled by default
#endif

    /*
     * From this point on, setup might set different options
     * depending on whether mode_lowpower is true or false.
     *
     * This is now a run-time changeable value to allow a remote
     * node to have it's power profile changed dynamically without
     * needing to be re-installed
     */

#ifdef display
    // If display present, display bootup version info
    render_display_bootup();
#endif

    // Set up output pins for status LEDs
    pinMode(HW_STATUS_LED_PIN, OUTPUT);
    pinMode(HW_PWM_RGB_RED, OUTPUT);

    // Call sub-function to set-up power-related options
    power_management();

#ifdef network
    // run various module setups
    netcore_setup();
#endif

    // start onewire
    // for lowpower mode, start it in 'blocking mode' (true)
    // for non-lowpower mode, start it in 'non-blocking mode' (false)
    // as mode_lowpower=true for lowpower mode, passing mode flag is ok for both
    onewire_setup(mode_lowpower);

    // the main scheduler is not used in lowpower mode, so only init if = false
    if(!mode_lowpower){
        scheduler_setup();
    }

#ifdef serial
    // start serial and console if not in lowpower mode
    if(!mode_lowpower){
        Serial.begin(115200);

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
    }
#endif


    // start the radio
    radio_setup();

    if(!mode_lowpower){
#ifdef serial
        // display info to serial
        showinfo();
#endif
        // start certain scheduler jobs (not in lowpower)
        scheduler_run();
    }
}

#ifdef lowpower
// Simpler variant of loop() for ISR testing
void loop(){
    static int counter=0;

    // disable ADC
    ADCSRA = 0;

    power_adc_disable(); // ADC converter
    //power_spi_disable(); // SPI
    power_usart0_disable();// Serial (USART)
    //power_timer0_disable();// Timer 0
    power_timer1_disable();// Timer 1
    power_timer2_disable();// Timer 2
    power_twi_disable(); // TWI (I2C)

    // clear various "reset" flags
    MCUSR = 0;
    // allow changes, disable reset
    WDTCSR = bit (WDCE) | bit (WDE);
    // set interrupt mode and an interval
    WDTCSR = bit (WDIE) | bit (WDP2) | bit (WDP1);    // set WDIE, and 1 seconds delay
    wdt_reset();  // pat the dog

    set_sleep_mode (SLEEP_MODE_PWR_DOWN);
    noInterrupts ();           // timed sequence follows
    sleep_enable();

    // turn off brown-out enable in software
    MCUCR = bit (BODS) | bit (BODSE);
    MCUCR = bit (BODS);
    interrupts ();             // guarantees next instruction executed
    sleep_cpu ();

    // cancel sleep as a precaution
    sleep_disable();

    // we get here after the WDT wakes the processor

    counter++;

    if(counter>10){
        if(enable_send_led){
            flash_led();
        }
        onewire_request_temps();
        onewire_read_temps();
        if(enable_send_led){
            flash_led();
        }
        radio_loop();
        counter=0;
    }

}

#else

void loop(){
unsigned long last=0;

	/*
	 * Run the loop
	 *
	 */

    do {
        // give various modules an execution slot
#ifdef network
        netcore_loop();
#endif
        scheduler_loop();

#ifdef serial
        // handle console
        console_handle_received_command();	// process command received completely
        console_handle_serial_reception();	// process new chars received on serial
#endif

        lt=micros()-last; // millis for last run
        last=micros();

    } while(1);

}
#endif


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

