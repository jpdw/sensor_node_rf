/*
 * role.cpp
 *
 *  Created on: 16 Jan 2015
 *      Author: wilkinsj
 */

#include <Arduino.h>
#include "target.h"
#include "role.h"

/*
 * Determine which operating mode this board should be running in:
 * - sender (usually remote sensornode ) or receiver (usually base_node)
 * This is done by:
 *  - checking an nvm bit that indicates to use nvm or hardware to detect
 *  - if hardware, set pin mode, wait short while and check
 *  - if nvm mode, read nvm 'role mode' bit
 *  Put result in to 'role_mode' global
*/

/*
void is_set(int address, uint8_t bit){
    // get the byte from eeprom
    uint8_t i = eeprom_read_byte((unsigned char*)address);
    // create mask
    uint8_t mask = 1 << bit
    if ( i & mask )

}
*/

void set_role_mode(void){
//    uint8_t i = eeprom_read_byte((unsigned char*)EVM_MAP_VER);
    // Mask the correct bit
//    if (i & (1<<EVM_INITFLAGS0_BF_HW_MODE))

    pinMode(HW_ROLE_PIN, INPUT);    // set up the role pin
    digitalWrite(HW_ROLE_PIN,HIGH); // Change this to LOW/HIGH instead of using an external pin
    delay(20);                      // Just to get a solid reading on the role pin

    if ( digitalRead(HW_ROLE_PIN) ) // read the address pin, establish our role
        role = role_sender;
    else
        role = role_receiver;
}


