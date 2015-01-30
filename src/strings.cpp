/*
 * strings.cpp
 *
 *  Created on: 8 Apr 2012
 *      Author: wilkinsj
 */

#include "strings.h"

const char dns_host[] PROGMEM = "centos.dev.jpdw.org";
const char uri_webapi[] PROGMEM = "/api/temp.php";
const char uri_hbeat[] PROGMEM = "/api/hbeat.php";
const char content_type_urlencoded[] PROGMEM= "application/x-www-form-urlencoded";
const char post_header[] PROGMEM="POST $F HTTP/1.0" "\r\n"
		"Host: $F\r\nContent-Type: $F\r\n"
		"Content-Length: $D\r\n"
		"\r\n"
		"$H";
const char xap_template[] PROGMEM="xap-hbeat\n{\n"
		"v=13\n"
		"Hop=1\n"
		"UID=$S\n"
		"Class=xap-hbeat.alive\n"
		"Source=$S\n"
		"Interval=$H\n"
		"Port=52706\n"
		"PID=1\n"
		"}\n\n";




