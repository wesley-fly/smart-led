#include "user_wifi_setup.h"
#include "user_interface.h"
#include "osapi.h"

static ETSTimer WiFiStationLinker;
static uint8_t wifiStatus = STATION_IDLE, lastWifiStatus = STATION_IDLE;
WifiConnectCallback wifiStationCb = NULL;
LOCAL os_timer_t app_socket_time_serv;

LOCAL ICACHE_FLASH_ATTR
void app_recv(void *arg, char *pdata, unsigned short len)
{
    struct espconn *pesp_conn = arg;
	static char __light_cmd[16];
	os_memset(__light_cmd, 0, sizeof(__light_cmd));
	os_memcpy(__light_cmd, pdata, len);
	if(os_strncmp(__light_cmd, "11111111", 8) == 0)
	{
		system_os_post(USER_TASK_PRIO_0, SIG_RX_APP, 'y');
	} 
	else if(os_strncmp(__light_cmd, "00000000", 8) == 0)
	{
		system_os_post(USER_TASK_PRIO_0, SIG_RX_APP, 'n');
	}
}

void ICACHE_FLASH_ATTR user_socket_init(void)
{
	LOCAL struct espconn esp_conn;
	LOCAL esp_udp espudp;

	esp_conn.type = ESPCONN_UDP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.udp = &espudp;
	esp_conn.proto.udp->local_port = LISTEN_UDP_PORT;
	espconn_regist_recvcb(&esp_conn, app_recv);

	espconn_create(&esp_conn);
}

static void ICACHE_FLASH_ATTR  wifi_station_connect_check(void *arg)
{
	struct ip_info ipConfig;

	os_timer_disarm(&WiFiStationLinker);
	wifi_get_ip_info(STATION_IF, &ipConfig);
	wifiStatus = wifi_station_get_connect_status();
	if (wifiStatus == STATION_GOT_IP && ipConfig.ip.addr != 0)
	{
		os_timer_setfn(&WiFiStationLinker, (os_timer_func_t *)wifi_station_connect_check, NULL);
		os_timer_arm(&WiFiStationLinker, 10*1000, 0);
		os_printf("WiFi Connect OK, start check every 10 seconds\n");
		/*static int tmp = 0;
		if (tmp == 0){
			os_printf("pwm_set_duty PWM_DUTY_MAX\n");
			pwm_set_duty(PWM_DUTY_MAX,0);
		}
		else if (tmp == 1)
		{
			os_printf("pwm_set_duty PWM_DUTY_MAX*9/16\n");
			pwm_set_duty(PWM_DUTY_MAX*9/16,0);
		}
		else if (tmp == 2)
			{
			os_printf("pwm_set_duty PWM_DUTY_MAX*10/16\n");
		pwm_set_duty(PWM_DUTY_MAX*10/16,0);
			}
		else if (tmp == 3)
			{
			os_printf("pwm_set_duty PWM_DUTY_MAX*11/16\n");
		pwm_set_duty(PWM_DUTY_MAX*11/16,0);
			}
		else if (tmp == 4)
			{
			os_printf("pwm_set_duty 0\n");
		pwm_set_duty(0,0);
			}
		pwm_start();

		tmp++;
		if(tmp == 5)
			tmp = 0;*/
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
			wifi_station_connect();
		}
		os_printf("WiFi Connect error, so re-connect it every 2 secondss\r\n");

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
		user_socket_init();
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
	struct station_config sta_config;
	wifi_station_set_auto_connect(0);
	wifi_station_disconnect();
	wifi_set_opmode_current(STATION_MODE);

	if (true == wifi_station_dhcpc_status())
	{
		os_printf("DHCP client on, so stop it\n");
		if(true != wifi_station_dhcpc_stop())
			os_printf("Stop DHCP client failed!\n");
	}
	
	struct ip_info sta_info;
	sta_info.ip.addr = ipaddr_addr(STATION_IP_ADDR);
	sta_info.gw.addr = ipaddr_addr(STATION_IP_GW);
	sta_info.netmask.addr = ipaddr_addr(STATION_IP_NETMASK);
	if ( true != wifi_set_ip_info(STATION_IF,&sta_info)) {
		os_printf("Set default ip wrong\n");
	}
	
	register_station_connect_callback(wifi_station_connect_callback);
	
	os_memset(&sta_config, 0, sizeof(struct station_config));
	os_memcpy(sta_config.ssid, ASSOC_AP_SSID, os_strlen(ASSOC_AP_SSID));
	os_memcpy(sta_config.password, ASSOC_AP_PSWD, os_strlen(ASSOC_AP_PSWD));
	if(true != wifi_station_set_config_current(&sta_config))
		os_printf("Set sta_config client failed!\n");
	
	os_timer_disarm(&WiFiStationLinker);
	os_timer_setfn(&WiFiStationLinker, (os_timer_func_t *)wifi_station_connect_check, NULL);
	os_timer_arm(&WiFiStationLinker, 5*1000, 0);

	wifi_station_connect();
}

