#include "c_types.h"
#include "eagle_soc.h"
#include "gpio.h"
#include "osapi.h"
#include "user_sensor.h"
#include "user_interface.h"
#include "os_type.h"
#include "c_types.h"
#include "driver/i2c_master.h"
#include <math.h>

#define SENSER_QUEUE_SIZE 4
os_event_t senser_procTaskQueue[SENSER_QUEUE_SIZE];
static ETSTimer illuminance_timer;
static ETSTimer human_timer;

LOCAL void ICACHE_FLASH_ATTR user_reset_pwm_duty(uint32 duty, uint8 channel)
{
    os_printf("PWM set duty : %d \r\n",duty);
    pwm_set_duty(duty,channel);
	pwm_start();
}

LOCAL void ICACHE_FLASH_ATTR Senser_Task(os_event_t *events)
{
	switch (events->sig)
	{
		case SIG_RX_HUMAN:
			os_printf("Human events->sig : %c \n",(char)events->par);
			if((char)events->par == 'y')
				GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_ONOFF_IO_NUM), LEVEL_L);
			else if((char)events->par == 'n')
				GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_ONOFF_IO_NUM), LEVEL_H);
		break;
		case SIG_RX_ILLUMINANCE:
			os_printf("ILLUMINANCE events->sig : %c \n",(char)events->par);
			if((char)events->par == 'l')
				user_reset_pwm_duty(PWM_DUTY_MAX,0);
			else if((char)events->par == 'm')
				user_reset_pwm_duty(PWM_DUTY_MAX/2,0);
			else if((char)events->par == 'h')
				user_reset_pwm_duty(0,0);
		break;
		case SIG_RX_APP:
			os_printf("App events->sig : %c \n",(char)events->par);
			if((char)events->par == 'y')
				GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_ONOFF_IO_NUM), LEVEL_L);
			else if((char)events->par == 'n')
				GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_ONOFF_IO_NUM), LEVEL_H);
		break;
		default:
		break;
	}
}
LOCAL void ICACHE_FLASH_ATTR
user_max44009_init(void)
{
    i2c_master_gpio_init();
}

LOCAL bool ICACHE_FLASH_ATTR
user_max44009_read(uint8 *pData)
{
    uint8 ack;

    i2c_master_start();
	
    i2c_master_writeByte(0x94);
    ack = i2c_master_getAck();
    if (ack) {
        os_printf("0x94 addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }
	i2c_master_writeByte(0x03);
	ack = i2c_master_getAck();
	if (ack) {
		os_printf("0x03 addr not ack when tx write cmd \n");
		i2c_master_stop();
		return false;
	}
	i2c_master_start();
	i2c_master_writeByte(0x95);
	ack = i2c_master_getAck();
	if (ack) {
		os_printf("0x95 addr not ack when tx write cmd \n");
		i2c_master_stop();
		return false;
	}
	
	pData[0] = i2c_master_readByte();

    i2c_master_start();
    i2c_master_writeByte(0x94);
    ack = i2c_master_getAck();
    if (ack) {
        os_printf("0x94 2 addr not ack when tx write cmd \n");
        i2c_master_stop();
        return false;
    }
	i2c_master_writeByte(0x04);
	ack = i2c_master_getAck();
	if (ack) {
		os_printf("0x04 2 addr not ack when tx write cmd \n");
		i2c_master_stop();
		return false;
	}

	i2c_master_start();
	i2c_master_writeByte(0x95);
	ack = i2c_master_getAck();
	if (ack) {
		os_printf("0x95 2addr not ack when tx write cmd \n");
		i2c_master_stop();
		return false;
	}

	pData[1] = i2c_master_readByte();

    i2c_master_stop();

    return true;
}

void human_intr_handler() 
{
	uint32_t gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status);
	if(gpio_status & (BIT(SENSOR_INFRARED_LED_IO_NUM))) {
		//disable interrupt
        gpio_pin_intr_state_set(GPIO_ID_PIN(SENSOR_INFRARED_LED_IO_NUM), GPIO_PIN_INTR_DISABLE);
		//clear interrupt status
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(SENSOR_INFRARED_LED_IO_NUM));
		if(GPIO_INPUT_GET(GPIO_ID_PIN(SENSOR_INFRARED_LED_IO_NUM)))
			system_os_post(USER_TASK_PRIO_0, SIG_RX_HUMAN, 'y');
		else
			system_os_post(USER_TASK_PRIO_0, SIG_RX_HUMAN, 'n');

		gpio_pin_intr_state_set(GPIO_ID_PIN(SENSOR_INFRARED_LED_IO_NUM), GPIO_PIN_INTR_ANYEDGE);
	}
}

static void ICACHE_FLASH_ATTR  get_illuminance_data_cb(void *arg)
{
	uint8 illuminance_data[2]={0};
	uint8_t Exponent = 0;
	uint8_t Mantissa = 0;
	uint32 ValueLux;

	os_timer_disarm(&illuminance_timer);
	
	user_max44009_init();
	if(user_max44009_read(illuminance_data))
	{
		Exponent = (illuminance_data[0] & 0xf0);
		Exponent >>= 4;

		Mantissa = (illuminance_data[0] & 0x0f);
		Mantissa <<= 4;
		Mantissa |= (illuminance_data[1] & 0x0f);

		if(Exponent > 0x0E)
		{
			Exponent = 0x0E;
		}

		ValueLux = (1 << Exponent) * Mantissa * 1000 / 45;
		ValueLux = ValueLux * 10 / 14;
		os_printf("i2c_master_readByte Light : %d %d, ret = %d \r\n",illuminance_data[0],illuminance_data[1],ValueLux);

		if(ValueLux <= 10000)
			system_os_post(USER_TASK_PRIO_0, SIG_RX_ILLUMINANCE, 'h');
		else if ((ValueLux > 10000)&&(ValueLux <= 2000000))
			system_os_post(USER_TASK_PRIO_0, SIG_RX_ILLUMINANCE, 'm');
		else if(ValueLux > 2000000)
			system_os_post(USER_TASK_PRIO_0, SIG_RX_ILLUMINANCE, 'l');
	}
	
	os_timer_setfn(&illuminance_timer, (os_timer_func_t *)get_illuminance_data_cb, NULL);
	os_timer_arm(&illuminance_timer, 5*1000, 0);
}
static void ICACHE_FLASH_ATTR  human_intterupt_init(void *arg)
{
	os_timer_disarm(&human_timer);
	os_printf("human_intterupt_init started\r\n");
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);

	//PIN_FUNC_SELECT(SENSOR_INFRARED_LED_IO_MUX, SENSOR_INFRARED_LED_IO_FUNC);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(SENSOR_INFRARED_LED_IO_NUM));
	
	ETS_GPIO_INTR_ATTACH(human_intr_handler,NULL);
	ETS_GPIO_INTR_DISABLE();
	gpio_output_set(0, 0, 0, GPIO_ID_PIN(SENSOR_INFRARED_LED_IO_NUM));
	gpio_register_set(GPIO_PIN_ADDR(SENSOR_INFRARED_LED_IO_NUM), GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)
						  | GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE)
						  | GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(SENSOR_INFRARED_LED_IO_NUM));
	gpio_pin_intr_state_set(GPIO_ID_PIN(SENSOR_INFRARED_LED_IO_NUM), GPIO_PIN_INTR_ANYEDGE);
	ETS_GPIO_INTR_ENABLE();
}
void ICACHE_FLASH_ATTR
user_sensor_init(void)
{
	//led on/off set
	PIN_FUNC_SELECT(LED_ONOFF_IO_MUX, LED_ONOFF_IO_FUNC);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(LED_ONOFF_IO_NUM), LEVEL_L);

	//pwd set
	uint32 pwm_duty_init[PWM_CHANNEL_NUM] = {0};
	uint32 io_info[][3] = {   
		{LED_PWM_IO_MUX,LED_PWM_IO_FUNC,LED_PWM_IO_NUM},
	};
	pwm_init(PWM_PERIOD_NUM,  pwm_duty_init ,PWM_CHANNEL_NUM,io_info);
	set_pwm_debug_en(0);
	os_printf("PWM version : %08x \r\n",get_pwm_version());
	
	user_reset_pwm_duty(0, 0);

	//i2c light read
	os_timer_disarm(&illuminance_timer);
	os_timer_setfn(&illuminance_timer, (os_timer_func_t *)get_illuminance_data_cb, NULL);
	os_timer_arm(&illuminance_timer, 5*1000, 0);
	
	//human intterupt init
	os_timer_disarm(&human_timer);
	os_timer_setfn(&human_timer, (os_timer_func_t *)human_intterupt_init, NULL);
	os_timer_arm(&human_timer, 60*1000, 0);

	system_os_task(Senser_Task, USER_TASK_PRIO_0, senser_procTaskQueue, SENSER_QUEUE_SIZE);
}
