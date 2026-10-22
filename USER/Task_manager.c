//############################################################
// FILE:  Task_manager.c
// summary: Task_manager
//############################################################
#include "main.h"

#define Task_Num   4   //task +1
#define HFPeriod_COUNT     1   	//1ms
#define RTUPeriod_COUNT    50    //50ms
#define FSMPeriod_COUNT    200   //200ms
#define TIMERPeriod_COUNT  250   //250ms

void Send_voltage_service(u16 send_speed);
void tank1_material(void);
void tank2_material(void);

#define RED     0x01
#define GREEN   0x00

TaskTime xdata TasksPare[Task_Num];
extern code const Product_TypeDef product_data[PRODUCT_TOTAL];

void Timer_Task_Count(void)
{
	u8 data Task_Count = 0;
	for(Task_Count = 0;Task_Count < Task_Num;Task_Count++)
	{
		if((TasksPare[Task_Count].Task_Count < TasksPare[Task_Count].Task_Period) && (TasksPare[Task_Count].Task_Period > 0))
		{
			TasksPare[Task_Count].Task_Count++;
		}
	}
}

void Execute_Task_List_RUN(void)
{
	u8 data Task_Count = 0;
	for(Task_Count = 0;Task_Count < Task_Num;Task_Count++)
	{
		if((TasksPare[Task_Count].Task_Count >= TasksPare[Task_Count].Task_Period) && (TasksPare[Task_Count].Task_Period > 0))
		{
			TasksPare[Task_Count].Task_Function();
			TasksPare[Task_Count].Task_Count = 0;
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
			spk_tick = 1 - spk_tick;
			if((spk_tick) && (win_str_data.alarm == 0))
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
			else if(temp == 3)
			{
				spk = 2 * SPK_TIME_OUT_TIMES;
			}

			win_str_data.used_time++;
		}
	}

	sys_write_vp(0x200b,(u8 *)&win_str_data.state,1);

	if(win_str_data.state == FSM_READY)
	{
		win_str_data.fan_tick++;
		if(win_str_data.fan_tick >= 1200)
		{
			RTU_Cmd_Pending(RTU_ACFAN_STOP,0);
			win_str_data.fan_tick = 0;
		}
	}
	if(win_str_data.bowl_tick > 0) win_str_data.bowl_tick--;
	if(win_str_data.fsm_tick > 0) win_str_data.fsm_tick--;
}

void RTU_send_Rece(void)
{
	RTU_Send_Task_Handler();
}

void Run_FSM(void)
{
	u16 xdata temp;
	static u16 xdata init_lock_done = 0;
	static u16 xdata init_step = 0;
	switch(win_str_data.state)
	{
		case FSM_INIT:
				switch(init_step)
				{
					case 0: //清空两个罐材质显示，进入延时
						init_lock_done = 0;
						win_str_data.fsm_tick = 8;
						init_step = 1;
						break;

					case 1: //延时结束，判断是否在1/2号位且有罐压锁
						if(win_str_data.fsm_tick == 0)
						{
							if((str_sys.location_test == 0x01 || str_sys.location_test == 0x02) && str_sys.tank_lock == 0)   //unlock
							{
								RTU_Cmd_Pending(RTU_SET_LED,RED);
								win_change_page(POPUP_PRESS_HANDLE);   //请下压把手 完成开机复位页面。
								init_step = 2;
							}
							else
							{
								RTU_Cmd_Pending(RTU_SET_LED,GREEN);
								win_change_page(POPUP_LOCK_RESET);    //等待机器复位界面
								init_step = 4;
							}
						}
						break;

					case 2: //压锁状态：先上锁
						if(str_sys.tank_lock == 1)//下压把手 开始上锁
						{
							RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK);
							win_change_page(POPUP_LOCKING);
							init_lock_done = 1;
							init_step = 3;
						}
						break;

					case 3: //等待锁到位；若仍压住则解锁重试，否则进入关盖流程
						if(str_sys.lock_state == 1)
						{
							if(str_sys.tank_lock == 0)  //锁紧失败
							{
								RTU_Cmd_Pending(RTU_SET_LED,RED);
								RTU_Cmd_Pending(RTU_LOCK_ACTION,UNLOCK);
								win_change_page(SELECT_TANK_PAGE);
								init_step = 2;
							}
							else
							{
								RTU_Cmd_Pending(RTU_SET_LED,GREEN);
								win_change_page(POPUP_LOCK_RESET);
								win_change_page(SELECT_TANK_PAGE);
								init_step = 4;
							}
						}
						break;

					case 4: //盖子未关则发关盖命令
						if(str_sys.case_state != 0)
						{
							RTU_Cmd_Pending(RTU_CASE_ACTION,LOCK);
						}
						init_step = 5;
						break;

					case 5: //等盖子关到位后启动电机转到2号位
						if(str_sys.case_state == 0)
						{
							RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);

							win_str_data.fsm_tick = 4;
							init_step = 6;
						}
						break;

					case 6: //转到2号位后停电机并开盖，决定是否还需上锁
						if(str_sys.location_test == 0x02 && win_str_data.fsm_tick == 0)
						{
							RTU_Cmd_Pending(RTU_STOP,MC_STOP);
							RTU_Cmd_Pending(RTU_CASE_ACTION,UNLOCK);
							init_step = init_lock_done ? 8 : 7;
						}
						break;

					case 7: //补一次上锁
						RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK);
						init_step = 8;
						break;

					case 8: //等锁到位后请求开盖
						if(str_sys.lock_state == 1)
						{
							win_str_data.lid_cmd = 0x01;
							init_step = 9;
						}
						break;

					case 9: //盖子开到位后进入选罐界面
						if(str_sys.case_state == 0x01)
						{
							win_change_page(SELECT_TANK_PAGE);
							win_str_data.state = FSM_SEL_BOWL;
							init_step = 0;
						}
						break;

					default: //异常回到初始步骤
						init_step = 0;
						break;
				}
				break;
		case FSM_READY:
			win_str_data.disp_cycle = win_str_data.use_cycle;
			win_str_data.disp_run_time = win_str_data.use_run_time;
			win_str_data.disp_pause_time = win_str_data.use_pause_time;
			win_str_data.disp_count_time = ((((u32)win_str_data.use_cycle * (win_str_data.use_run_time + win_str_data.use_pause_time)) - win_str_data.use_pause_time) * 60);
			win_disp_level(MAIN_TIMER,win_str_data.use_level);


			if(win_str_data.start == 1)    //接收到Start_win  start=1开始信号
			{
				RTU_Cmd_Pending(RTU_ACFAN_RUN,1);
				win_str_data.state = FSM_START;
			}
			else
			{
				win_str_data.start = 0;
			}

			if(win_str_data.lid_cmd == 0x11)	//open lid
			{
				win_change_page(PLEASE_WAIT_PAGE);   //切换检测转盘位置界面
				Send_voltage_service(18);  //send motor speed
				RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
				win_str_data.lid_cmd = 0x01;
			}
			break;

		case FSM_START:
			Send_voltage_service(win_str_data.use_level);
			RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
			win_str_data.nm_dir_flag = 0;
			win_str_data.state = FSM_RUN;
			break;

		case FSM_CHECK_POSITION:
			if(str_sys.case_state == 0x01)  // lid open in place
			{
				Send_voltage_service(23);
				win_change_page(SELECT_TANK_PAGE);
				win_str_data.state = FSM_SEL_BOWL;
			}
			break;

		case FSM_SEL_BOWL:
			if(win_str_data.sel_bowll == 1 && win_str_data.bowl_tick == 0)
			{
				RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
				win_str_data.bowl_tick = 12;
			}
			else if(win_str_data.sel_bowll == 2 && win_str_data.bowl_tick == 0)
			{
				RTU_Cmd_Pending(RTU_RUN_REV,MC_REV);
				win_str_data.bowl_tick = 12;
			}

			if(win_str_data.sel_bowll == str_sys.location_test)
			{
				win_str_data.bowl_tick = 0;
				win_str_data.sel_bowll = 0;
			}

			temp = (str_sys.location_test == 0x01);
			sys_write_vp(ICON_BOWL_SEL1_ADDR,(u8 *)&temp,1);
			temp = (str_sys.location_test == 0x02);
			sys_write_vp(ICON_BOWL_SEL2_ADDR,(u8 *)&temp,1);

			if(win_str_data.bowl_tick == 0 && str_sys.mc_para.set_run != MC_STOP)
			{
				str_sys.mc_para.set_run = MC_STOP;
				RTU_Cmd_Pending(RTU_STOP,MC_STOP);
			}
			if(win_str_data.lid_cmd == 0x10)   //close lid
			{
				win_str_data.lid_cmd = 0x00;
				win_change_page(PLEASE_WAIT_PAGE);   //等待关盖界面
				win_str_data.state = FSM_CASE_LOCK;
			}
			break;

		case FSM_RUN:
			if(str_sys.run_state >= (win_str_data.use_level - 5)) str_sys.run_state = win_str_data.use_level;
			win_disp_level(MAIN_TIMER,str_sys.run_state);

			if(str_sys.str_mems.z <= 0) str_sys.str_mems.z = 0;
			if(str_sys.str_mems.z > win_str_data.shake)
			{
				RTU_Cmd_Pending(RTU_STOP,MC_STOP);
				win_str_data.start = 0;
				win_str_data.pause = 1;
				win_str_data.shak_warning = 1;
				win_change_page(WORK_SHAKE_WARNING_PAGE);
			}
			
			Send_voltage_service(win_str_data.use_level);
			if(win_str_data.disp_count_time == 0)
			{
				win_str_data.start = 0;
			}

			if(win_str_data.start == 0 || win_str_data.pause == 1)
			{
				RTU_Cmd_Pending(RTU_STOP,MC_STOP);
				win_str_data.state = FSM_SLOW_DOWN;
				win_change_page(WORK_PAGE_NO_TOUCH);
			}
			break;

		case FSM_SLOW_DOWN:
			win_disp_level(MAIN_TIMER,str_sys.run_state);
			if(str_sys.run_state == 0)
			{
				win_change_page(WORK_PAGE);
			}
			break;

		case FSM_PAUSE:
			if((win_str_data.start == 1) && (win_str_data.pause == 0))
			{
				Send_voltage_service(win_str_data.use_level);

				if(win_str_data.nm_dir_flag == 0)
				{
					RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
				}
				else
				{
					RTU_Cmd_Pending(RTU_RUN_REV,MC_REV);
				}
				win_str_data.state = FSM_RUN;
			}

			if((win_str_data.start == 0) && (win_str_data.pause == 0))
			{
				win_str_data.state = FSM_STOP;
			}

			if(win_str_data.lid_cmd == 0x11) // open lid
			{
				win_change_page(PLEASE_WAIT_PAGE);
				Send_voltage_service(18);
				RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
				win_str_data.lid_cmd = 0x01;
			}
			break;

		case FSM_STOP:
			win_str_data.fan_tick = 0;
			win_str_data.used_time_l = win_str_data.used_time & 0xffff;
			win_str_data.used_time_h = (win_str_data.used_time>>16) & 0xffff;
			norflash_write(RUNTIME_MIN_HADDR,(u8*)&win_str_data.used_time_h,2);

			win_disp_run_time(MAIN_TIMER,win_str_data.use_run_time);
			win_disp_pause_time(MAIN_TIMER,win_str_data.use_pause_time);
			win_disp_cycle(MAIN_TIMER,win_str_data.use_cycle);
			win_str_data.disp_count_time = ((((u32)win_str_data.use_cycle * (win_str_data.use_run_time + win_str_data.use_pause_time)) - win_str_data.use_pause_time) * 60);
			win_disp_count_time(MAIN_TIMER,win_str_data.disp_count_time);

			win_str_data.state = FSM_READY;
			break;

		default:
			break;
	}
}

void HFPeriod_1msTask(void)
{
	btn_click_handler();
	rtu_response_handler();
	win_fresh_window(&str_sys);

	if(win_str_data.state == FSM_READY || win_str_data.state == FSM_PAUSE) 
	{
		if(str_sys.location_test == 0x02 && win_str_data.lid_cmd == 0x01)  // turn to No.2 seat  open the lid
		{
			win_str_data.lid_cmd = 0x00;
			win_change_page(PLEASE_WAIT_PAGE);   //切换等待开盖界面
			RTU_Cmd_Pending(RTU_STOP,MC_STOP);
			RTU_Cmd_Pending(RTU_CASE_ACTION,UNLOCK); //send unlock lid
			win_str_data.state = FSM_CHECK_POSITION;
		}
	}
}

void Task_Manage_List_Init(void)
{
	TasksPare[0].Task_Period = HFPeriod_COUNT;
	TasksPare[0].Task_Count = 1;
	TasksPare[0].Task_Function = HFPeriod_1msTask;

	TasksPare[1].Task_Period = RTUPeriod_COUNT;
	TasksPare[1].Task_Count = 8;
	TasksPare[1].Task_Function = RTU_send_Rece;

	TasksPare[2].Task_Period = FSMPeriod_COUNT;
	TasksPare[2].Task_Count = 80;
	TasksPare[2].Task_Function = Run_FSM;

	TasksPare[3].Task_Period = TIMERPeriod_COUNT;
	TasksPare[3].Task_Count = 300;
	TasksPare[3].Task_Function = Task_Timer_Sec;
}

void Send_voltage_service(u16 send_speed)
{
	u16 vol_temp;
	vol_temp = win_level_trans_volt(send_speed);

	if(vol_temp != str_sys.mc_para.set_vol)
	{
		str_sys.mc_para.set_vol = vol_temp;
		RTU_Cmd_Pending(RTU_SET_VLOTAGE,vol_temp);
	}
}