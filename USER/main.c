#include "main.h"
#include "string.h"


/**************************************************
触摸地址设置在#include "func_handler.h"
显示变量地址在#include "start_win.h"

//点击按键处理回调函数
void btn_click_callback(u16 addr,u16 val)

**************************************************/
SysDataDef xdata str_sys;


int i=0;
void Init_Rtu_RegHoldingBuf(SysDataDef* dat)
{
	usRegHoldingBuf[ADDR_MOTOR_GET_FWD].dat = (u16 *)&dat->mc_para.fwd;
	usRegHoldingBuf[ADDR_MOTOR_GET_REV].dat = (u16 *)&dat->mc_para.rev;
	usRegHoldingBuf[ADDR_MOTOR_GET_RESET].dat = (u16 *)&dat->mc_para.reset;
	usRegHoldingBuf[ADDR_MOTOR_GET_FAULT].dat = (u16 *)&dat->mc_para.fault;
	usRegHoldingBuf[ADDR_MOTOR_SET_RUN].dat = (u16 *)&dat->mc_para.set_run;
	usRegHoldingBuf[ADDR_MOTOR_SET_VOL].dat = (u16 *)&dat->mc_para.set_vol;

	usRegHoldingBuf[ADDR_GET_MEMSX].dat = (u16 *)&dat->str_mems.x;
	usRegHoldingBuf[ADDR_GET_MEMSY].dat = (u16 *)&dat->str_mems.y;
	usRegHoldingBuf[ADDR_GET_MEMSZ].dat = (u16 *)&dat->str_mems.z;
	
	usRegHoldingBuf[ADDR_RUN_STATE].dat = (u16 *)&dat->run_state;
	usRegHoldingBuf[ADDR_CUR].dat = (u16 *)&dat->current;
	
	usRegHoldingBuf[ADDR_SET_LED].dat = (u16 *)&dat->set_led;
	
	usRegHoldingBuf[ADDR_SET_AC1].dat = (u16 *)&dat->set_ac1;
	usRegHoldingBuf[ADDR_SET_AC2].dat = (u16 *)&dat->set_ac2;
	usRegHoldingBuf[ADDR_SET_AC3].dat = (u16 *)&dat->set_ac3;

	usRegHoldingBuf[ADDR_SET_CASE].dat = (u16 *)&dat->set_case;
	usRegHoldingBuf[ADDR_SET_LOCK].dat = (u16 *)&dat->set_lock;
	usRegHoldingBuf[ADDR_CASE_STATE].dat = (u16 *)&dat->case_state;  //盖子状态
	usRegHoldingBuf[ADDR_LOCK_STATE].dat = (u16 *)&dat->lock_state;  //锁状态
	
	usRegHoldingBuf[ADDR_BOWL1_NUM].dat = (u16 *)&dat->bowl_num;
	usRegHoldingBuf[ADDR_BOWL2_NUM].dat = (u16 *)&dat->bowl_test_flag;
	usRegHoldingBuf[ADDR_MAGNET_NUM].dat = (u16 *)&dat->magnet_num;

	usRegHoldingBuf[ADDR_TANK1_TEMP].dat = (u16 *)&dat->tank1_temp;
	usRegHoldingBuf[ADDR_TANK1_PRE].dat = (u16 *)&dat->tank1_pre;
	usRegHoldingBuf[ADDR_TANK2_TEMP].dat = (u16 *)&dat->tank2_temp;
	usRegHoldingBuf[ADDR_TANK2_PRE].dat = (u16 *)&dat->tank2_pre;
	
	usRegHoldingBuf[ADDR_DOOR].dat = (u16 *)&dat->case_open;//	  PAin(0) 
	usRegHoldingBuf[ADDR_LOCK].dat = (u16 *)&dat->case_close;//   PAin(1)		
	usRegHoldingBuf[ADDR_OPEN].dat = (u16 *)&dat->tank_lock;   //按压后上锁
}

void main(void)
{ 
	sys_init();//系统初始化
	Init_Rtu_RegHoldingBuf(&str_sys);
	rtu_data_init();
	btn_init();        
	T5L_HMIslvInit(); //初始化通讯参数
  EA=1;
	Task_Manage_List_Init( );  // 事件任务函数初始化
	win_init();
	while(1)
	{
		Execute_Task_List_RUN();
		T5L_HMIslvSub(); //功能：在线更新触摸屏程序
		
	}
}



















