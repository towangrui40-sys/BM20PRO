#include "main.h"
#include "string.h"


SysDataDef xdata str_sys;


int i=0;
void Init_Rtu_RegHoldingBuf(SysDataDef* dat)
{
	usRegHoldingBuf[ADDR_GET_MEMSX].dat = (u16 *)&dat->str_mems_x;
	usRegHoldingBuf[ADDR_GET_MEMSY].dat = (u16 *)&dat->str_mems_y;
	usRegHoldingBuf[ADDR_GET_MEMSZ].dat = (u16 *)&dat->str_mems_z;
	
	usRegHoldingBuf[ADDR_SET_MOTOR].dat = (u16 *)&dat->set_motor;
	usRegHoldingBuf[ADDR_SET_SPEED].dat = (u16 *)&dat->set_speed;	
	usRegHoldingBuf[ADDR_READ_SPEED].dat = (u16 *)&dat->read_speed;		
	usRegHoldingBuf[ADDR_MOTOR_STATE].dat = (u16 *)&dat->motor_state;	
	usRegHoldingBuf[ADDR_MOTOR_CURRENT].dat = (u16 *)&dat->motor_current;	

	usRegHoldingBuf[ADDR_SET_LED].dat = (u16 *)&dat->set_led;
	
	usRegHoldingBuf[ADDR_SET_AC1].dat = (u16 *)&dat->set_ac1;
	usRegHoldingBuf[ADDR_SET_AC2].dat = (u16 *)&dat->set_ac2;
	usRegHoldingBuf[ADDR_SET_AC3].dat = (u16 *)&dat->set_ac3;

	usRegHoldingBuf[ADDR_SET_CASE].dat = (u16 *)&dat->set_case;
	usRegHoldingBuf[ADDR_SET_LOCK].dat = (u16 *)&dat->set_lock;
	usRegHoldingBuf[ADDR_CASE_STATE].dat = (u16 *)&dat->case_state;
	usRegHoldingBuf[ADDR_LOCK_STATE].dat = (u16 *)&dat->lock_state;
	
	usRegHoldingBuf[ADDR_LOCATION_TEST].dat = (u16 *)&dat->location_test;
	usRegHoldingBuf[ADDR_TANK_LOCK].dat = (u16 *)&dat->tank_lock;

}

void main(void)
{ 
	sys_init();
	Init_Rtu_RegHoldingBuf(&str_sys);
	rtu_data_init();
	btn_init();        
	T5L_HMIslvInit();
    EA=1;
	Task_Manage_List_Init( );
	win_init();
	while(1)
	{
		Execute_Task_List_RUN();
		T5L_HMIslvSub();
		
	}
}



















