#include <stdio.h>
#include <string.h>
#include "func_handler.h"
#include "nor_flash.h"
#include "start_win.h"
#include "motor_cmd.h"
#include "main.h"

u16 xdata tick_sec = 0;
code const char Software_version[11]={"V1.00"};

code const char Dir_on[3] = {"ON"};
code const char Dir_off[3] = {"OFF"};

Product_TypeDef product_data[PRODUCT_TOTAL]={ 
//id, product name,		 level		total_time	 work_time	   pause_time	  voltage		  current
{ 0,	"BM20Pro",		100,800,		1,99,     1,99,          1,99,        0,2542, 	    1000,15000,	},
};

STR_WIN_TypeDef xdata win_str_data;
static bit para_keypad_first_digit = 0;

//man page button handler API
static void win_main_btn_click_handler(u16 btn_val);
static void win_manual_btn_click_handler(u16 btn_val);
//work parameters settings page
static void win_test_para_btn_hold_handler(u16 btn_val);
static void win_prog_para_btn_hold_handler(u16 btn_val);
static void win_set_para_btn_hold_handler(u16 btn_val);
static void win_para_keypad_btn_click_handler(u16 btn_val);
static void win_factory_keypad_btn_click_handler(u16 btn_val);
static void win_pass_keypad_btn_click_handler(u16 btn_val);
static void win_prog_num_sel_btn_click_handler(u16 btn_val);

static void win_light_set_btn_hold_handler(u16 btn_val);
static void win_shake_set_btn_hold_handler(u16 btn_val);
static void win_language_set_btn_hold_handler(u16 btn_val);
static void win_buzz_set_btn_hold_handler(u16 btn_val);

//factory page button handler api
static void win_fac_set_btn_click_handler(u16 btn_val);
//function
static void win_calc_function(void);
static void win_load_setpara(void);

void win_save_prog_paras(u16 offaddr,u8 *var_addr,u16 len);
void win_load_prog_paras(u16 offaddr,u8 *var_addr,u16 len);
void win_fresh_model_icon(u16 mode);


extern u16 const page_reg_ch[22]={
	{ 1}, // MENU_PAGE = 0
	{ 2}, // MANUAL_PAGE
	{ 3}, // LOGO_PAGE
	{ 4}, // INFO_PAGE
	{ 5}, // INFO_NO_LOGO_PAGE
	{ 6}, // SYSTEM_PAGE
	{ 8}, // PASSWORD_PAGE
	{ 9}, // PASSWORD_CONFIRM_PAGE
	{15}, // WORK_PAGE
	{16}, // WORK_PAGE_NO_TOUCH
	{17}, // WORK_CONFIRM_PAGE
	{18}, // WORK_SHAKE_WARNING_PAGE
	{25}, // SET_PAGE
	{26}, // SET_CONFIRM_PAGE
	{28}, // PROGRAM_PAGE
	{29}, // PROGRAM_CONFIRM_PAGE
	{31}, // SELECT_TANK_PAGE
	{35}, // POPUP_LOCK_RESET       锁紧装置正在复位
	{36}, // POPUP_DETECT_TURN      正在检测转盘位置
	{37}, // POPUP_PRESS_HANDLE     请下压把手,锁紧研磨罐
	{38}, // POPUP_LOCKING          正在锁紧研磨罐
};

	
void btn_click_callback(u16 addr,u16 val)
{
	switch(addr)
	{
		case BTN_MAIN_PAGE_ADDR:				//Main page button   0x1100 
				win_main_btn_click_handler(val);
			break;
		case BTN_MANUAL_PAGE_ADDR:				//manual page button 0x1101
				win_manual_btn_click_handler(val);
			break;
		case BTN_SEL_BOWL_ADDR:
				win_str_data.sel_bowll = val;
			break;
		case BTN_PARA_KEYPAD_ADDR:
				win_para_keypad_btn_click_handler(val);
			break;
		case BTN_PROG_NUM_SEL_ADDR:
				win_prog_num_sel_btn_click_handler(val);
			break;
		case BTN_PASS_KEYPAD_ADDR:
				win_pass_keypad_btn_click_handler(val);
			break;
		case BTN_FAC_SET_ADDR:
				win_fac_set_btn_click_handler(val);
			break;
		default:
			break;
	}
}

void btn_hold_res_callback(u16 addr,u16 val)
{
	switch(addr)
	{
		case BTN_TEST_MODE_SET_ADDR:
			win_test_para_btn_hold_handler(val);
			break;
		case BTN_PROG_MODE_SET_ADDR:
			win_prog_para_btn_hold_handler(val);
			break;
		case BTN_SET_LIGHT_ADDR:
			win_light_set_btn_hold_handler(val);
			break;
		case BTN_SET_SHAKE_ADDR:
			win_shake_set_btn_hold_handler(val);
			break;
		case BTN_LANGUAGE_ADDR:
			win_language_set_btn_hold_handler(val);
			break;
		case BTN_SET_BUZZ_ADDR:
			win_buzz_set_btn_hold_handler(val);
			break;
		default:
			break;
	}
}

void btn_hold_click_callback(u16 addr,u16 val)
{
	val =0;
	if(win_str_data.start == 1) return;
	switch(addr)
	{		
		default:
			break;
	}
}

void btn_hold_callback(u16 addr,u16 val)
{
	val =0;
	if(win_str_data.start == 1) return;
	switch(addr)
	{
		default:
			break;
	}
}


static void win_calc_function(void)
{
	float xdata temp,temp1;
	
	temp = product_data[win_str_data.product_id].vol_max-product_data[win_str_data.product_id].vol_min;
	temp1 = product_data[win_str_data.product_id].para1.max - 1;
	win_str_data.k_factor = temp/temp1;
	
	temp = win_str_data.k_factor * 1;
	win_str_data.b_factor = product_data[win_str_data.product_id].vol_min-temp;
}

u16 win_level_trans_volt(u16 level)       //�����ѹֵ
{
	float temp;
	temp = level;
	
	temp = (float)(win_str_data.k_factor * temp)+win_str_data.b_factor;	

	return (u16)temp;
}
//load bar display

void win_change_page(u16 page)
{
	u16 temp;
	temp = page_reg_ch[page];  //Chinese
	//fresh page
	switch(page)
	{
		case WORK_PAGE:
			if((win_str_data.pause == 0)&&(win_str_data.start == 0))
			{
				win_disp_level(MAIN_TIMER,win_str_data.use_level);
				win_disp_cycle(MAIN_TIMER,win_str_data.use_cycle);
				win_disp_run_time(MAIN_TIMER,win_str_data.use_run_time);
				win_disp_pause_time(MAIN_TIMER,win_str_data.use_pause_time);
				win_disp_dir(MAIN_TIMER, win_str_data.use_dir);
			
				sys_write_vp(ARTNUM_MAIN_PROG_NUM_ADDR,(u8*)&win_str_data.prog_grp,1);
				win_str_data.disp_count_time = ((((u32)win_str_data.use_cycle * (win_str_data.use_run_time + win_str_data.use_pause_time)) - win_str_data.use_pause_time) * 60);
				win_disp_count_time(MAIN_TIMER,win_str_data.disp_count_time);
			}
			break;

		case SET_PAGE:
			win_disp_level(SET_TIMER,win_str_data.set_level);
			win_disp_cycle(SET_TIMER,win_str_data.set_cycle);
			win_disp_run_time(SET_TIMER,win_str_data.set_run_time);
			win_disp_pause_time(SET_TIMER,win_str_data.set_pause_time);
			win_disp_dir(SET_TIMER, win_str_data.set_dir);
			break;

		case PROGRAM_PAGE:
			win_disp_level(SET_TIMER,win_str_data.set_level);   //显示设置值
			win_disp_cycle(SET_TIMER,win_str_data.set_cycle);
			win_disp_run_time(SET_TIMER,win_str_data.set_run_time);
			win_disp_pause_time(SET_TIMER,win_str_data.set_pause_time);
			win_disp_dir(SET_TIMER, win_str_data.set_dir);
			//read norflash	
			win_disp_level(PROG_TIMER,win_str_data.prog_level); //显示程序值
			win_disp_cycle(PROG_TIMER,win_str_data.prog_cycle);
			win_disp_run_time(PROG_TIMER,win_str_data.prog_run_time);
			win_disp_pause_time(PROG_TIMER,win_str_data.prog_pause_time);
			win_disp_dir(PROG_TIMER, win_str_data.prog_dir);

			break;
		default:break;
	}
	//goto page
	sys_disp_page(temp);
}

//---------------------------------------------------------------//
//name: win_load_setpara
//---------------------------------------------------------------//
static void win_load_setpara(void)
{
	if(win_str_data.logo >0) win_str_data.logo = 4;
	sys_write_vp(ANIMOTION_LOGO_ADDR,(u8*)&win_str_data.logo,2);	//no logo for oem

	if(win_str_data.light > 100) win_str_data.light = 100;
	if(win_str_data.light < 10) win_str_data.light = 10;
	sys_setdis_light(win_str_data.light);
	sys_write_vp(BTN_SET_LIGHT_ADDR,(u8*)&win_str_data.light,1);
	if(win_str_data.shake > 150) win_str_data.shake = 150;
	if(win_str_data.shake < 10) win_str_data.shake = 10;
	sys_write_vp(BTN_SET_SHAKE_ADDR,(u8*)&win_str_data.shake,1);
	if(win_str_data.alarm > 1) win_str_data.alarm = 1;
	if(win_str_data.alarm < 0) win_str_data.alarm = 0;
	sys_enable_speaker(1-win_str_data.alarm);
	sys_write_vp(BTN_SET_BUZZ_ADDR,(u8*)&win_str_data.alarm,1);
	
	if(win_str_data.language >= 3) win_str_data.language = 0;
	if(win_str_data.language <= 0) win_str_data.language = 0;
	sys_write_vp(BTN_LANGUAGE_ADDR,(u8*)&win_str_data.language,1);
}

static void win_main_btn_click_handler(u16 btn_val)
{
	#define  BTN_VAL_START	     0X01    //start
	#define  BTN_VAL_PAUSE    	 0X02    //pause
	#define  BTN_VAL_STOP        0X03    //stop 

	#define  BTN_VAL_TEST        0X06 	 //test mode
	#define  BTN_VAL_MENU        0X07    //settings into
	#define  BTN_VAL_CONFOIRM    0X08    //Confirm startup
	#define  BTN_VAL_UNLOCK  	 0X09

	#define  BTN_VAL_BACK_TO_MAIN      	 0X0A
	#define  BTN_VAL_BACK_TO_MENU     	 0X0B
	#define  BTN_VAL_BACK_TO_SET     	 0X0C


	#define  BTN_VAL_SYS_SET             0X0D
	#define  BTN_VAL_COMPANY_INFO     	 0X0E
	#define  BTN_VAL_MANUAL              0X0F
	#define  BTN_VAL_RATIO_SET         	 0X10

	#define  BTN_VAL_CASE   		     0X11

	u16 xdata temp = 0;
	switch(btn_val)
	{
		case BTN_VAL_START:     //Page1:Push Start
			win_str_data.shak_warning = 0;
			if(win_str_data.pause == 1)		//pause   
			{
				win_str_data.start = 1;
				win_str_data.pause = 0;
			}
			else if((win_str_data.pause == 0)&&(win_str_data.start == 0))
			{
				win_change_page(WORK_CONFIRM_PAGE);
			}
			break;
		case BTN_VAL_CONFOIRM:
			win_str_data.start = 1;
			win_str_data.pause = 0;
			win_change_page(WORK_PAGE);
			break;
		case BTN_VAL_STOP:
				win_str_data.start = 0;
				win_str_data.pause = 0;
			break;
		case BTN_VAL_PAUSE:
				win_str_data.start = 0;
				win_str_data.pause = 1;
			break;
		case BTN_VAL_TEST:
			if((win_str_data.pause == 1)||(win_str_data.start != 0)) break;
		
			win_str_data.test_mode = 0;
			win_str_data.prog_grp = 0;
			win_fresh_model_icon(win_str_data.test_mode);

			norflash_write(SYS_LAST_PROG_OFFSET,(u8*)&win_str_data.prog_grp,2);
		
			win_load_prog_paras(PROG_LEVEL_OFFSET,(u8*)&win_str_data.use_level,6);
		
		
			win_disp_level(MAIN_TIMER,win_str_data.use_level);
			win_disp_cycle(MAIN_TIMER,win_str_data.use_cycle);
			win_disp_run_time(MAIN_TIMER,win_str_data.use_run_time);
			win_disp_pause_time(MAIN_TIMER,win_str_data.use_pause_time);
			win_disp_dir(MAIN_TIMER, win_str_data.use_dir);
			if(win_str_data.start == 0 && win_str_data.pause == 0) 
			{
				win_str_data.disp_count_time = ((((u32)win_str_data.use_cycle * (win_str_data.use_run_time + win_str_data.use_pause_time)) - win_str_data.use_pause_time) * 60);
				win_disp_count_time(MAIN_TIMER,win_str_data.disp_count_time);
			}

			break;
		case BTN_VAL_MENU:
			if((win_str_data.pause == 1)||(win_str_data.start != 0)) break;
			win_change_page(MENU_PAGE);
			break;
		
		case BTN_VAL_BACK_TO_MAIN:
			win_change_page(WORK_PAGE);
			break;
		
	    case BTN_VAL_BACK_TO_MENU:
			win_change_page(MENU_PAGE);
			break;

		case BTN_VAL_SYS_SET:
			win_change_page(SYSTEM_PAGE);
			break;

		case BTN_VAL_COMPANY_INFO:
			if(win_str_data.logo == 0)
				win_change_page(INFO_PAGE);
			else
				win_change_page(INFO_NO_LOGO_PAGE);
			break;

		case BTN_VAL_MANUAL:
			win_change_page(MANUAL_PAGE);
			break;

		case BTN_VAL_RATIO_SET:
			//
			break;

		case BTN_VAL_CASE:
			if(win_str_data.start != 1)
			{
				if(win_str_data.lid_cmd == 0x00)
				{
					win_str_data.lid_cmd = 0x11;  //请求开盖
				}
				else if(win_str_data.lid_cmd == 0x01)
				{
					if(win_str_data.state == FSM_SEL_BOWL)
					win_str_data.lid_cmd = 0x10;  //请求关盖
				}
			}
			break;
			
		default:
			break;
	}
}


static void win_manual_btn_click_handler(u16 btn_val)
{
	#define  BTN_VAL_M_OPEN_LID     1
	#define  BTN_VAL_M_CLOSE_LID    2
	#define  BTN_VAL_M_RUN_POS      3
	#define  BTN_VAL_M_RUN_TO_LOCK  4
	#define  BTN_VAL_M_UNLOCK       5
	#define  BTN_VAL_M_LOCK         6

	switch(btn_val)
	{
		case BTN_VAL_M_OPEN_LID:    //开盖
			RTU_Cmd_Pending(RTU_CASE_ACTION, UNLOCK);
			break;
		case BTN_VAL_M_CLOSE_LID:   //关盖
			RTU_Cmd_Pending(RTU_CASE_ACTION, LOCK);
			break;
		case BTN_VAL_M_RUN_POS:     //运行位置
			RTU_Cmd_Pending(RTU_LOCK_ACTION, LOCK_TO_RUN);
			break;
		case BTN_VAL_M_RUN_TO_LOCK: //运行位置到上锁
			RTU_Cmd_Pending(RTU_LOCK_ACTION, RUN_TO_LOCK);
			break;
		case BTN_VAL_M_UNLOCK:      //开锁
			RTU_Cmd_Pending(RTU_LOCK_ACTION, UNLOCK);
			break;
		case BTN_VAL_M_LOCK:        //上锁
			RTU_Cmd_Pending(RTU_LOCK_ACTION, LOCK);
			break;

		default:
			break;
	}
}


#define  BTN_VAL_NO	   		0X1005
#define  BTN_VAL_YES	   	0X1004
#define  BTN_VAL_RETURN		0X1003
#define  BTN_VAL_UPSET	 	0X1002
#define  BTN_VAL_OVERSET  	0X1001
#define  BTN_VAL_TOSET	    0X1000

#define  BTN_VAL_DIR	    		0X05
#define  BTN_VAL_CYCLE				0X04
#define  BTN_VAL_SEL_PAUSE_TIME	    0X03
#define  BTN_VAL_SEL_RUN_TIME		0X02
#define  BTN_VAL_SEL_LEVEL	        0X01


static void win_test_para_btn_hold_handler(u16 btn_val)
{
	u16 xdata temp;
	 
	if(win_str_data.paras_sel == BTN_VAL_SEL_LEVEL)
	{
		if(win_str_data.set_level >= product_data[win_str_data.product_id].para1.max) win_str_data.set_level = product_data[win_str_data.product_id].para1.max;
		else if(win_str_data.set_level < product_data[win_str_data.product_id].para1.min) win_str_data.set_level = product_data[win_str_data.product_id].para1.min;	
		win_disp_level(SET_TIMER,win_str_data.set_level);
	}
	else if(win_str_data.paras_sel == BTN_VAL_CYCLE)
	{
		if(win_str_data.set_cycle >product_data[win_str_data.product_id].para2.max) win_str_data.set_cycle =product_data[win_str_data.product_id].para2.max;
		else if(win_str_data.set_cycle < product_data[win_str_data.product_id].para2.min) win_str_data.set_cycle =product_data[win_str_data.product_id].para2.min;
		win_disp_cycle(SET_TIMER,win_str_data.set_cycle);
	}
	else if(win_str_data.paras_sel == BTN_VAL_SEL_RUN_TIME)
	{
		if(win_str_data.set_run_time >product_data[win_str_data.product_id].para3.max) win_str_data.set_run_time =product_data[win_str_data.product_id].para3.max;
		else if(win_str_data.set_run_time < product_data[win_str_data.product_id].para3.min) win_str_data.set_run_time =product_data[win_str_data.product_id].para3.min;
		win_disp_run_time(SET_TIMER,win_str_data.set_run_time);	
	}
	else if(win_str_data.paras_sel == BTN_VAL_SEL_PAUSE_TIME)
	{
		if(win_str_data.set_pause_time >product_data[win_str_data.product_id].para4.max) win_str_data.set_pause_time =product_data[win_str_data.product_id].para4.max;
		else if(win_str_data.set_pause_time < product_data[win_str_data.product_id].para4.min) win_str_data.set_pause_time =product_data[win_str_data.product_id].para4.min;
		win_disp_pause_time(SET_TIMER,win_str_data.set_pause_time);
	}
	switch(btn_val)
	{
		case BTN_VAL_SEL_LEVEL:
				win_str_data.input_disp1 = 0x01;
				win_str_data.input_disp2 = 0x00;
				win_str_data.input_disp3 = 0x00;
				win_str_data.input_disp4 = 0x00;
				disp_input_icon();
				para_keypad_first_digit = 1;
				win_str_data.paras_sel = btn_val;
			break;
		case BTN_VAL_SEL_RUN_TIME:
				win_str_data.input_disp1 = 0x00;
				win_str_data.input_disp2 = 0x01;
				win_str_data.input_disp3 = 0x00;
				win_str_data.input_disp4 = 0x00;
				disp_input_icon();

				win_str_data.set_run_time = 0;
				win_disp_run_time(SET_TIMER,win_str_data.set_run_time);

				para_keypad_first_digit = 1;
		  	    win_str_data.paras_sel = btn_val;
			break;
		case BTN_VAL_SEL_PAUSE_TIME:
				win_str_data.input_disp1 = 0x00;
				win_str_data.input_disp2 = 0x00;
				win_str_data.input_disp3 = 0x01;
				win_str_data.input_disp4 = 0x00;
				disp_input_icon();
		
				win_str_data.set_pause_time = 0;
				win_disp_pause_time(SET_TIMER,win_str_data.set_pause_time);	
		
				para_keypad_first_digit = 1;
				win_str_data.paras_sel = btn_val;
			break;		
		case BTN_VAL_CYCLE:
				win_str_data.input_disp1 = 0x00;
				win_str_data.input_disp2 = 0x00;
				win_str_data.input_disp3 = 0x00;
				win_str_data.input_disp4 = 0x01;
				disp_input_icon();
				para_keypad_first_digit = 1;
				win_str_data.paras_sel = btn_val;
			break;

		case BTN_VAL_TOSET:    //Into parameter 
			   memcpy((u8 *)&win_str_data.set_level,(u8 *)&win_str_data.use_level,10);
			   win_change_page(SET_PAGE);	 //change page
			   break;
		case BTN_VAL_RETURN:   //return 
			win_str_data.paras_sel = 0;

			win_str_data.input_disp1 = 0x00;
			win_str_data.input_disp2 = 0x00;
			win_str_data.input_disp3 = 0x00;
			win_str_data.input_disp4 = 0x00;
			disp_input_icon();

			temp = 0;
			sys_write_vp(BTN_TEST_MODE_SET_ADDR,(u8 *)&temp,1);
			win_change_page(WORK_PAGE);
			break;

		case BTN_VAL_OVERSET:  //jump to pop up
			win_str_data.paras_sel = 0;

			win_str_data.input_disp1 = 0x00;
			win_str_data.input_disp2 = 0x00;
			win_str_data.input_disp3 = 0x00;
			win_str_data.input_disp4 = 0x00;
			disp_input_icon();		
			win_change_page(SET_CONFIRM_PAGE);
			break;
		case BTN_VAL_YES:
			memcpy((u8 *)&win_str_data.use_level,(u8 *)&win_str_data.set_level,10);
			win_save_prog_paras(PROG_LEVEL_OFFSET,(u8*)&win_str_data.use_level,6);
			win_change_page(SET_PAGE);
			break;
		case BTN_VAL_NO:
				win_change_page(SET_PAGE);
			break;
		case BTN_VAL_DIR:
				win_str_data.input_disp1 = 0x00;
				win_str_data.input_disp2 = 0x00;
				win_str_data.input_disp3 = 0x00;
				win_str_data.input_disp4 = 0x00;
				disp_input_icon();

				win_str_data.set_dir = 1 - win_str_data.set_dir;
				if(win_str_data.set_dir > 0) win_str_data.set_dir = 1;
				win_disp_dir(SET_TIMER, win_str_data.set_dir);

				temp = 0;
				sys_write_vp(BTN_TEST_MODE_SET_ADDR,(u8 *)&temp,1);
			break;			
		default:
			break;
	}
}

static void win_prog_para_btn_hold_handler(u16 btn_val)
{
	u16 xdata temp;
	if((win_str_data.pause == 1)||(win_str_data.start != 0))
	{
		temp = 0;
		sys_write_vp(BTN_PROG_MODE_SET_ADDR,(u8*)&temp,1);
		return;
	}
	//jugde other dat correct
	switch(btn_val)	
	{
		case BTN_VAL_TOSET:
				if(win_str_data.prog_grp == 0) win_str_data.prog_grp = 1;
				sys_write_vp(ARTNUM_PROG_NUM_ADDR,(u8*)&win_str_data.prog_grp,1);
				memcpy((u8 *)&win_str_data.set_level,(u8 *)&win_str_data.use_level,10);	         //display set
				win_load_prog_paras(PROG_LEVEL_OFFSET,(u8*)&win_str_data.prog_level,6);  //read prog para from flash
				win_change_page(PROGRAM_PAGE);	 //change page
			break;
		case BTN_VAL_RETURN: //Reset the key value when returning and jump page and clean paras_sel
				win_str_data.input_disp1 = 0x00;
				win_str_data.input_disp2 = 0x00;
				win_str_data.input_disp3 = 0x00;
				win_str_data.input_disp4 = 0x00;
				disp_input_icon();

				win_str_data.paras_sel = 0;
				if(win_str_data.test_mode == 0) win_str_data.prog_grp = 0;
				win_change_page(WORK_PAGE);
			break;
		case BTN_VAL_OVERSET:  //Jump to pop-up page
				win_str_data.paras_sel = 0;
				win_change_page(PROGRAM_CONFIRM_PAGE);	
			break;
		case BTN_VAL_YES:  	 //pop-up page YES   Put Set -> Prog and save flash
				memcpy((u8 *)&win_str_data.prog_level,(u8 *)&win_str_data.set_level,10);  					//Set value to prog value
				win_save_prog_paras(PROG_LEVEL_OFFSET,(u8*)&win_str_data.prog_level,6);	//write norflash
				win_change_page(PROGRAM_PAGE);		//change page
			break;
		case BTN_VAL_NO:
				win_change_page(PROGRAM_PAGE);	
			break;
		case BTN_VAL_UPSET:
				win_str_data.test_mode = 1;
				win_fresh_model_icon(win_str_data.test_mode);
		
				norflash_write(SYS_LAST_PROG_OFFSET,(u8*)&win_str_data.prog_grp,2);
				sys_write_vp(ARTNUM_MAIN_PROG_NUM_ADDR,(u8*)&win_str_data.prog_grp,1);
				memcpy((u8 *)&win_str_data.set_level,(u8 *)&win_str_data.prog_level,10);
				memcpy((u8 *)&win_str_data.use_level,(u8 *)&win_str_data.prog_level,10);
		
				win_disp_level(SET_TIMER,win_str_data.set_level);
				win_disp_cycle(SET_TIMER,win_str_data.set_cycle);
				win_disp_run_time(SET_TIMER,win_str_data.set_run_time);
				win_disp_pause_time(SET_TIMER,win_str_data.set_pause_time);
				win_disp_dir(SET_TIMER, win_str_data.set_dir);		
	
				temp = 0;
				sys_write_vp(BTN_PROG_MODE_SET_ADDR,(u8*)&temp,1);
			break;
				
		default:
			break;
		}
}


#define  BTN_VAL_ACC	    0X02
#define  BTN_VAL_DEC	    0X01

static void win_prog_num_sel_btn_click_handler(u16 btn_val)
{
	u16 xdata temp;
	 
	switch(btn_val)
	{
		case BTN_VAL_ACC:
				if(win_str_data.prog_grp < 20) win_str_data.prog_grp++;
			break;
		case BTN_VAL_DEC:
				if(win_str_data.prog_grp > 1) win_str_data.prog_grp--;
			break;
		default:break;
	}
	sys_write_vp(ARTNUM_PROG_NUM_ADDR,(u8*)&win_str_data.prog_grp,1);
	//read nor flash  fresh display
	win_load_prog_paras(PROG_LEVEL_OFFSET,(u8*)&win_str_data.prog_level,6);

	win_disp_level(PROG_TIMER,win_str_data.prog_level);
	win_disp_run_time(PROG_TIMER,win_str_data.prog_run_time);
	win_disp_pause_time(PROG_TIMER,win_str_data.prog_pause_time);
	win_disp_cycle(PROG_TIMER,win_str_data.prog_cycle);

	temp = 0;
	sys_write_vp(BTN_PROG_MODE_SET_ADDR,(u8*)&temp,1);

}

/********************************************************
?????????win_para_keypad_btn_click_handler
???????????????????????
*********************************************************/
#define  BTN_VAL_1		1
#define  BTN_VAL_2		2
#define  BTN_VAL_3	    3
#define  BTN_VAL_4      4
#define  BTN_VAL_5      5
#define  BTN_VAL_6		6
#define  BTN_VAL_7		7
#define  BTN_VAL_8      8
#define  BTN_VAL_9		9
#define  BTN_VAL_0		10
#define  BTN_VAL_cancel	11

static u16 win_para_keypad_input(u16 old_val, u16 btn_val)
{
	u16 xdata val;

	if(btn_val == BTN_VAL_cancel)
	{
		if(para_keypad_first_digit)
		{
			val = 0;
		}
		else
		{
			val = old_val / 10;
		}
	}
	else if((btn_val >= BTN_VAL_1) && (btn_val <= BTN_VAL_0))
	{
		if(para_keypad_first_digit)
		{
			val = (btn_val == BTN_VAL_0) ? 0 : btn_val;
		}
		else
		{
			val = (old_val % 1000) * 10;
			val += (btn_val == BTN_VAL_0) ? 0 : btn_val;
		}
	}
	else
	{
		return old_val;
	}

	para_keypad_first_digit = 0;
	return val;
}

static void win_para_keypad_btn_click_handler(u16 btn_val)
{
	switch(win_str_data.paras_sel)
	{
		case BTN_VAL_SEL_LEVEL:
			win_str_data.set_level = win_para_keypad_input(win_str_data.set_level, btn_val);
			win_disp_level(SET_TIMER, win_str_data.set_level);
			break;
		case BTN_VAL_CYCLE:
			win_str_data.set_cycle = win_para_keypad_input(win_str_data.set_cycle, btn_val);
			win_disp_cycle(SET_TIMER, win_str_data.set_cycle);
			break;
		case BTN_VAL_SEL_RUN_TIME:
			win_str_data.set_run_time = win_para_keypad_input(win_str_data.set_run_time, btn_val);
			win_disp_run_time(SET_TIMER, win_str_data.set_run_time);
			break;
		case BTN_VAL_SEL_PAUSE_TIME:
			win_str_data.set_pause_time = win_para_keypad_input(win_str_data.set_pause_time, btn_val);
			win_disp_pause_time(SET_TIMER, win_str_data.set_pause_time);
			break;
		default:
			break;
	}
}

static void win_pass_keypad_btn_click_handler(u16 btn_val)
{
	u16 xdata temp;
	temp = win_str_data.passcode;
	if(btn_val == 12)
	{
		win_str_data.used_time = 0;
		sys_write_vp(ARTNUM_UT_ADDR,(u8*)&win_str_data.used_time,2);
  	    norflash_write(RUNTIME_MIN_HADDR,(u8*)&win_str_data.used_time,2);
		win_str_data.passcode = 0;
		sys_write_vp(ARTNUM_PASSCODE_ADDR,(u8*)&win_str_data.passcode,2);
	}
	else if(btn_val < BTN_VAL_cancel)
	{
		if(btn_val == 10) btn_val =0;
		temp = temp%1000;
		temp = temp*10;
		temp +=btn_val;
		if(temp>9999) temp =9999;
		else if(temp <0) temp =0;
		win_str_data.passcode = temp;
		sys_write_vp(ARTNUM_PASSCODE_ADDR,(u8*)&win_str_data.passcode,2);
	}
	else
	{
		temp= temp/10;
		if(temp <0) temp=0;
		win_str_data.passcode = temp;
		sys_write_vp(ARTNUM_PASSCODE_ADDR,(u8*)&win_str_data.passcode,2);
	}
	if(temp == 1010)
	{
		win_str_data.passcode = 0;
		sys_write_vp(ARTNUM_PASSCODE_ADDR,(u8*)&win_str_data.passcode,2);
	}
	else if(temp == 5973)
	{
		win_str_data.passcode = 0;
		sys_write_vp(ARTNUM_PASSCODE_ADDR,(u8*)&win_str_data.passcode,2);
	}
}


static void win_set_para_btn_hold_handler(u16 btn_val)
{
	win_str_data.language = btn_val&0x0001;

	win_str_data.alarm = btn_val&0x0002;
	sys_enable_speaker(1 - win_str_data.alarm);

	win_str_data.light = btn_val>>8&0xff;
	sys_setdis_light(win_str_data.light);

}


static void win_fac_set_btn_click_handler(u16 btn_val)
{
	#define  BTN_VAL_PASSCODE	    0X01
	#define  BTN_VAL_LOGO	    	0X08
	win_str_data.factory_sel = 0;
	switch(btn_val)
	{
		case BTN_VAL_PASSCODE://logo
			win_change_page(PASSWORD_PAGE);
			break;
		case BTN_VAL_LOGO://logo
			win_str_data.logo = 4 - win_str_data.logo;
			if(win_str_data.logo > 0) win_str_data.logo = 4;
			sys_write_vp(ANIMOTION_LOGO_ADDR,(u8 *)&win_str_data.logo,2);	//no logo for oem	
			norflash_write(SYS_LOGO_OFFSET,(u8 *)&win_str_data.logo,2);
			break;
		default:
			break;
	}
	win_str_data.factory_sel = btn_val;
}

void win_save_prog_paras(u16 offaddr,u8 *var_addr,u16 len)
{
	u16 addr;
	if(win_str_data.prog_grp > 0)
	{
		addr = PROG1_BASE_NORADDR + 0x10*(win_str_data.prog_grp-1)+offaddr;	
	}
	else
	{
		addr = CUSTOM_BASE_NORADDR + offaddr;
	}
	norflash_write(addr,var_addr,len);
}



void win_load_prog_paras(u16 offaddr,u8 *var_addr,u16 len)
{
	u16 xdata addr;
	u16 xdata i;
	u16 xdata *p_val;

	if(win_str_data.prog_grp > 0)
	{
		addr = PROG1_BASE_NORADDR + 0x10 * (win_str_data.prog_grp - 1) + offaddr;
	}
	else
	{
		addr = CUSTOM_BASE_NORADDR + offaddr;
	}
	norflash_read(addr,var_addr,len);

	p_val = (u16 xdata *)var_addr;

	for(i = 0;i<len;i++)
	{
		addr = offaddr + i;
		switch(addr)
		{
			case PROG_LEVEL_OFFSET:
				if(p_val[i] > product_data[win_str_data.product_id].para1.max)
					p_val[i] = product_data[win_str_data.product_id].para1.max;
				else if(p_val[i] < product_data[win_str_data.product_id].para1.min)
					p_val[i] = product_data[win_str_data.product_id].para1.min;
				break;
			case PROG_RUN_TIME_OFFSET:
				if(p_val[i] > product_data[win_str_data.product_id].para3.max)
					p_val[i] = product_data[win_str_data.product_id].para3.max;
				else if(p_val[i] < product_data[win_str_data.product_id].para3.min)
					p_val[i] = product_data[win_str_data.product_id].para3.min;
				break;
			case PROG_PAUSE_TIME_OFFSET:
				if(p_val[i] > product_data[win_str_data.product_id].para4.max)
					p_val[i] = product_data[win_str_data.product_id].para4.max;
				else if(p_val[i] < product_data[win_str_data.product_id].para4.min)
					p_val[i] = product_data[win_str_data.product_id].para4.min;
				break;
			case PROG_CYCLE_OFFSET:
				if(p_val[i] > product_data[win_str_data.product_id].para2.max)
					p_val[i] = product_data[win_str_data.product_id].para2.max;
				else if(p_val[i] < product_data[win_str_data.product_id].para2.min)
					p_val[i] = product_data[win_str_data.product_id].para2.min;
				break;
			case PROG_DIR_OFFSET:
				if(p_val[i] > 1)
					p_val[i] = 1;
				break;
			default:
				break;
		}
	}
}



void win_disp_level(u16 ch,u16 val)
{
	if(ch & MAIN_TIMER)
	{
		sys_write_vp(ARTNUM_MAIN_LEVEL_ADDR,(u8 *)&val,1);
	}
	if(ch & SET_TIMER)
	{
		sys_write_vp(ARTNUM_SET_LEVEL_ADDR,(u8 *)&val,1);
	}
	if(ch & PROG_TIMER)
	{
		sys_write_vp(ARTMUM_PROG_LEVEL_ADDR,(u8 *)&val,1);
	}
}

void win_disp_cycle(u16 ch,u16 val) 
{
	u16 xdata h_dat;
	u16 xdata l_dat;

	if(ch & MAIN_TIMER)  //主页面
	{
		h_dat = win_str_data.use_cycle - val+1;	//运行次数从1开始
		l_dat = win_str_data.use_cycle;
	}
	else  //其他页面
	{
		l_dat = val;
	}
	
	if(ch & MAIN_TIMER)
	{
		sys_write_vp(ARTNUM_MAIN_CYCLE_ADDR,(u8 *)&l_dat,1);
		sys_write_vp(ARTNUM_MAIN_CYCLE_HADDR,(u8 *)&h_dat,1);
	}
	if(ch & SET_TIMER)
	{
		sys_write_vp(ARTNUM_SET_CYCLE_ADDR,(u8 *)&l_dat,1);
	}
	if(ch & PROG_TIMER)
	{
		sys_write_vp(ARTNUM_PROG_CYCLE_ADDR,(u8 *)&l_dat,1);
	}
}

void win_disp_count_time(u16 ch,u32 val)
{
	u16 xdata h_dat;
	u16 xdata m_dat;
	u16 xdata s_dat;

	h_dat = val/3600;
	m_dat = (val%3600)/60;
	s_dat = (val%3600)%60;
	
	if(h_dat >= 999)
	{
		h_dat = 999;
	}
	if(ch & MAIN_TIMER)
	{
		sys_write_vp(ARTNUM_MAIN_COUNT_TIMEH_ADDR,(u8 *)&h_dat,1);
		sys_write_vp(ARTNUM_MAIN_COUNT_TIMEM_ADDR,(u8 *)&m_dat,1);
		sys_write_vp(ARTNUM_MAIN_COUNT_TIMES_ADDR,(u8 *)&s_dat,1);
	}
}


void win_disp_run_time(u16 ch,u16 val)
{
	if(ch & MAIN_TIMER)
	{
		sys_write_vp(ARTNUM_MAIN_RUN_TIME_ADDR,(u8 *)&val,1);
	}	
	if(ch & SET_TIMER)
	{
		sys_write_vp(ARTNUM_SET_RUN_TIME_ADDR,(u8 *)&val,1);
	}
	if(ch & PROG_TIMER)
	{
		sys_write_vp(ARTNUM_PROG_RUN_TIME_ADDR,(u8 *)&val,1);
	}
}

void win_disp_pause_time(u16 ch,u16 val)
{
	if(ch & MAIN_TIMER)
	{
		sys_write_vp(ARTNUM_MAIN_PAUSE_TIME_ADDR,(u8 *)&val,1);
	}
	if(ch & SET_TIMER)
	{
		sys_write_vp(ARTNUM_SET_PAUSE_TIME_ADDR,(u8 *)&val,1);
	}
	if(ch & PROG_TIMER)
	{
		sys_write_vp(ARTNUM_PROG_PAUSE_TIME_ADDR,(u8 *)&val,1);
	}
}

void win_disp_dir(u16 ch, u16 val)
{
	if(ch & MAIN_TIMER)
	{
		if(val == 1)  //反转开
			sys_write_vp(STRING_MAIN_DIR_ADDR,(u8*)Dir_on,sizeof(Dir_on));
		if(val == 0)  //反转关
			sys_write_vp(STRING_MAIN_DIR_ADDR,(u8*)Dir_off,sizeof(Dir_off));
	}
	if(ch & SET_TIMER)
	{
		if(val == 1)
			sys_write_vp(STRING_SET_DIR_ADDR,(u8*)Dir_on,sizeof(Dir_on));
		if(val == 0)
			sys_write_vp(STRING_SET_DIR_ADDR,(u8*)Dir_off,sizeof(Dir_off));
	}
	if(ch & PROG_TIMER)
	{
		if(val == 1)
			sys_write_vp(STRING_PROG_DIR_ADDR,(u8*)Dir_on,sizeof(Dir_on));
		if(val == 0)
			sys_write_vp(STRING_PROG_DIR_ADDR,(u8*)Dir_off,sizeof(Dir_off));
	}
				
}
u16 win_sec_timer_handle(u16* stage)
{
	u16 xdata ret = 0;
	u16 xdata cycle_flag =0;
	if(*stage == 0)//run stage
	{
		win_str_data.start = 1;
		if(win_str_data.disp_run_time > 0)win_str_data.disp_run_time--;
		if(win_str_data.disp_run_time == 0)
		{
			win_str_data.disp_run_time = win_str_data.use_run_time;
			*stage = 1;	
			ret= 1;// return1  pause
		}
	}
	else if(*stage == 1)//pause stage
	{
		win_str_data.start = 2;
		if(win_str_data.disp_pause_time > 0)win_str_data.disp_pause_time--;
		if(win_str_data.disp_pause_time == 0)
		{
			win_str_data.disp_pause_time = win_str_data.use_pause_time;
			if(win_str_data.disp_cycle > 0)
				win_str_data.disp_cycle--;
			*stage = 0;
			ret= 2;  //return2 start
		}
	}

	if(win_str_data.disp_count_time > 0)			//process total time
	{
		win_str_data.disp_count_time--;
	}
	
	if(win_str_data.disp_count_time == 0)
	{
		ret= 3;
	}

	
	return ret;
}

void win_fresh_window(SysDataDef *sys)
{
	sys_write_vp(ICON_SHAK_WARRING_ADDR,(u8*)&win_str_data.shak_warning,2);
	sys_write_vp(ICON_START_ADDR,(u8*)&win_str_data.start,1);
	sys_write_vp(ICON_PAUSE_ADDR,(u8*)&win_str_data.pause,1);

	//fresh main page timer block
	win_fresh_timer_block();
	//fresh load bar

	if(win_str_data.start == 0 && win_str_data.pause == 0)
	{
		tick_sec = 0;
	}
}

void win_fresh_model_icon(u16 mode)
{
	u16 xdata temp;
	if(mode == 0)
	{
		temp = 1;
		sys_write_vp(ICON_TEST_ADDR,(u8*)&temp,1);
		temp = 0;
		sys_write_vp(ICON_PROG_ADDR,(u8*)&temp,1);
	}
	else
	{
		temp = 0;
		sys_write_vp(ICON_TEST_ADDR,(u8*)&temp,1);
		temp = 1;
		sys_write_vp(ICON_PROG_ADDR,(u8*)&temp,1);
	}
}		

void win_fresh_timer_block(void)
{
	static u16 timer_fresh_ratio;
	if(timer_fresh_ratio >0) timer_fresh_ratio--;
	{
		timer_fresh_ratio = 50;//20hz
		
		if((win_str_data.start != 0)&&(win_str_data.pause == 0))  
		{
			win_disp_run_time(MAIN_TIMER,win_str_data.disp_run_time);
			win_disp_pause_time(MAIN_TIMER,win_str_data.disp_pause_time);
			win_disp_cycle(MAIN_TIMER,win_str_data.disp_cycle);
			win_disp_count_time(MAIN_TIMER,win_str_data.disp_count_time);			
		}
	}
}

void Fresh_Service_Time(void)
{

}


//-------------------------------------------------------------------
//函数名: win_light_set_btn_hold_handler
//功能:   背光亮度设置(长按增减调节)
//参数:   btn_val - DGUS屏按钮返回值(亮度等级)
//说明:   1. 将按钮值写入全局结构体light变量
//       2. 调用sys_setdis_light()即时设置DGUS屏背光亮度
//       3. 将新亮度值保存到NorFlash,上电时读取恢复
//-------------------------------------------------------------------
static void win_light_set_btn_hold_handler(u16 btn_val)
{
	win_str_data.light = btn_val;            // 1. 保存到全局运行变量
	sys_setdis_light(win_str_data.light);    // 2. 实时设置DGUS屏背光(0~5级)
	norflash_write(SYS_LIGHT_OFFSET,(u8 *)&win_str_data.light,2);  // 3. 写入NorFlash持久化存储
}

//-------------------------------------------------------------------
//函数名: win_shake_set_btn_hold_handler
//功能:   震动报警开关设置
//参数:   btn_val - DGUS屏按钮返回值(0=关闭, 1=开启)
//说明:   将震动报警开关状态保存到全局变量并写入NorFlash
//-------------------------------------------------------------------
static void win_shake_set_btn_hold_handler(u16 btn_val)
{
	win_str_data.shake = btn_val;            // 1. 保存震动开关状态(0关/1开)
	norflash_write(SYS_SHAKE_OFFSET,(u8 *)&win_str_data.shake,2);  // 2. 写入NorFlash持久化存储
}

//-------------------------------------------------------------------
//函数名: win_language_set_btn_hold_handler
//功能:   系统语言设置
//参数:   btn_val - DGUS屏按钮返回值(语言编号,如0=中文,1=英文)
//说明:   将语言设置保存到全局变量并写入NorFlash,上电读取恢复
//-------------------------------------------------------------------
static void win_language_set_btn_hold_handler(u16 btn_val)
{
	win_str_data.language = btn_val;         // 1. 保存语言编号到全局变量
	norflash_write(SYS_LANGUAGE_OFFSET,(u8 *)&win_str_data.language,2);  // 2. 写入NorFlash持久化存储
}

//-------------------------------------------------------------------
//函数名: win_buzz_set_btn_hold_handler
//功能:   蜂鸣器开关设置
//参数:   btn_val - DGUS屏按钮返回值(0=关闭, 1=开启)
//说明:   1. 保存开关状态到全局变量
//       2. 调用sys_enable_speaker()即时控制DGUS蜂鸣器
//       3. 写入NorFlash持久化存储
//-------------------------------------------------------------------
static void win_buzz_set_btn_hold_handler(u16 btn_val)
{
	win_str_data.alarm = btn_val;            // 1. 保存蜂鸣器开关状态(0关/1开)
	sys_enable_speaker(1-win_str_data.alarm);// 2. 反相设置DGUS蜂鸣器(1=en, alarm=0时使能)
	norflash_write(SYS_BUZZ_OFFSET,(u8 *)&win_str_data.alarm,2);  // 3. 写入NorFlash持久化存储
}



void disp_input_icon(void)
{
	sys_write_vp(ICON_INPUT1_ADDR,(u8 *)&win_str_data.input_disp1,4);
}



//****************************????????????****************************
void win_init(void)
{
//1.init workbence para
	memset(&win_str_data,0,sizeof(win_str_data));
//2.read no flash	
	norflash_read(SYS_LOGO_OFFSET,(u8 *)&win_str_data.logo,8);
	win_str_data.product_id = 0;
//3.read & load system parametes settings
	win_load_setpara();
//3	service time
	win_str_data.used_time = win_str_data.used_time_h;
	win_str_data.used_time <<= 16;
	win_str_data.used_time |= win_str_data.used_time_l;
	win_str_data.disp_time = win_str_data.used_time/3600;
//4 program group
	if(win_str_data.prog_grp > 20 || win_str_data.prog_grp == 0)
	{
		win_str_data.prog_grp = 0;
		win_str_data.test_mode = 0;
	}
	if(win_str_data.prog_grp>0) win_str_data.test_mode = 1;
		win_fresh_model_icon(win_str_data.test_mode);
	
//5.update product id &software version
	sys_write_vp(STRING_PRODUCT_ADDR,(u8*)product_data[win_str_data.product_id].name,sizeof(product_data[win_str_data.product_id].name));
	sys_write_vp(STRING_VERSION_ADDR,(u8*)Software_version,sizeof(Software_version));

//6. load the last time backgroud setting parameters 
	win_load_prog_paras(PROG_LEVEL_OFFSET,(u8*)&win_str_data.prog_level,6);
	memcpy((u8*)&win_str_data.use_level,(u8*)&win_str_data.prog_level,10);

//10. calc level vs voltage function
	win_calc_function();
	sys_delay_about_ms(6000);
//11. jump work page	

	win_change_page(WORK_PAGE);
}