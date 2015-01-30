/*
 * eeprom_map.h
 *
 *  Created on: 3 Jan 2012
 *      Author: wilkinsj
 *
 *  This is a map of the EEPROM memory used by the app
 *  A global instance of the EEPROM memory read/write class will be available
 */

#ifndef EEPROM_MAP_H_
#define EEPROM_MAP_H_

// all prefixed EVM - eeprom value map
/////// LABEL_NAME  BYTE_OFFSET // BYTE_SIZE : USAGE/DESCRIPTION/NOTES
#define EVM_MAP_VER 1			//     1     : version of map (in case this changes...)
#define EVM_INITFLAGS0 2        //     1     : flags used at initialisation to enable/disable things
#define EVM_RF_CHNNL 3          //     1     : RF channel to use (if unset, use default)
#define EVM_HW_MODEPIN 4        //     1     : (arduino) pin # to read hw mode from


// =====================
// BIT FIELD DEFINITIONS
// =====================

// EVM_NETFLAGS - Bits for netflags
#define EVM_INITFLAGS0_BF_DEBUG_ENABLED 0 // : if set, debug=true at boot
#define EVM_INITFLAGS0_BF_OLED_ENABLED 1  // : if set, enable oled display o/p
#define EVM_INITFLAGS0_BF_SERIAL_ENABLED 2// : if set, enable serial/console
#define EVM_INITFLAGS0_BF_HW_MODE 3       // : HW_MODE unless HW_MODEPIN != 0

// EVM_INITFLAGS - Bits for initflags
//#define EVM_INITFLAGS0_BF_INITSCHED 0  // b0 high = Initialise scheduler jobs

#define evm_netflags 1
#define EVM_NETFLAGS_BF_DHCP 0
#define EVM_STATIC_IP1 1
#define EVM_STATIC_GWIP1 1
#define EVM_STATIC_MASK1 1

#endif /* EEPROM_MAP_H_ */
