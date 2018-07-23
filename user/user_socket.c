#include "user_socket.h"
#include "user_interface.h"
#include "osapi.h"
#include "espconn.h"

void ICACHE_FLASH_ATTR user_socket_init(void)
{
	LOCAL struct espconn esp_conn;
	LOCAL esp_udp espudp;

	esp_conn.type = ESPCONN_UDP;
	esp_conn.state = ESPCONN_NONE;
	esp_conn.proto.udp = &espudp;
	esp_conn.proto.udp->local_port = LISTEN_UDP_PORT;
	espconn_regist_recvcb(&esp_conn, webserver_listen);

	espconn_create(&esp_conn);

	os_timer_disarm(&ssdp_time_serv);
	os_timer_setfn(&ssdp_time_serv, (os_timer_func_t *)airkiss_wifilan_time_callback, NULL);
	os_timer_arm(&ssdp_time_serv, 1000, 1);
}

