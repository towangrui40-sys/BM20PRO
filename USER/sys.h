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
	/* power on and run */
	FSM_POWER_ON = 0,                  // 开机第一步
	FSM_POWER_ON_DELAY = 14,           // 等待控制器状态稳定
	FSM_READY = 1,                     // 就绪/待机
	FSM_START = 2,                     // 启动电机
	FSM_RUN = 3,                       // 正常运行
	FSM_PAUSE_STOPPING = 6,            // 暂停前固定延时等待机械减速
	FSM_PAUSE = 4,                     // 暂停
	FSM_STOP_STOPPING = 29,            // 停止前固定延时等待机械减速
	FSM_STOP = 5,                      // 停止

	/* initialization */
	FSM_INIT_LOCK_CHECK = 24,          // 检查位置和锁状态
	FSM_INIT_WAIT_LOCK = 15,           // 等待研磨罐锁到位
	FSM_INIT_LOCKING = 22,             // 初始化上锁中
	FSM_INIT_UNLOCKING = 23,           // 初始化上锁失败，自动解锁
	FSM_INIT_RECLOSE = 16,             // 关盖后重新初始化
	FSM_INIT_LEAVE_POS2 = 12,          // 先转离2号位
	FSM_INIT_WAIT_POS2 = 13,           // 等待重新到达2号位
	FSM_POSITION_CHECK = 10,           // 开盖后检查位置
	FSM_LOCK_RESET = 17,               // 开盖后复位锁紧装置
	FSM_LOCK_RELOCK = 18,              // 等待锁紧装置重新到位

	/* tank selection */
	FSM_TANK_SELECT = 7,               // 选择研磨罐
	FSM_TANK_MOVE_POS1 = 8,            // 前往1号位
	FSM_TANK_MOVE_POS2 = 9,            // 前往2号位
	FSM_TANK_LOCK_WAIT_UNLOCK = 19,    // 等待解锁到位
	FSM_TANK_LOCK_WAIT_PRESS = 20,     // 等待把手下压
	FSM_TANK_LOCK_WAIT_LOCK = 21,      // 等待重新上锁到位

	/* close lid */
	FSM_CASE_CLOSE = 11,               // 关盖流程
	FSM_CLOSE_CLEAR_TANK_POS = 25,     // 转离1/2号位
	FSM_CLOSE_LOCK_TO_RUN = 26,        // 锁转到运行位
	FSM_CLOSE_SEND_CASE_CMD = 27,      // 发送关盖命令
	FSM_CLOSE_WAIT_CASE_DONE = 28,     // 等待盖子关到位
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


