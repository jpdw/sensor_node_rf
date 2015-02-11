/*
 * reboot.cpp
 *
 *  Created on: 11 Feb 2015
 *      Author: wilkinsj
 */


void(* resetFunc)(void) = 0; //declare reset function @ address 0
void reboot(){ resetFunc();} //function to call the above... hacky, can prob be improved



