#ifndef __USER_SENSOR_H__
#define __USER_SENSOR_H__

#define LEVEL_H										(1)
#define LEVEL_L										(0)

#define GPIO_I_INTER_IP_NUM							(4)
//Control LED on/off
#define GPIO_O_CTRL_LED								(15)
//Light sensor:high level=>on, low level=>off
#define GPIO_I_LX_ENABLE							(14)


void user_sensor_init(void);

#endif
