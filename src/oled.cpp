/*
 * oled.cpp
 *
 * Functions for outputting to the oled display
 *
 *  Created on: 16 Jan 2015
 *      Author: wilkinsj
 */

#include "target.h"
#include "config.h"
#include "build_info.h"

#include <Arduino.h>

#include "radio.h"
#include "onewirecore.h"

#ifdef display
  #include "U8glib.h"
#endif


#ifdef display
U8GLIB_SSD1306_128X64 u8g(U8G_I2C_OPT_NONE|U8G_I2C_OPT_DEV_0);  // I2C / TWI
extern char timestamp[7];
extern char datestamp[7];

#endif

#ifdef display

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

/*
 * render_display_main
 *
 * create the main display screen on the old
 *
 *
 */
void render_display_main(void){
    static unsigned long start;
    static unsigned long last;

    dtostrf(last,6,0,tmp);
    dtostrf(onewire_mostrecentvalue(0),6,2,tmp2);
    if(radio_is_running() == true){
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
    u8g.print(datestamp);
    u8g.print("/");
    u8g.print(timestamp);
}
void render_display_bootup(void){
    u8g.firstPage();
    do {
      d2();
    } while( u8g.nextPage() );
}

#endif



