#ifndef __MOTOR_CMD_H_
#define __MOTOR_CMD_H_

typedef enum
{
	MC_FWD = 0X01,
	MC_REV = 0X02,
	MC_STOP = 0X04,
}MC_Cmd;


void motor_set_run(u16 paras);


#endif


