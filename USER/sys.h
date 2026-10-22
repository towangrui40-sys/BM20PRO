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


typedef struct
{
	int str_mems_x;
	int str_mems_y;
	int str_mems_z;
	int set_motor;
	int set_speed;	
	int read_speed;		
	int motor_state;	
	int motor_current;	
	int run_state;
	int set_led;
	int set_ac1;
	int set_ac2;
	int set_ac3;
	int set_case;
	int set_lock;
	int case_state;     //盖子状态
	int lock_state;     //研磨罐锁状态 
	int location_test;  //研磨罐位置检测
	int tank_lock;      //检测把手状态传感器
}SysDataDef;

extern SysDataDef str_sys;

typedef enum
{
  	FSM_POWER_ON = 0,        // 开机第一步：判断盖子状态
	FSM_READY,               // 就绪/待机
	FSM_START,               // 启动电机
	FSM_RUN,                 // 正常运行
	FSM_PAUSE,               // 暂停
	FSM_STOP,                // 停止
	FSM_STOPPING,            // 等待电机减速到零
	FSM_TANK_SELECT,         // 选择研磨罐/等待转盘选位
	FSM_TANK_MOVE_POS1,      // 研磨罐转盘前往1号位
	FSM_TANK_MOVE_POS2,      // 研磨罐转盘前往2号位
	FSM_POSITION_CHECK,      // 检测转盘位置
	FSM_CASE_CLOSE,          // 关盖流程
	FSM_INIT_LEAVE_POS2,     // 初始化：先转离2号位
	FSM_INIT_WAIT_POS2,      // 初始化：等待重新到达2号位
	FSM_POWER_ON_DELAY,      // 开机等待控制器状态稳定
	FSM_INIT_WAIT_LOCK,      // 初始化：等待研磨罐锁到位
	FSM_INIT_RECLOSE,        // 初始化：盖子开着且不在罐位，先关盖再重开
	FSM_LOCK_RESET,          // 开盖后复位锁紧装置
	FSM_LOCK_RELOCK,         // 复位锁紧装置：重新上锁并等待到位
	FSM_LOCK_1,              // 选罐开锁：等待解锁到位
	FSM_LOCK_2,              // 选罐开锁：等待把手下压
	FSM_LOCK_3,              // 选罐开锁：上锁并检查是否锁好
	FSM_INIT_LOCKING,        // 初始化上锁：等待锁到位
	FSM_INIT_UNLOCKING,      // 初始化上锁失败：自动解锁后等待重新按压
	FSM_INIT_LOCK_CHECK,     // 初始化：检查转盘位置和研磨罐锁状态
}FSM_StateType;

typedef enum
{
    FSM_OK = 0,        
    FSM_ERROR_OPEN_OT       

}FSM_ErrorType;


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


