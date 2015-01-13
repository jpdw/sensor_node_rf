/*
 * target.h
 *
 *  Created on: 3 Jan 2015
 *      Author: wilkinsj
 */

#ifndef TARGET_H_
#define TARGET_H_

/* PIN CONFIG
 *
 * Define constants for which <ARDUINO> pin is used
 * for various I/O
 *
 * For each entry, comment includes:
 * Port <atmega i/o port designation>
 * ARD #<arduino port designation>
 * Pin <atmega328p pin number>
 *
 * Const value must be Arduino designation
 *
 * For comparison, Arduino UNO Status LED is on Pin 13 (PB5 -- SPI SCK)
 *
 * This schema also reserves PC4/PC5 for I2C.
 * Leaves the following Arduino (atmega) I/O available:
 *   A0 (PC0), A1 (PC1), A2 (PC2), A3 (PC3)
 *
 * If another INT pin is needed: Move PWM Red from #3 to #9; Move RF24 EN from #9 to #A3
 */

// Dig O/P used for status flashes: Port PD4, ARD#4, Pin 6
#define HW_STATUS_LED_PIN 4
// Dig O/P used for 1Wire network: Port PD7, ARD#7, Pin 13
#define HW_ONEWIRE_PIN 7
// Dig O/P used to Enable SPI to 2.4GHz RF NB radio link: Port PB1, ARD#9, Pin 15
#define HW_SPI_EN_RF24 9
// Dig O/P used to Enable SPI to ethernet module: Port PB0, ARD#8, Pin 14
#define HW_SPI_EN_ETHERNET 8
// Dig O/P used to PWM Red: Port PD3, ARD#3, Pin 5
#define HW_PWM_RGB_RED 3
// Dig O/P used to PWM Green: Port PD5, ARD#5, Pin 11
#define HW_PWM_RGB_GREEN 5
// Dig O/P used to PWM Blue: Port PD6, ARD#6, Pin 12
#define HW_PWM_RGB_BLUE 6
// Dig I/P for hardware interrupt (used by RF24)
#define HW_INT0 2


#endif /* TARGET_H_ */
