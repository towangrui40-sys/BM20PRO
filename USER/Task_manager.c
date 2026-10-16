//############################################################
// FILE:  Task_manager.c
// Created on: 2022��2��20��
// Author: Jason
// summary: Task_manager
//############################################################
#include "main.h"

void Send_voltage_service(void);
void Send_voltage_service_spd(u16 spd);

#define Task_Num   4   //task +1
#define HFPeriod_COUNT     1   	//1ms
#define RTUPeriod_COUNT    50    //50ms
#define FSMPeriod_COUNT    5  	//5ms
#define TIMERPeriod_COUNT  250   //250ms

TaskTime xdata TasksPare[Task_Num];
extern code const Product_TypeDef product_data[PRODUCT_TOTAL];
void Timer_Task_Count(void)
{
	u8 data Task_Count=0;
 for(Task_Count=0;Task_Count<Task_Num;Task_Count++)    //TASK_NUM=5
 {
  if((TasksPare[Task_Count].Task_Count<TasksPare[Task_Count].Task_Period)&&(TasksPare[Task_Count].Task_Period>0))
  {
	  TasksPare[Task_Count].Task_Count++;
  }
 }
}

void Execute_Task_List_RUN(void)
{
	u8 data	Task_Count = 0;
 for(Task_Count=0;Task_Count < Task_Num;Task_Count++)
 {
  if((TasksPare[Task_Count].Task_Count>=TasksPare[Task_Count].Task_Period)&&(TasksPare[Task_Count].Task_Period > 0))
  {
		TasksPare[Task_Count].Task_Function();
	  TasksPare[Task_Count].Task_Count=0;
  }
 }
}
#define SPK_TIME_OUT_TIMES 5
void Task_Timer_Sec(void)
{
	static u16 xdata temp = 0;
	static u16 xdata tick = 0;
	static u16 xdata spk = 0;
	static u16 xdata spk_tick = 0;
	static u16 xdata Once_flag = 0;
	if(win_str_data.state == FSM_READY)
	{
		win_str_data.run_timer_state = 0;
		if(spk)
		{
			spk--;	
			spk_tick = 1-spk_tick;
			if((spk_tick)&&(win_str_data.alarm == 0))
			sys_speaker_work(20);
		}
	}
	
	
	if(win_str_data.state == FSM_RUN) 
	{
		tick++;	
		if(tick >= 4) 
		{
			tick = 0;
			temp = win_sec_timer_handle(&win_str_data.run_timer_state);
			if(temp == 2)
			{
				if(win_str_data.use_dir == 0)
				{
					RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
				}
				else
				{
					if(win_str_data.nm_dir_flag == 0 && Once_flag == 0)
					{
						RTU_Cmd_Pending(RTU_RUN_REV,MC_REV);
						win_str_data.nm_dir_flag = 1;
						Once_flag = 1;
					}
					if(win_str_data.nm_dir_flag == 1 && Once_flag == 0)
					{
						RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
						win_str_data.nm_dir_flag = 0;
						Once_flag = 1;
					}
				}
			}
			else if(temp == 1)
			{
				RTU_Cmd_Pending(RTU_STOP,MC_STOP);
				Once_flag = 0;
			}
			else if (temp == 3) spk = 2*SPK_TIME_OUT_TIMES;
			
			win_str_data.used_time++;
			
		}
	}
	sys_write_vp(0x200b,(u8 *)&win_str_data.state,1);

/*********��ʱ5min�ط���*************/
	if(win_str_data.state == FSM_READY)
	{
		win_str_data.fan_tick++;
		if(win_str_data.fan_tick >= 1200)
		{
			win_str_data.fan_tick = 0;
		}
	}	
	if(win_str_data.bowl_tick > 0) win_str_data.bowl_tick --;
	if(win_str_data.fsm_tick > 0) win_str_data.fsm_tick --;
}

void RTU_send_Rece(void)
{
	RTU_Send_Task_Handler();
}

void Run_FSM(void)
{
	

}
void HFPeriod_1msTask(void)
{
	btn_click_handler();
	rtu_response_handler();
	win_fresh_window(&str_sys);
}


void Task_Manage_List_Init(void)
{
	TasksPare[0].Task_Period = HFPeriod_COUNT; //PERIOD_COUNT=1    1ms
	TasksPare[0].Task_Count = 1;
	TasksPare[0].Task_Function = HFPeriod_1msTask;  

	TasksPare[1].Task_Period = RTUPeriod_COUNT; //2ms
	TasksPare[1].Task_Count = 8;
	TasksPare[1].Task_Function = RTU_send_Rece;

	TasksPare[2].Task_Period = FSMPeriod_COUNT; //100ms
	TasksPare[2].Task_Count=80;
	TasksPare[2].Task_Function = Run_FSM;

	TasksPare[3].Task_Period = TIMERPeriod_COUNT;//500ms
	TasksPare[3].Task_Count=300;
	TasksPare[3].Task_Function = Task_Timer_Sec;
}

void Send_voltage_service(void)
{
	u16 vol_temp;
	//vol_temp = win_level_trans_volt(win_str_data.use_level);
	if(str_sys.mc_para.set_vol != vol_temp)
	{
		str_sys.mc_para.set_vol = vol_temp;
		RTU_Cmd_Pending(RTU_SET_VLOTAGE,vol_temp);
	}
}
void Send_voltage_service_spd(u16 spd)
{
	u16 vol_temp;
	vol_temp = spd;
	if(str_sys.mc_para.set_vol != vol_temp)
	{
		str_sys.mc_para.set_vol = vol_temp;
		RTU_Cmd_Pending(RTU_SET_VLOTAGE,vol_temp);
	}
}
// USER CODE END

