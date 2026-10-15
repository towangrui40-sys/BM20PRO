#include "func_handler.h"
#include "start_win.h"
//////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////button paras config///////////////////////////
//////////////////////////////////////////////////////////////////////////////////
/*********************��ά����********************/
code const u16 btn_click_pararlist[BTN_EVENT_NUM][2]={   //BTN_EVENT_NUM   ��ť���� ����ʵ�ʸ���
{BTN_MAIN_PAGE_ADDR,			BTN_TYPE_CLICK},//paras input 
{BTN_MANUAL_PAGE_ADDR,			BTN_TYPE_CLICK},
{BTN_PASS_KEYPAD_ADDR,			BTN_TYPE_CLICK},
{BTN_PROG_NUM_SEL_ADDR,			BTN_TYPE_CLICK},
{BTN_PARA_KEYPAD_ADDR,			BTN_TYPE_CLICK},
{BTN_FAC_SET_ADDR,				BTN_TYPE_CLICK},
{BTN_SEL_BOWL_ADDR,		    	BTN_TYPE_CLICK},
{BTN_TEST_MODE_SET_ADDR,		BTN_TYPE_HOLD_RES},
{BTN_PROG_MODE_SET_ADDR,		BTN_TYPE_HOLD_RES},

};
//////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////

bit is_btn_scan = 0;//是否要进行按钮点击扫描

xdata STR_BTN_TypeDef btn_str_data[BTN_EVENT_NUM];



static void btn_InitStructure(STR_BTN_TypeDef *obj,u16 index)
{
	obj->addr = btn_click_pararlist[index][0];//button address
	obj->type = btn_click_pararlist[index][1];//button type
	switch(obj->type)
	{
		case BTN_TYPE_CLICK:
				obj->func_callback = btn_click_callback;
			break;
		case BTN_TYPE_HOLD_RES:
				obj ->func_callback = btn_hold_res_callback;
			break;
		case BTN_TYPE_HOLD_TIME:
				obj->func_callback = btn_hold_click_callback;
				obj->func_hold_callback = btn_hold_callback;
			break;
		default:
				obj->func_callback = 0;
				obj->func_hold_callback = 0;
			break;
	}
	obj->val =0;
	obj->val_temp = 0;
	obj->timer = 0;
}
static void btn_click_event(STR_BTN_TypeDef *obj)
{
	u16 val;
	sys_read_vp(obj->addr,(u8*)&obj->val,1);
	switch(obj->type)
	{
		case BTN_TYPE_CLICK:
			//检查是否有按钮按下
			if(obj->val !=0)
			{
				//按钮点击事件处理,传递给相应的界面	
				obj->func_callback(obj->addr,obj->val);			
				//清除按键值
				obj->val = 0;
				sys_write_vp(obj->addr,(u8*)&obj->val,1);	
			}
			break;
		case BTN_TYPE_HOLD_RES:
			if(obj->val !=obj->val_temp)
			{
				obj->func_callback(obj->addr,obj->val);			
				obj->val_temp = obj->val;
				val=1;
				sys_write_vp(HOLD_ACTIVE_ADDR,(u8*)&val,1);
				val=0;
				sys_write_vp(HOLD_PASSIVE_ADDR,(u8*)&val,1);
			}				
			break;
		case BTN_TYPE_HOLD_TIME://click event,selected func
			if(obj->val !=0)
			{		
				if((obj->timer <10)&&(obj->val != obj->val_temp))
				{
					obj->func_callback(obj->addr,obj->val);	
				}
				if(obj->timer ==20)// 600ms hold event,popup shotkey select menu
				{
					obj->func_hold_callback(obj->addr,obj->val);
					//清除按键值
					obj->val = 0;
					sys_write_vp(obj->addr,(u8*)&obj->val,1);		
				}
				obj->timer++;
			}
			else
			{
				obj->timer = 0;
			}
			obj->val_temp = obj->val;
			break;
		default:
			break;
	}
}

void btn_init(void)
{
	u16 i;
	u16 val;
	for(i=0;i<BTN_EVENT_NUM;i++)
	{
		btn_InitStructure(&btn_str_data[i],i);
	}
	//init hold button reg
	val=1;
	sys_write_vp(HOLD_ACTIVE_ADDR,(u8*)&val,1);
	val=0;
	sys_write_vp(HOLD_PASSIVE_ADDR,(u8*)&val,1);
}

//用于产生按钮点击扫描信号,放在1ms的定时器中调用即可
void btn_click_tick()
{
	static idata u8 tick = 0;
	tick++;
	if(tick==BTN_SCAN_PERIOD)
	{
		tick = 0;
		is_btn_scan = 1;//产生一个按钮点击扫描信号
	}
}

//按钮点击事件扫描并处理
void btn_click_handler(void)
{
	u16 i;

	if(is_btn_scan==0)//是否有扫描信号,没有的话就啥也不做,直接返回
		return;
	
	is_btn_scan = 0;//清除

	for(i=0;i<BTN_EVENT_NUM;i++)
	{
		btn_click_event(&btn_str_data[i]);
	}
}










