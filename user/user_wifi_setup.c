#include "user_wifi_setup.h"
#include "user_interface.h"
#include "osapi.h"

static ETSTimer WiFiStationLinker;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
WifiConnectCallback wifiStationCb = NULL;
static void ICACHE_FLASH_ATTR  wifi_station_connect_check(void *arg)
{
	struct ip_info ipConfig;

	os_timer_disarm(&WiFiStationLinker);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{
		os_timer_setfn(&WiFiStationLinker, (os_timer_func_t *)wifi_station_connect_check, NULL);
		os_timer_arm(&WiFiStationLinker, 15*1000, 0);
	}
	else
	{
		if(wifi_station_get_connect_status() == STATION_WRONG_PASSWORD)
		{
			os_printf("STATION_WRONG_PASSWORD\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_NO_AP_FOUND)
		{
			os_printf("STATION_NO_AP_FOUND\r\n");
			wifi_station_connect();
		}
		else if(wifi_station_get_connect_status() == STATION_CONNECT_FAIL)
		{
			os_printf("STATION_CONNECT_FAIL\r\n");
			wifi_station_connect();
		}
		else
		{
			os_printf("STATION_IDLE\r\n");
		}

		os_timer_setfn(&WiFiStationLinker, (os_timer_func_t *)wifi_station_connect_check, NULL);
		os_timer_arm(&WiFiStationLinker, 2*1000, 0);
	}
	if(wifiStatus != lastWifiStatus){
		lastWifiStatus = wifiStatus;
		if(wifiStationCb)
			wifiStationCb(wifiStatus);
	}
}
static void ICACHE_FLASH_ATTR wifi_station_connect_callback(uint8_t status)
{
	if(status == STATION_GOT_IP)
	{
		os_printf("Assoiciation OK, and start UDP port!\n");
	}
	else
	{
		os_printf("Assoiciation Error!\n");
	}
}
static void ICACHE_FLASH_ATTR register_station_connect_callback(WifiConnectCallback cb)
{
	if(cb != NULL) {
		wifiStationCb = cb;
	}
}

void ICACHE_FLASH_ATTR user_wifi_setup_init(void)
{
	struct ip_info sta_info;
	struct station_config sta_config;

	if (true == wifi_station_dhcpc_status())
	{
		wifi_station_dhcpc_stop();
	}
	sta_info.ip.addr = ipaddr_addr(STATION_IP_ADDR);
	sta_info.gw.addr = ipaddr_addr(STATION_IP_GW);
	sta_info.netmask.addr = ipaddr_addr(STATION_IP_NETMASK);
	if ( true != wifi_set_ip_info(STATION_IF,&sta_info)) {
		os_printf("set default ip wrong\n");
	}
	register_station_connect_callback(wifi_station_connect_callback);
	wifi_set_opmode_current(STATION_MODE);
	
	os_memset(&sta_config, 0, sizeof(struct station_config));
	os_memcpy(sta_config.ssid, ASSOC_AP_SSID, os_strlen(ASSOC_AP_SSID));
	os_memcpy(sta_config.password, ASSOC_AP_PSWD, os_strlen(ASSOC_AP_PSWD));
	wifi_station_set_config_current(&sta_config);
	
	os_timer_disarm(&WiFiStationLinker);
	os_timer_setfn(&WiFiStationLinker, (os_timer_func_t *)wifi_station_connect_check, NULL);
	os_timer_arm(&WiFiStationLinker, 5*1000, 0);

	wifi_station_connect();
}

