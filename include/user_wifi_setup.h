#ifndef __USER_WIFI_SETUP_H__
#define __USER_WIFI_SETUP_H__
#include "os_type.h"
#include "user_sensor.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"


#define STATION_IP_ADDR		("192.168.10.112")
#define STATION_IP_GW		("192.168.10.1")
#define STATION_IP_NETMASK	("255.255.255.0")

#define ASSOC_AP_SSID		("mesh_2g")
#define ASSOC_AP_PSWD		("1234567890")

#define LISTEN_UDP_PORT		(25525)

#define ROMOTE_IP			("192.168.10.200")
#define REMOTE_UDP_PORT		(25526)

typedef void (*WifiConnectCallback)(uint8_t);

#endif
