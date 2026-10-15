#ifndef __FUNC_HANDLER_H__
#define __FUNC_HANDLER_H__
#include "sys.h"

//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////button paras config///////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*---------------------define arae-------------------------*/
//main page
#define BTN_LANGUAGE_ADDR			0X0FFF   //多语言

#define BTN_MAIN_PAGE_ADDR		    0x1100
#define BTN_SEL_BOWL_ADDR		    0X1102
#define BTN_MANUAL_PAGE_ADDR		0X1104

//paras input //参数设置
#define BTN_TEST_MODE_SET_ADDR		 0X1110    //实验模式设置按钮
#define BTN_PROG_MODE_SET_ADDR		 0X1112	   //程序模式设置按钮
#define BTN_PROG_NUM_SEL_ADDR		 0X1114    //程序选择
#define BTN_PARA_KEYPAD_ADDR		 0X1116    //参数设置键盘

#define BTN_SET_LIGHT_ADDR			0x1200 
#define BTN_SET_SHAKE_ADDR			0x1201
#define BTN_SET_BUZZ_ADDR			0x1202 
#define BTN_PASS_KEYPAD_ADDR		0X1203   //密码键盘	
//factory mode
#define BTN_FAC_SET_ADDR		0X1300
#define BTN_FAC_SEL_ADDR		0X1302
/*-------------------------------------------------------*/


/*----------------------config arae-----------------------------*/
#define HOLD_ACTIVE_ADDR		0x1150	//hold button active register default 1
#define HOLD_PASSIVE_ADDR		0x1152	//hold button passive register default 0

#define BTN_EVENT_NUM 		 12



#define BTN_SCAN_PERIOD		30
/*-------------------------------------------------------*/

//#define BTN_TYPE_CLICK  0
//#define BTN_TYPE_HOLD_RES  1
//#define BTN_TYPE_HOLD_TIME  2
enum BTN_TYPE { BTN_TYPE_CLICK = 0,BTN_TYPE_HOLD_RES,BTN_TYPE_HOLD_TIME};


typedef struct{
	u16 addr;
	u16 val;
	u16 val_temp;
	u16 timer;
	enum BTN_TYPE  type;
	void (*func_callback)(u16 addr,u16 val);
	void (*func_hold_callback)(u16 addr,u16 val);	
}STR_BTN_TypeDef;

void btn_init(void);
void btn_click_tick(void);
void btn_click_handler(void);


void btn_click_callback(u16 addr,u16 val);
void btn_hold_res_callback(u16 addr,u16 val);
void btn_hold_click_callback(u16 addr,u16 val);
void btn_hold_callback(u16 addr,u16 val);

#endif
