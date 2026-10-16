#ifndef __SYS_H__
#define __SYS_H__
#include "t5los8051.h"
#include <string.h>

typedef unsigned char   u8;
typedef unsigned short  u16;
typedef unsigned long   u32;
typedef signed char     s8;
typedef signed short    s16;
typedef signed long     s32;


#define	WDT_ON()				MUX_SEL|=0x02
#define	WDT_OFF()				MUX_SEL&=0xFD
#define	WDT_RST()				MUX_SEL|=0x01


#define FOSC     				206438400UL
#define T1MS    				(65536-FOSC/12/1000)





//app setting parsmetes config
#define CFG_SYS_SLEEP_TIME  180 //3min,unit 1s  


typedef enum {
	SHAPE_TYPE_POINT = 0x0001,
	SHAPE_TYPE_LINE = 0x0002,
	SHAPE_TYPE_RECT = 0x0003,
	SHAPE_TYPE_FILL_RECT = 0x0004,
	SHAPE_TYPE_AREA_COPY = 0x0006,
	SHAPE_TYPE_VER_LINE =  0x0009
}SHAPE_TYPE;

typedef struct{
	u16 vp;
	u16 shape_type;
	u16 shape_num;
	u16 x;
	u16 y;
	u16 color;
	u16 end_flag;
}SHAPE_DATA_POINT;

typedef struct{
	u16 vp;
	u16 shape_type;
	u16 shape_num;
	u16 color;
	u16 x0;
	u16 y0;
	u16 x1;
	u16 y1;
	u16 end_flag;
}SHAPE_DATA_LINE;

typedef struct{
	u16 vp;
	u16 shape_type;
	u16 shape_num;
	u16 xs;
	u16 ys;
	u16 xe;
	u16 ye;
	u16 color;
	u16 end_flag;
}SHAPE_DATA_RECT;
typedef SHAPE_DATA_RECT	 SHAPE_DATA_FILL_RECT;
	

typedef struct{
	u16 vp;
	u16 shape_type;
	u16 shape_num;
	u16 pic_id;
	u16 xs;
	u16 ys;
	u16 xe;
	u16 ye;
	u16 x;
	u16 y;
	u16 end_flag;
}SHAPE_DATA_AREA_COPY;

//Ƶ��,��ֱ��
typedef struct{
	u16 vp;
	u16 shape_type;
	u16 shape_num;
	u16 color;
	u16 x0;
	u16 y0s;
	u16 y0e;
	u16 end_flag;
}SHAPE_DATA_VER_LINE;

typedef struct {
	int fwd;
	int rev;
	int reset;
	int fault;
	int set_run;
	int set_vol;
}MOTOR_Control_Order;

typedef struct
{
    signed short x;
    signed short y;
    signed short z;
    
}MEMS_TYPEDEF;


typedef struct
{
	MOTOR_Control_Order mc_para;
	MEMS_TYPEDEF str_mems;
	int run_state;
	int current;
	int set_led;
	int set_ac1;
	int set_ac2;
	int set_ac3;
	int set_case;
	int set_lock;
	int case_state;
	int lock_state;
	int location_test;
	int install_test;
}SysDataDef;

//ϵͳ�ṹ��
extern SysDataDef str_sys;

typedef enum
{
  	FSM_INIT = 0, 
	FSM_READY,	
	FSM_START,
	FSM_RUN,
	FSM_PAUSE,
	FSM_STOP,
	FSM_SLOW_DOWN,
	FSM_SEL_BOWL,  	      
	FSM_CHECK_POSITION,  //检查转盘位置
	FSM_CASE_LOCK,
	FSM_CASE_UNLOCK,

}FSM_StateType;

typedef enum
{
    FSM_OK = 0,        
    FSM_ERROR_OPEN_OT       

}FSM_ErrorType;


//��������
void sys_init(void);
void sys_delay_about_ms(u16 ms);
void sys_delay_about_us(u8 us);
void sys_delay_ms(u16 ms);
void sys_read_vp(u16 addr,u8* buf,u16 len);
void sys_write_vp(u16 addr,u8* buf,u16 len);
void sys_draw_shape(u16 * shape_data);
void sys_disp_page(u16 page);
void sys_enable_speaker(u16 enable);
void sys_speaker_work(u16 timer);//unit 8ms
void sys_setdis_light(u16 light_val);
void sys_readflash(u16 addr,u8*buf,u16 num);
void sys_wirteflash(u16 addr,u8*buf,u16 num);
#endif


