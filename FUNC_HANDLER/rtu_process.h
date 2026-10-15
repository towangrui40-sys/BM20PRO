#ifndef __RTU_PROCESS_H_
#define __RTU_PROCESS_H_


//TRU CMD    //RTU状态
typedef enum
{
	RTU_IDlE = 0,        
	RTU_START,        
	RTU_STOP,              
	RTU_ERROR,            
	RTU_SET_VLOTAGE,  //设置电压
	RTU_RUN_FWD, //正转运行
	RTU_RUN_REV, //反转运行	
//DC	
	RTU_SET_LED,

	RTU_CASE_ACTION,
	RTU_LOCK_ACTION,
	RTU_LOCK_STATE,
	RTU_ACFAN_RUN,
	RTU_ACFAN_STOP,
	
}RTU_StateType;

void RTU_Send_Task_Handler(void);

void RTU_Cmd_Pending(u8 cmd,u16 dat);
#endif
