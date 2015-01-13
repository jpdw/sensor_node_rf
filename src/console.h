/*
 * console.h
 *
 *  Created on: 29 Oct 2011
 *      Author: wilkinsj
 */

#ifndef CONSOLE_H_
#define CONSOLE_H_

extern void console_setup();
extern void console_register(const char* command, void(*cbfunction)());
extern void console_start();
extern void console_send_prompt();
extern void console_handle_serial_reception();
extern void console_handle_received_command();


extern void(* resetFunc) (void);


#endif /* CONSOLE_H_ */
