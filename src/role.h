/*
 * role.h
 *
 *  Created on: 16 Jan 2015
 *      Author: wilkinsj
 */

#ifndef ROLE_H_
#define ROLE_H_

extern void set_role_mode(void);

typedef enum { role_sender = 1, role_receiver } role_e;                 // The various roles supported by this sketch

extern role_e role;



#endif /* ROLE_H_ */
