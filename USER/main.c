#include "main.h"
#include "string.h"


/**************************************************
������ַ������#include "func_handler.h"
��ʾ������ַ��#include "start_win.h"

//������������ص�����
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
	usRegHoldingBuf[ADDR_CASE_STATE].dat = (u16 *)&dat->case_state;
	usRegHoldingBuf[ADDR_LOCK_STATE].dat = (u16 *)&dat->lock_state;
	
	usRegHoldingBuf[ADDR_LOCATION_TEST].dat = (u16 *)&dat->location_test;
	usRegHoldingBuf[ADDR_INSTALL_TEST].dat = (u16 *)&dat->install_test;

}

void main(void)
{ 
	sys_init();//ϵͳ��ʼ��
	Init_Rtu_RegHoldingBuf(&str_sys);
	rtu_data_init();
	btn_init();        
	T5L_HMIslvInit(); //��ʼ��ͨѶ����
  EA=1;
	Task_Manage_List_Init( );  // �¼���������ʼ��
	win_init();
	while(1)
	{
		Execute_Task_List_RUN();
		T5L_HMIslvSub(); //���ܣ����߸��´���������
		
	}
}



















