#ifndef _T5L_UART4_H_
#define _T5L_UART4_H_

union TX_DREVCRC{
	unsigned char Value[2];
	unsigned int Data;
};

//接收数据解析结构 3  6
struct TX_SLV_UNIONHEAD{
	unsigned char Address; //本机地址
	unsigned char Command;//命令字
	unsigned int  RegisterAddr ;//寄存器地址
	unsigned int  DataValue; // 数据/数值
	unsigned char HiCrc; //
	unsigned char LowCrc;
};

struct TX_SLV_UNIONHEAD16{
	unsigned char Address; //本机地址
	unsigned char Command;//命令字
	unsigned int  RegisterAddr ;//寄存器地址
	unsigned int  DataValue; // 数据/数值
	unsigned char ByteCount; //有多少个字节数据
	unsigned char uint8[250];
};

union TX_SLVBUFF_STRUCT{
	struct TX_SLV_UNIONHEAD Item;
	struct TX_SLV_UNIONHEAD16 ItemX;
	unsigned char Value[300];
};

//通讯控制协调结构
struct TX_SLVCMD_STRUCT{
	unsigned char Addr; //本机地址
	unsigned char Step;
	unsigned char Length;
	unsigned char RxLength;
	unsigned char SendLength;
	union TX_DREVCRC JsCrc;
	unsigned char Delay;
	unsigned char Status; //=0 接收状态, =1 备用或通讯转发需要延迟, =2 发送数据
	unsigned char TxTimeOut; //接收数据间断超时
};

extern struct TX_SLVCMD_STRUCT data HmiSlvCmd;
extern union TX_SLVBUFF_STRUCT xdata HmiSlvBuff;

void sys_uart4_init();//115200
void T5L_HMIslvInit();
unsigned char T5L_HMIslvSub();
#endif