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


		case FSM_POSITION_CHECK:
			if(str_sys.case_state == OPEN_PLACE)        //初始化开盖完成
			{
				RTU_Cmd_Pending(RTU_SET_LED,RED);
				win_change_page(POPUP_LOCK_RESET);
				win_str_data.state = FSM_LOCK_RESET;
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

		case FSM_INIT_LOCK_CHECK:
			if(str_sys.location_test == 0x01 || str_sys.location_test == 0x02)
			{
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
			}
			else if(str_sys.case_state == OPEN_PLACE)   //开盖但不在1/2号位
			{
				start_init_reclose();
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
		case FSM_TANK_SELECT:
			temp = (str_sys.location_test == 0x01);
			sys_write_vp(ICON_BOWL_SEL1_ADDR,(u8 *)&temp,1);
			temp = (str_sys.location_test == 0x02);
			sys_write_vp(ICON_BOWL_SEL2_ADDR,(u8 *)&temp,1);

			if(win_str_data.lid_cmd == 0x10)            //请求关盖，优先处理
			{
				if(str_sys.set_motor != MC_STOP)
				{
					str_sys.set_motor = MC_STOP;
					RTU_Cmd_Pending(RTU_STOP,MC_STOP);
				}
				win_str_data.sel_bowll = 0;
				win_str_data.bowl_tick = 0;
				win_str_data.lid_cmd = 0x00;
				win_change_page(POPUP_CASE);
				RTU_Cmd_Pending(RTU_CASE_ACTION,CLOSE);
				win_str_data.state = FSM_CASE_CLOSE;
			}
			else if(win_str_data.tank_lock_action == 1) //选择页开锁
			{
				win_str_data.tank_lock_action = 0;
				if(str_sys.location_test == 0x01 || str_sys.location_test == 0x02)
				{
					RTU_Cmd_Pending(RTU_SET_LED,RED);
					RTU_Cmd_Pending(RTU_LOCK_ACTION,UNLOCK);
					win_str_data.state = FSM_LOCK_1;
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

		case FSM_LOCK_1:
			if(str_sys.lock_state == 0 && str_sys.tank_lock == 1)
			{
				win_str_data.state = FSM_LOCK_2;         //解锁完成，等待把手下压
			}
			break;

		case FSM_LOCK_2:
			if(str_sys.tank_lock == 0)                  //把手下压到位
			{
				win_change_page(POPUP_LOCKING);        //正在锁紧
				RTU_Cmd_Pending(RTU_LOCK_ACTION,LOCK);
				win_str_data.state = FSM_LOCK_3;
			}
			break;

		case FSM_LOCK_3:
			if(str_sys.lock_state == 1)
			{
				if(str_sys.tank_lock == 1)
				{
					RTU_Cmd_Pending(RTU_SET_LED,RED);
					RTU_Cmd_Pending(RTU_LOCK_ACTION,UNLOCK);
					win_str_data.state = FSM_LOCK_1;
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
		default:
			break;
	}
}

void HFPeriod_1msTask(void)
{
	btn_click_handler();
	rtu_response_handler();
	win_fresh_window(&str_sys);

	if(win_str_data.state == FSM_INIT_WAIT_POS2 && str_sys.location_test == 0x02)
	{
		win_change_page(POPUP_CASE);       //等待盖子到位
		RTU_Cmd_Pending(RTU_STOP,MC_STOP);
		RTU_Cmd_Pending(RTU_CASE_ACTION,UNLOCK);
		win_str_data.state = FSM_POSITION_CHECK;
	}

	if(win_str_data.state == FSM_READY || win_str_data.state == FSM_PAUSE) 
	{
		if(str_sys.location_test == 0x02 && win_str_data.lid_cmd == 0x01)  // turn to No.2 seat  open the lid
		{
			win_str_data.lid_cmd = 0x00;
			win_change_page(POPUP_CASE);   //等待盖子到位界面
			RTU_Cmd_Pending(RTU_STOP,MC_STOP);
			RTU_Cmd_Pending(RTU_CASE_ACTION,UNLOCK); //send unlock lid
			win_str_data.state = FSM_POSITION_CHECK;
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
