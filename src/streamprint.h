/*
 * streamprint.h
 *
 *  Created on: 27 Jan 2015
 *      Author: wilkinsj
 */

#ifndef STREAMPRINT_H_
#define STREAMPRINT_H_

#include <Arduino.h>
#include <avr/pgmspace.h>

#define Serialprint(format, ...) StreamPrint_progmem(Serial,PSTR(format),##__VA_ARGS__)
#define Streamprint(stream,format, ...) StreamPrint_progmem(stream,PSTR(format),##__VA_ARGS__)

extern void StreamPrint_progmem(Print &out,PGM_P format,...);

#endif /* STREAMPRINT_H_ */
