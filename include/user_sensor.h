#ifndef __USER_SENSOR_H__
#define __USER_SENSOR_H__

#define LEVEL_H							1
#define LEVEL_L							0

#define SIG_RX_HUMAN			0x00
#define SIG_RX_ILLUMINANCE		0x01
#define SIG_RX_APP				0x02

#define LED_ONOFF_IO_MUX				PERIPHS_IO_MUX_MTCK_U
#define LED_ONOFF_IO_NUM				13
#define LED_ONOFF_IO_FUNC				FUNC_GPIO13

#define PWM_CHANNEL_NUM					1
#define PWM_PERIOD_NUM					500  //0.5khz
#define PWM_DUTY_MAX					11111

#define LED_PWM_IO_MUX					PERIPHS_IO_MUX_MTDI_U
#define LED_PWM_IO_NUM					12
#define LED_PWM_IO_FUNC					FUNC_GPIO12

#define SENSOR_INFRARED_LED_IO_MUX		PERIPHS_IO_MUX_MTDO_U
#define SENSOR_INFRARED_LED_IO_NUM		15
#define SENSOR_INFRARED_LED_IO_FUNC		FUNC_GPIO15

void user_sensor_init(void);

#endif
