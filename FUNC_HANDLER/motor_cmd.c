#include "sys.h"
#include "rtu_protocol.h"
#include "motor_cmd.h"

#include "main.h"

ST_MOTOR_PARAS_DEF st_motor_paras;

void motor_set_station(u8 station)
{
		st_motor_paras.station = station;
}
/*--------------------------------/
命令内容含义：
0001：正转运行（无参数）
0002：反转运行（无参数）
0003：减速停机
0004：自由停机
0005：正转点动起动
0006：正转点动停机
0007：保留
0008：运行（无方向）
0009：故障复位
000A: 反转点动起动
000B: 反转点动停机
000C：休眠唤醒
/--------------------------------*/

// rtu_protocol_format_wr     wr写
void motor_set_run(u16 paras)
{
	rtu_protocol_format_wr(ADDR_MOTOR_SET_RUN,1,&paras);
}

//void motor_set_rpm(u16 rpm)     //电机写转速
//{
//	rtu_protocol_format_wr(ADDR_MOTOR_SET_RPM,1,&rpm);
//}
//void motor_set_door(u16 enable)   //
//{
//	rtu_protocol_format_wr(ADDR_SET_DOOR,1,&enable);
//}


//void motor_get_runstate(MC_GetState state)
//{
//	switch(state)
//	{
//		case MC_IDLE:
//			rtu_protocol_format_rd(ADDR_GET_ROTORID,(ADDR_GET_ROLLER+1)-ADDR_GET_ROTORID);
//			break;
//		case MC_WORK:
//			rtu_protocol_format_rd(ADDR_MOTOR_GET_CTRLMODE,(ADDR_GET_ROLLER+1)-ADDR_MOTOR_GET_CTRLMODE);
//			break;
//		default:break;
//	}

//}

//void motor_get_rpm(void)  //rd 读
//{
//	rtu_protocol_format_rd(ADDR_MOTOR_GET_RPM,1);
//}
//void motor_get_door_lock(void)
//{
//	rtu_protocol_format_rd(ADDR_GET_DOORLOCK,1);
//}
//void motor_get_roller(void)
//{
//	rtu_protocol_format_rd(ADDR_GET_ROLLER,1);
//}

