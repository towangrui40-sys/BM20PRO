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
#define GREEN   0x02
#define POWER_ON_DELAY_TICKS  12   //12 * 250ms = 3s
#define MOTOR_STOP_DELAY_TICKS 40  //20 * 250ms = 5s，等待机械完全停下

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
		win_reset_grind_timer();
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

static void start_pos2_detect(void)
{
	win_change_page(POPUP_DETECT_TURN);
	Send_voltage_service(20);
	RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
	win_str_data.state = FSM_INIT_LEAVE_POS2;
}

static void start_init_reclose(void)
{
	RTU_Cmd_Pending(RTU_CASE_ACTION,CLOSE);
	win_change_page(POPUP_CASE);
	win_str_data.state = FSM_INIT_RECLOSE;
}

static void start_init_unlock(void)
{
	RTU_Cmd_Pending(RTU_SET_LED,RED);
	RTU_Cmd_Pending(RTU_LOCK_ACTION,UNLOCK);
	win_change_page(POPUP_LOCK_RESET);
	win_str_data.state = FSM_INIT_UNLOCKING;
}



void Run_FSM(void)
{
	u16 xdata temp;

	switch(win_str_data.state)
	{
		case FSM_POWER_ON:
			win_str_data.fsm_tick = POWER_ON_DELAY_TICKS;
			win_str_data.state = FSM_POWER_ON_DELAY;
			break;

		case FSM_POWER_ON_DELAY:
			if(win_str_data.fsm_tick != 0)              //等待控制器状态稳定
			{
				break;
			}

			if(str_sys.case_state == CLOSE_PLACE)       //盖子已关，直接找位置后重开
			{
				start_pos2_detect();
			}
			else if(str_sys.location_test != 0x01 && str_sys.location_test != 0x02)
			{
				//不在1/2号位时直接关盖，关盖后再找位置
				start_init_reclose();
			}
			else                                        //在1/2号位，统一判断锁状态
			{
				win_str_data.state = FSM_INIT_LOCK_CHECK;
			}
			break;

		case FSM_INIT_LOCK_CHECK:
			if(str_sys.tank_lock == 0)
			{
				start_init_reclose();
			}
			else if(str_sys.lock_state == 1 && str_sys.tank_lock == 1)
			{
				start_init_unlock();
			}
			else
			{
				RTU_Cmd_Pending(RTU_SET_LED,RED);
				win_change_page(POPUP_PRESS_HANDLE);          //请下压把手
				win_str_data.state = FSM_INIT_WAIT_LOCK;
			}
			break;

		case FSM_INIT_WAIT_LOCK:
			if(str_sys.tank_lock == 0)
			{
				win_change_page(POPUP_LOCKING);            //正在锁紧
				RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK);
				win_str_data.state = FSM_INIT_LOCKING;
			}
			break;

		case FSM_INIT_LOCKING:
			if(str_sys.lock_state == 1)
			{
				if(str_sys.tank_lock == 0)
				{
					start_init_reclose();                 //电机动作和传感器都到位
				}
				else                                      //锁到了但没锁好，自动解锁
				{
					start_init_unlock();
				}
			}
			break;

		case FSM_INIT_UNLOCKING:
			if(str_sys.lock_state == 0 && str_sys.tank_lock == 1)
			{
				win_change_page(POPUP_PRESS_HANDLE);
				win_str_data.state = FSM_INIT_WAIT_LOCK;  //解锁完成，等待重新按压
			}
			break;

		case FSM_INIT_RECLOSE:
			if(str_sys.case_state == CLOSE_PLACE)        //关盖到位后重新检测
			{
				start_pos2_detect();
			}
			break;

		case FSM_INIT_LEAVE_POS2:
			if(str_sys.location_test != 0x02)           //已经离开2号位
			{
				win_str_data.state = FSM_INIT_WAIT_POS2;
			}
			break;

		case FSM_INIT_WAIT_POS2:
			//等待HFPeriod_1msTask检测重新到达2号位
			break;

		case FSM_POSITION_CHECK:
			if(str_sys.case_state == OPEN_PLACE)        //初始化开盖完成
			{
				win_str_data.lid_cmd = 0x01;
				RTU_Cmd_Pending(RTU_SET_LED,RED);
				win_change_page(POPUP_LOCK_RESET);
				win_str_data.state = FSM_LOCK_RESET;
			}
			break;

		case FSM_LOCK_RESET:
				RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK);
				win_str_data.state = FSM_LOCK_RELOCK;
			break;

		case FSM_LOCK_RELOCK:
			if(str_sys.lock_state == 1)
			{
				win_change_page(SELECT_TANK_PAGE);
				win_str_data.state = FSM_TANK_SELECT;
			}
			break;

		case FSM_TANK_SELECT:
			temp = (str_sys.location_test == 0x01);
			sys_write_vp(ICON_BOWL_SEL1_ADDR,(u8 *)&temp,1);
			temp = (str_sys.location_test == 0x02);
			sys_write_vp(ICON_BOWL_SEL2_ADDR,(u8 *)&temp,1);

			if(win_str_data.lid_cmd == 0x10)            //请求关盖，优先处理
			{
				win_str_data.sel_bowll = 0;
				win_str_data.bowl_tick = 0;

				if(str_sys.location_test == 0x01)       //在1号位，反转离开
				{
					Send_voltage_service(23);
					RTU_Cmd_Pending(RTU_RUN_REV,MC_REV);
				}
				else if(str_sys.location_test == 0x02)  //在2号位，正转离开
				{
					Send_voltage_service(23);
					RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
				}

				win_str_data.lid_cmd = 0x00;
				win_str_data.state = FSM_CLOSE_CLEAR_TANK_POS;
			}
			else if(win_str_data.tank_lock_action == 1) //选择页开锁
			{
				win_str_data.tank_lock_action = 0;
				if(str_sys.location_test == 0x01 || str_sys.location_test == 0x02)
				{
					RTU_Cmd_Pending(RTU_SET_LED,RED);
					RTU_Cmd_Pending(RTU_LOCK_ACTION,UNLOCK);
					win_str_data.state = FSM_TANK_LOCK_WAIT_UNLOCK;
				}
			}
			else if(win_str_data.sel_bowll == 1)        //选择1号位
			{
				win_str_data.sel_bowll = 0;
				if(str_sys.location_test != 0x01)
				{
					win_str_data.bowl_tick = 4*4;
					str_sys.set_motor = MC_FWD;
					Send_voltage_service(20);
					RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
					win_str_data.state = FSM_TANK_MOVE_POS1;
				}
			}
			else if(win_str_data.sel_bowll == 2)        //选择2号位
			{
				win_str_data.sel_bowll = 0;
				if(str_sys.location_test != 0x02)
				{
					win_str_data.bowl_tick = 4*4;
					str_sys.set_motor = MC_REV;
					Send_voltage_service(20);
					RTU_Cmd_Pending(RTU_RUN_REV,MC_REV);
					win_str_data.state = FSM_TANK_MOVE_POS2;
				}
			}
			break;

		case FSM_TANK_MOVE_POS1:
			if(str_sys.location_test == 0x01 || win_str_data.bowl_tick == 0)
			{
				if(str_sys.set_motor != MC_STOP)
				{
					str_sys.set_motor = MC_STOP;
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);
				}
				win_str_data.bowl_tick = 0;
				win_str_data.state = FSM_TANK_SELECT;
			}
			break;

		case FSM_TANK_MOVE_POS2:
			if(str_sys.location_test == 0x02 || win_str_data.bowl_tick == 0)
			{
				if(str_sys.set_motor != MC_STOP)
				{
					str_sys.set_motor = MC_STOP;
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);
				}
				win_str_data.bowl_tick = 0;
				win_str_data.state = FSM_TANK_SELECT;
			}
			break;

		case FSM_TANK_LOCK_WAIT_UNLOCK:
			if(str_sys.lock_state == 0 && str_sys.tank_lock == 1)
			{
				win_str_data.state = FSM_TANK_LOCK_WAIT_PRESS;  //解锁完成，等待把手下压
			}
			break;

		case FSM_TANK_LOCK_WAIT_PRESS:
			if(str_sys.tank_lock == 0)                  //把手下压到位
			{
				win_change_page(POPUP_LOCKING);        //正在锁紧
				RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK);
				win_str_data.state = FSM_TANK_LOCK_WAIT_LOCK;
			}
			break;

		case FSM_TANK_LOCK_WAIT_LOCK:
			if(str_sys.lock_state == 1)
			{
				if(str_sys.tank_lock == 1)
				{
					RTU_Cmd_Pending(RTU_SET_LED,RED);
					RTU_Cmd_Pending(RTU_LOCK_ACTION,UNLOCK);
					win_str_data.state = FSM_TANK_LOCK_WAIT_UNLOCK;
				}
				else
				{
					RTU_Cmd_Pending(RTU_SET_LED,GREEN);
					win_str_data.sel_bowll = 0;
					win_change_page(SELECT_TANK_PAGE);
					win_str_data.state = FSM_TANK_SELECT;
				}
			}
			break;

		case FSM_CLOSE_CLEAR_TANK_POS:
			if(win_str_data.lid_cmd == 0x00)            //开盖状态
			{
				if(str_sys.location_test != 0x01 && str_sys.location_test != 0x02)
				{
					win_change_page(POPUP_CASE);    //等待界面，页面38
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);
					win_str_data.state = FSM_CLOSE_LOCK_TO_RUN;
					win_str_data.fsm_tick = 2;
				}
			}
			break;

		case FSM_CLOSE_LOCK_TO_RUN:
			if(win_str_data.fsm_tick == 0)
			{
				RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK_TO_RUN);
				win_str_data.state = FSM_CLOSE_SEND_CASE_CMD;
			}
			break;

		case FSM_CLOSE_SEND_CASE_CMD:
			if(str_sys.lock_state == 2)                //锁转到运行位
			{
				RTU_Cmd_Pending(RTU_CASE_ACTION,CLOSE); //关闭盖子
				win_str_data.state = FSM_CLOSE_WAIT_CASE_DONE;
			}
			break;

		case FSM_CLOSE_WAIT_CASE_DONE:
			if(str_sys.case_state == CLOSE_PLACE)      //盖子关到位
			{
				win_change_page(WORK_PAGE);
				if(win_str_data.pause == 1)            //暂停中的维护流程，保留本轮进度
				{
					win_disp_run_time(MAIN_TIMER,win_str_data.disp_run_time);
					win_disp_pause_time(MAIN_TIMER,win_str_data.disp_pause_time);
					win_disp_cycle(MAIN_TIMER,win_str_data.disp_cycle);
					win_disp_count_time(MAIN_TIMER,win_str_data.disp_count_time);
					win_disp_level(MAIN_TIMER,win_str_data.use_level);
					win_str_data.state = FSM_PAUSE;
				}
				else
				{
					win_str_data.state = FSM_READY;
				}
			}
			break;
		case FSM_READY:
		/**************显示值设定为设置值*******************/
				win_str_data.disp_cycle = win_str_data.use_cycle;
				win_str_data.disp_run_time = win_str_data.use_run_time;
				win_str_data.disp_pause_time = win_str_data.use_pause_time;
				win_str_data.disp_count_time = win_calc_total_time_sec();
				win_disp_level(MAIN_TIMER,win_str_data.use_level);
				/**************************************************/
				if(win_str_data.start == 1)    //接收到win_str_data.start = 1开始信号
				{
					win_str_data.state = FSM_START;
				}
				else
				{
					if(win_str_data.lid_cmd == 0x11)	//open lid
					{
						win_change_page(POPUP_DETECT_TURN);   //切换检测转盘位置界面
 						Send_voltage_service(18);
						RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
						
						win_str_data.lid_cmd = 0x01;
					}
				}

			break;
	case FSM_START:/*******************************************状态机：开始状态********************************/
					Send_voltage_service(win_str_data.use_level);
					RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);//BM反转模式第一次为正转
					win_str_data.nm_dir_flag = 0;
					
					win_str_data.state = FSM_RUN;   //切换到运行状态 update fsm
			break;
		case FSM_RUN:/*******************************************状态机：运行状态*************************************/   //运行过程中
				Send_voltage_service(win_str_data.use_level);
				if(win_str_data.disp_count_time == 0)
				{
					win_str_data.start = 0;
				}
				
			  if(win_str_data.pause == 1)/********** pause = 1暂停信号 pause = 0无暂停信号******************************/
				{
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);   //send motor stop
					win_str_data.fsm_tick = MOTOR_STOP_DELAY_TICKS;
					win_str_data.state = FSM_PAUSE_STOPPING;
					win_change_page(WORK_PAGE_NO_TOUCH);  //切换到主界面
				}							
			  else if(win_str_data.start == 0) //start = 0停止信号 start = 1 运行信号
				{
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);   //研磨电机停止

					win_str_data.fsm_tick = MOTOR_STOP_DELAY_TICKS;
					win_str_data.state = FSM_STOP_STOPPING;
					win_change_page(WORK_PAGE_NO_TOUCH);  //切换到主界面
				}
				if(str_sys.read_speed >= (win_str_data.use_level - 5)) str_sys.read_speed  = win_str_data.use_level;
				win_disp_level(MAIN_TIMER,str_sys.read_speed);
				if(str_sys.str_mems_z <= 0) str_sys.str_mems_z = 0;
				if(str_sys.str_mems_z > win_str_data.shake)    /**********************震动过大保护**************/
				{
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);   //send motor stop
					win_str_data.start = 0;
					win_str_data.pause = 1;
					win_str_data.shak_warning = 1; 
				}
/************************************************************************************************************/
			break;

		case FSM_PAUSE_STOPPING:
				win_disp_level(MAIN_TIMER,str_sys.read_speed);
				if(win_str_data.fsm_tick == 0)
				{
					if(win_str_data.shak_warning == 1)
						win_change_page(WORK_SHAKE_WARNING_PAGE);
					else
						win_change_page(WORK_PAGE);  //切换到主界面
					win_str_data.state = FSM_PAUSE;
				}
			break;

		case FSM_STOP_STOPPING:
				win_disp_level(MAIN_TIMER,str_sys.read_speed);
				if(win_str_data.fsm_tick == 0)
				{
					win_change_page(WORK_PAGE);
					win_str_data.state = FSM_STOP;
				}
			break;

		case FSM_PAUSE: /*****************************************状态机：暂停状态********************************/
				if((win_str_data.start == 1) && (win_str_data.pause == 0))     //点运行按钮,恢复运行
				{
					if(win_str_data.run_timer_state == 0)  //运行阶段才立即启动电机
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
					}
					win_str_data.state = FSM_RUN;   				//FSM to FSM_RUN
				}
				if((win_str_data.start == 0) && (win_str_data.pause == 0))      //接收到Start_win  start=0停止信号
				{
					win_str_data.state = FSM_STOP;
				}
				if(win_str_data.lid_cmd == 0x11)	//open lid
				{
					win_change_page(POPUP_DETECT_TURN);   //切换检测转盘位置界面
					Send_voltage_service(18);
					RTU_Cmd_Pending(RTU_RUN_FWD,MC_FWD);
					
					win_str_data.lid_cmd = 0x01;
				}
			break;

		case FSM_STOP:
			    win_str_data.fan_tick = 0;
				win_str_data.used_time_l = win_str_data.used_time & 0xffff;  //Save used time
				win_str_data.used_time_h = (win_str_data.used_time>>16) & 0xffff; 
				norflash_write(RUNTIME_MIN_HADDR,(u8*)&win_str_data.used_time_h,2);
				norflash_write(RUNTIME_MIN_LADDR,(u8*)&win_str_data.used_time_l,2);
		
				win_disp_run_time(MAIN_TIMER,win_str_data.use_run_time);
				win_disp_pause_time(MAIN_TIMER,win_str_data.use_pause_time);
				win_disp_cycle(MAIN_TIMER,win_str_data.use_cycle);
				win_str_data.disp_count_time = win_calc_total_time_sec();
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
	win_fresh_window();

	if(win_str_data.state == FSM_INIT_WAIT_POS2 && str_sys.location_test == 0x02)
	{
		win_change_page(POPUP_CASE);       //等待盖子到位
		RTU_Cmd_Pending(RTU_STOP,MC_STOP);
		RTU_Cmd_Pending(RTU_CASE_ACTION,OPEN);
		win_str_data.state = FSM_POSITION_CHECK;
	}

	if(win_str_data.state == FSM_READY || win_str_data.state == FSM_PAUSE) 
	{
		if(str_sys.location_test == 0x02 && win_str_data.lid_cmd == 0x01)  // turn to No.2 seat  open the lid
		{
			win_str_data.lid_cmd = 0x00;                //开盖命令只发送一次
			win_change_page(POPUP_CASE);   //等待盖子到位界面
			RTU_Cmd_Pending(RTU_STOP,MC_STOP);
			RTU_Cmd_Pending(RTU_CASE_ACTION,OPEN);  //发送开盖命令
			win_str_data.state = FSM_POSITION_CHECK; //等待开盖到位后复位锁紧装置，再进入选罐
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

	if(vol_temp != str_sys.set_speed)
	{
		str_sys.set_speed = vol_temp;
		RTU_Cmd_Pending(RTU_SET_SPEED,vol_temp);
	}
}
