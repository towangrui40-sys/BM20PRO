#ifndef __MOTOR_CMD_H_
#define __MOTOR_CMD_H_


typedef struct
{
	//set paras
	//human-machine com
	u8 station;
	//break_res
	u8 brk_res_voltage;
	u8 brk_res_duty;
	u8 brk_res_interval;
	u8 brk_res_pwm;
	// foc/vf
	u8 motor_type;
	//work paras
	u8 run;	
	// monitor paras
	u16 bus_vol;
	u16 bus_cur;
	u16 uphase_cur;
	u16 vphase_cur;
	u16 wphase_cur;
	u16 heatsink_temp;
	
	u16	warning_state;

}ST_MOTOR_PARAS_DEF;

typedef enum
{
  MC_IDLE = 0, 
	MC_WORK,
}MC_GetState;

typedef enum
{
  MC_FWD = 0X01,
	MC_REV = 0X02,
	MC_STOP = 0X04,
}MC_Cmd;

//void motor_set_station(u8 station);
void motor_set_run(u16 paras);
//void motor_set_rpm(u16 rpm);
//void motor_set_door(u16 enable);

//void motor_get_runstate(MC_GetState state);
//void motor_get_rpm(void);
//void motor_get_door_lock(void);
//void motor_get_roller(void);


#endif


