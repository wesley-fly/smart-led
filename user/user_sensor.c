#include "c_types.h"
#include "eagle_soc.h"
#include "gpio.h"

#include "user_sensor.h"
#define SENSER_QUEUE_SIZE 4
os_event_t senser_procTaskQueue[SENSER_QUEUE_SIZE];

LOCAL void ICACHE_FLASH_ATTR Senser_Task(os_event_t *events)
{
}

void ICACHE_FLASH_ATTR
user_sensor_init(void)
{
	gpio_init();
	
	// GPIO4 input
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(GPIO_I_INTER_IP_NUM));
	// GPIO15 out put
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
	GPIO_OUTPUT_SET(GPIO_ID_PIN(GPIO_O_CTRL_LED), LEVEL_H);
	// GPIO14 INPUT
	PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
	GPIO_DIS_OUTPUT(GPIO_ID_PIN(GPIO_I_LX_ENABLE));
	system_os_task(Senser_Task, 0, senser_procTaskQueue, SENSER_QUEUE_SIZE);
	/*system_os_task(bopin_light_sysTask, 0, __bopin_sysTaskQueue, 32);

	ETS_GPIO_INTR_DISABLE();
	ETS_GPIO_INTR_ATTACH(bopin_gpio_intr_handle, NULL);
	gpio_pin_intr_state_set(GPIO_ID_PIN(GPIO_I_INTER_IP_NUM), GPIO_PIN_INTR_POSEDGE);
	GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(GPIO_I_INTER_IP_NUM));
	ETS_GPIO_INTR_ENABLE();*/
}
