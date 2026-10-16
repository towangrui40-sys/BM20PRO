#ifndef __START_WIN_H__
#define __START_WIN_H__
#include "sys.h"

#define PRODUCT_TOTAL 1
/*---------------------define arae-------------------------*/
#define ANIMOTION_LOGO_ADDR		    0x2100  
//WORK PAGE VAR

//0x2000~0x201F
#define ARTNUM_MAIN_LEVEL_ADDR		    	0x2000

#define ARTNUM_MAIN_COUNT_TIMEH_ADDR 	 	0x2001
#define ARTNUM_MAIN_COUNT_TIMEM_ADDR 	 	0x2002
#define ARTNUM_MAIN_COUNT_TIMES_ADDR 	 	0x2003

#define ARTNUM_MAIN_CENTRIFUGAL_ACC_ADDR 	0x2004

#define ARTNUM_MAIN_RUN_TIME_ADDR 	   		0x2005
#define ARTNUM_MAIN_PAUSE_TIME_ADDR 	 	0x2006

#define ARTNUM_MAIN_CYCLE_ADDR   	      	0x2007
#define ARTNUM_MAIN_CYCLE_HADDR		      	0X2008

#define ARTNUM_MAIN_PROG_NUM_ADDR   	    0x200A
/********************************************************/  
//0x2020~0x203F
#define ARTNUM_SET_LEVEL_ADDR		      0x2020
#define ARTNUM_SET_RUN_TIME_ADDR 	   	  0x2021
#define ARTNUM_SET_PAUSE_TIME_ADDR 	      0x2022
#define ARTNUM_SET_CYCLE_ADDR   		  0x2023
/********************************************************/
//0x2040~0x205F
#define ARTMUM_PROG_LEVEL_ADDR			  0x2040
#define ARTNUM_PROG_RUN_TIME_ADDR		  0x2041
#define ARTNUM_PROG_PAUSE_TIME_ADDR		  0x2042
#define ARTNUM_PROG_CYCLE_ADDR   		  0x2043

#define ARTNUM_PROG_NUM_ADDR			  0x2046
/*******************************************************/

#define ARTNUM_PASSCODE_ADDR			  0x2060
#define ARTNUM_INPUTNUM_ADDR		      0x2061
#define ARTNUM_UT_ADDR					  0x2062 //service used time


#define ICON_START_ADDR   0x2500
#define ICON_PAUSE_ADDR   0x2501

#define ICON_PROG_ADDR    0x2503
#define ICON_TEST_ADDR    0x2504

#define ICON_INPUT1_ADDR 	0x2505
#define ICON_INPUT2_ADDR 	0x2506
#define ICON_INPUT3_ADDR 	0x2507
#define ICON_INPUT4_ADDR 	0x2508
#define ICON_INPUT5_ADDR 	0x2509

#define ICON_DOOR_WARRING_ADDR  0x2509
#define ICON_SHAK_WARRING_ADDR  0x250A

#define ICON_SHAK_GEAR_ADDR 	0X2518

#define ICON_BOWL_SEL1_ADDR 	0x251A
#define ICON_BOWL_SEL2_ADDR 	0x251B

#define ICON_BOWL1_TEST_CH_ADDR 0x251C
#define ICON_BOWL2_TEST_CH_ADDR 0x251D 

#define ICON_BOWL1_TEST_ADDR    0x251E
#define ICON_BOWL2_TEST_ADDR    0x251F 




//string
#define STRING_PRODUCT_ADDR 	0x3000
#define STRING_VERSION_ADDR 	0x3100
#define STRING_MAIN_DIR_ADDR	0X3200
#define STRING_SET_DIR_ADDR	    0X3300
#define STRING_PROG_DIR_ADDR	0X3400





//nor flash address list,the address lentgh is 2 bytes 


// each addr include 2 byte
#define SYS_LOGO_OFFSET			0X00
#define SYS_LANGUAGE_OFFSET		0X01
#define SYS_LIGHT_OFFSET		0X02
#define SYS_BUZZ_OFFSET		    0X03
#define SYS_SHAKE_OFFSET		0X04
#define SYS_LAST_PROG_OFFSET	0X05
#define RUNTIME_MIN_HADDR 	    0X06
#define RUNTIME_MIN_LADDR 		0X07

#define CUSTOM_BASE_NORADDR		0x10
#define PROG1_BASE_NORADDR		0x20
#define PROG2_BASE_NORADDR		0x30
#define PROG3_BASE_NORADDR		0x40
#define PROG4_BASE_NORADDR		0x50
#define PROG5_BASE_NORADDR		0x60
#define PROG6_BASE_NORADDR		0x70
#define PROG7_BASE_NORADDR		0x80
#define PROG8_BASE_NORADDR		0x90
#define PROG9_BASE_NORADDR		0xA0
#define PROG10_BASE_NORADDR		0xB0
#define PROG11_BASE_NORADDR		0xC0
#define PROG12_BASE_NORADDR		0xD0
#define PROG13_BASE_NORADDR		0xE0
#define PROG14_BASE_NORADDR		0xF0
#define PROG15_BASE_NORADDR		0x100
#define PROG16_BASE_NORADDR		0x110
#define PROG17_BASE_NORADDR		0x120
#define PROG18_BASE_NORADDR		0x130
#define PROG19_BASE_NORADDR		0x140
#define PROG20_BASE_NORADDR		0x150
// each addr include 2 byte
#define PROG_LEVEL_OFFSET		0  //level
#define PROG_RUN_TIME_OFFSET	1  //run_time
#define PROG_PAUSE_TIME_OFFSET	2  //pause_time
#define PROG_CYCLE_OFFSET		3  //cycle
#define PROG_DIR_OFFSET			4  //dir





/*-------------------------------------------------------*/


typedef enum{
	MENU_PAGE = 0,
	MANUAL_PAGE,
	LOGO_PAGE,
	INFO_PAGE,
	INFO_NO_LOGO_PAGE,
	SYSTEM_PAGE,
	PASSWORD_PAGE,
	PASSWORD_CONFIRM_PAGE,
	WORK_PAGE,
	WORK_PAGE_NO_TOUCH,
	WORK_CONFIRM_PAGE,
	WORK_SHAKE_WARNING_PAGE,
	SET_PAGE,
	SET_CONFIRM_PAGE,
	PROGRAM_PAGE,
	PROGRAM_CONFIRM_PAGE,
	SELECT_TANK_PAGE,
	PLEASE_WAIT_PAGE,
}Change_Page_Def;



	
typedef struct{
	u16 min;	//para min value
	u16 max;	//para max value
}PARA_RangeDef;

typedef struct{
	u16 model_id;
	char name[20];//product name

	PARA_RangeDef para1;//level
	PARA_RangeDef para2;//total time
	PARA_RangeDef para3;//work time
	PARA_RangeDef para4;//pause time
	
	u16 vol_min;		//the minimum level vs the minimum output voltage
	u16 vol_max;		//the maximum level vs the maximum output voltage
	u16 cur_min;		//the minimum AC current vs load bar 0%
	u16 cur_max;		//the maximum AC current vs load bar 100%
	
	u16 timer_mode;		//function:3:total time+cycle mode2--cycle mode.1--work time+pause time combo enable,0--disable
	u16 ccw_en;			//function:1--motor cc/ccw enable,0--disable
	u16 load_bar_en;	//function:1--display load bar,0--disable
}Product_TypeDef;



typedef struct{
	u16 start;	// 0:stop 1:run:2:run-pause
	u16 pause;	//0:No_pause 1:pause
	u16 lock;		//0:No_lock 1:lock
	u16 paras_sel;//current work parasmeters selected 
	u16 factory_sel;
	float k_factor;
	float b_factor;
	
	//work parasmeters
	u16 use_level;
	u16 use_run_time;
	u16 use_pause_time;
	u16 use_cycle;
	u16 use_dir;
	//background settigns parameters 
	u16 set_level;
	u16 set_run_time;
	u16 set_pause_time;	
	u16 set_cycle;
	u16 set_dir;
	//background program parameters 
	u16 prog_level;
	u16 prog_run_time;
	u16 prog_pause_time;	
	u16 prog_cycle;
	u16 prog_dir;
	//main page display parameters during running state
	u16 disp_run_time;
	u16 disp_pause_time;
	u16 disp_cycle;
	u32 disp_count_time;
	u16 pre_page;//the last page,use for reback last work page
	//nor flash cache variable//
	u16 logo;     //0:on  1:off
	int language;
	u16 light;
	int alarm;    //0:on  1:off
	u16 shake;
 	u16 prog_grp; //0:test mode,1-20 program mode
	u16 used_time_h;
	u16 used_time_l;
	/********************/
	u32 used_time;
	u16 disp_time;
	/********************/
	u16 test_mode;//0:test mode 1:program mode 
	u16 shak_warning;
	u16 product_id;
	
	
	u16 passcode;
	u16 product_tpage;
	///////bg paras settings/////////
	u16 lid_cmd;
	///////select bowl/////////	
	u16 sel_bowll;
	u16 state;
	u16 run_timer_state;  //0 run 1pause
	
	u16 fan_tick;
	u16 bowl_tick;
	u16 nm_dir_flag;
	u16 fsm_tick;
	u16 tank1_install;
	u16 tank2_install;
	u16	tank1_material;
	u16 tank2_material;
	u16 fsm_state;    //0 ready 1 pause
	u16 start_close;
	u16 pc_run_state;
	u16 input_disp1;
	u16 input_disp2;
	u16 input_disp3;
	u16 input_disp4;	
}STR_WIN_TypeDef;


//START_WIN�ṹ��
extern STR_WIN_TypeDef xdata win_str_data;

typedef enum
{
	MAIN_TIMER=1,
	SET_TIMER=2,
	PROG_TIMER=4,
}Timer_CH_Def;

void win_disp_level(u16 ch,u16 val);
void win_disp_cycle(u16 ch,u16 val);
void win_disp_run_time(u16 ch,u16 val);
void win_disp_pause_time(u16 ch,u16 val);
void win_disp_dir(u16 ch, u16 val);
void win_disp_count_time(u16 ch,u32 val);
u16 win_sec_timer_handle(u16* stage);

void win_init(void);
void win_change_page(u16 page);
u16 win_level_trans_volt(u16 level);
void win_fresh_timer_block(void);
void win_fresh_window(SysDataDef *sys);
void Fresh_Service_Time(void);
void disp_input_icon(void);
#endif

