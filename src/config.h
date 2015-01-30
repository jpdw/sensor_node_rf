/*
 * config.h
 *
 *  Created on: 16 Jan 2015
 *      Author: wilkinsj
 */

#ifndef CONFIG_H_
#define CONFIG_H_

//set remote_node to build lower power version with various features ommitted
#define remote_node
//set base_node to build full-featured (non low power) build
//#define base_node

//build for low power mode:
// - shutdown as much as we can that we dont need
// - sleep when idle
// - wake every 1 sec for scheduler
// - keep T0 running (for millis/micros still work)
//#define lowpower

/*
 * The following defines control features to include/exclude
 * By defining a feature, related code will be INCLUDED
 *
 * network -> include wired network
 * display -> include i2c based oled
 * serial  -> include serial interface and interactive console
 *
 */


#ifdef remote_node
  #undef network
  #undef display
  #define serial
  #define opt_onewire
  #define lowpower
#else
  #undef network
  #define display
  #define serial
  #define opt_onewire
  #undef lowpower
#endif


#endif /* CONFIG_H_ */
