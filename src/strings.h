/*
 * strings.h
 *
 *  Created on: 8 Apr 2012
 *      Author: wilkinsj
 */

#ifndef STRINGS_H_
#define STRINGS_H_

#ifndef Arduino_h
  #include <Arduino.h>
#endif

//extern char website[] PROGMEM;

extern const char dns_host[];
extern const char uri_webapi[];
extern const char uri_hbeat[];
extern const char content_type_urlencoded[];
extern const char post_header[];
extern const char xap_template[];

#endif /* STRINGS_H_ */
