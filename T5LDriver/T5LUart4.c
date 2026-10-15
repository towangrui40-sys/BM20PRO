/*
	HMI 上位机通讯  ModBusSlave 115200,n,8,1  ，通讯地址 1 ，用于在线更新载程序和更新资源文件
	接收数据写入 HmiSlvBuff 缓冲区中
*/
#include "T5LDriver.h"
#include "T5LUart4.h"
#include "crc16table.h"

//变量定义
struct TX_SLVCMD_STRUCT data HmiSlvCmd;
union TX_SLVBUFF_STRUCT xdata HmiSlvBuff;
unsigned char xdata HmiSlvSendBuff[250]; //发送报文长度

/*

//使用说明 拷贝到定时器中 5ms ，完成通讯的超时异常处理
	if (HmiSlvCmd.Status==1){
		HmiSlvCmd.Delay++; 
		if (HmiSlvCmd.Delay>0) HmiSlvCmd.Status=2; 
	}
	if (HmiSlvCmd.Step>0 || HmiSlvCmd.Status>2) //ModBus 接收数据超时判断
	{
		HmiSlvCmd.TxTimeOut++; //超时接收发送时间 100ms
		if (HmiSlvCmd.TxTimeOut>20)
		{
			HmiSlvCmd.TxTimeOut=0;
			HmiSlvCmd.Step=0;
			HmiSlvCmd.Status=0;
		}
	}
	else HmiSlvCmd.TxTimeOut=0;


*/

void sys_uart4_init()//115200
{
	SCON2T=0x80;		//UART4 配置 8N1 115200
	SCON2R=0x80;
	BODE2_DIV_H=0x00;	//波特率=FCLK/8*DIV  T5L  s4_s5_25804800/9600=2688=0X0A80 S4_S5_ 115200=00E0   s2_115200=03E4 S2_9600=02B0
	//BODE2_DIV_L=0x1c;    //波特率为： 921600
	BODE2_DIV_L=0xe0;    //波特率为： 115200
	ES2T=1;
	ES2R=1;
}

//串口4发送中断
void T5L_Uart4_TX() interrupt 10 using 2
{
	
	SCON2T&=0xfe;

	if (HmiSlvCmd.Status==3)
	{
		HmiSlvCmd.TxTimeOut=0;
		if (HmiSlvCmd.Length>=HmiSlvCmd.SendLength)
		{
			HmiSlvCmd.Status=0;
			HmiSlvCmd.Step=0;
		}
		else SBUF2_TX=HmiSlvSendBuff[HmiSlvCmd.Length++];
	}
}
//接收中断
void T5L_Uart4_RX() interrupt 11 using 2
{
	unsigned int data rData,tmp;
	rData=SBUF2_RX;
	SCON2R&=0xfe;
	if (HmiSlvCmd.Status>0) return;
	HmiSlvCmd.TxTimeOut=0;
	switch (HmiSlvCmd.Step){
	case 0://判断地址
		if (rData==HmiSlvCmd.Addr)
		{
			HmiSlvBuff.Value[0]=rData;
			HmiSlvCmd.Length=1;
			HmiSlvCmd.Step=1;
			HmiSlvCmd.JsCrc.Data=0xffff;
			tmp=CRC16_TABLE[(HmiSlvCmd.JsCrc.Data^rData)&0x00ff];
			HmiSlvCmd.JsCrc.Data=(HmiSlvCmd.JsCrc.Data>>8)^tmp;
		}
		break;
	case 1://获取报文包头块
		HmiSlvBuff.Value[HmiSlvCmd.Length++]=rData;			
		if (HmiSlvCmd.Length<7)
		{
			tmp=CRC16_TABLE[(HmiSlvCmd.JsCrc.Data^rData)&0x00ff];
			HmiSlvCmd.JsCrc.Data=(HmiSlvCmd.JsCrc.Data>>8)^tmp;
		}
		if(HmiSlvCmd.Length==7 && HmiSlvBuff.Item.Command==0x10)
		{
			tmp=CRC16_TABLE[(HmiSlvCmd.JsCrc.Data^rData)&0x00ff];
			HmiSlvCmd.JsCrc.Data=(HmiSlvCmd.JsCrc.Data>>8)^tmp;				
			HmiSlvCmd.RxLength=9+HmiSlvBuff.ItemX.ByteCount;
			if(HmiSlvCmd.RxLength>252) 
			{
				HmiSlvCmd.Step=0;
				break;
			}
			HmiSlvCmd.Step=2;
			break;
		}			
		if (HmiSlvCmd.Length>=8){ //功能码3 和 6
			if (HmiSlvCmd.JsCrc.Value[0]==HmiSlvBuff.Item.LowCrc && HmiSlvCmd.JsCrc.Value[1]==HmiSlvBuff.Item.HiCrc)
			{//接收正确					
				HmiSlvCmd.Delay=0;
				HmiSlvCmd.Status=2; //数据接收完成   通讯转发需要延迟 Status=1
			}
			HmiSlvCmd.Step=0;
		}
		break;
	case 2: //0x10
		HmiSlvBuff.Value[HmiSlvCmd.Length++]=rData;			
		if (HmiSlvCmd.Length<(HmiSlvCmd.RxLength-1))
		{
			tmp=CRC16_TABLE[(HmiSlvCmd.JsCrc.Data^rData)&0x00ff];
			HmiSlvCmd.JsCrc.Data=(HmiSlvCmd.JsCrc.Data>>8)^tmp;
		}
		if (HmiSlvCmd.Length>=HmiSlvCmd.RxLength){
			if (HmiSlvCmd.JsCrc.Value[0]==HmiSlvBuff.Value[HmiSlvCmd.RxLength-1] && HmiSlvCmd.JsCrc.Value[1]==HmiSlvBuff.Value[HmiSlvCmd.RxLength-2])
			{//接收正确
				
				HmiSlvCmd.Delay=0;
				HmiSlvCmd.Status=2; //数据接收完成  通讯转发需要延迟 Status=1
			}
			HmiSlvCmd.Step=0;
		}			
		break;
	default:
		HmiSlvCmd.Step=0;
		break;
	}	
}

//初始显示通讯程序
void T5L_HMIslvInit()
{
	HmiSlvCmd.Addr=1; //ModBus 地址为1
	HmiSlvCmd.Status=0;
	HmiSlvCmd.Step=0;	
	HmiSlvCmd.TxTimeOut=0;
}
//解析处理数据
unsigned char HMICommand()
{
	unsigned char i;
	unsigned int tmp;
	unsigned char retVal;
// 返回0 不发送数据 返回1发送数据
	retVal=0;
	switch (HmiSlvBuff.Item.Command)
	{
	case 3: //读取
		if (HmiSlvBuff.Item.DataValue==0 || HmiSlvBuff.Item.DataValue>115)
		{//异常或读取内容禁止处理		
			return 0;
		}
		HmiSlvSendBuff[0]=HmiSlvCmd.Addr;
		HmiSlvSendBuff[1]=3;
		HmiSlvSendBuff[2]=HmiSlvBuff.Item.DataValue*2;
		HmiSlvCmd.SendLength=HmiSlvSendBuff[2]+5;
		tmp=HmiSlvBuff.Item.RegisterAddr*2;
		T5L_ReadDgusBytes(HmiSlvBuff.Item.RegisterAddr,&HmiSlvSendBuff[3],HmiSlvSendBuff[2]);
		retVal=1;
		break;
	case 6: //写入
		T5L_WriteDgusBytes(HmiSlvBuff.Item.RegisterAddr,&HmiSlvBuff.Value[4],2);
		HmiSlvCmd.SendLength=8;
		for (i=0;i<6;i++)
		{
			HmiSlvSendBuff[i]=HmiSlvBuff.Value[i];
		}
		retVal=1;
		break;
	case 16: //批量写入
		if(HmiSlvBuff.ItemX.DataValue==0 || HmiSlvBuff.ItemX.DataValue>125) return 0; //不能超过125个 word
		T5L_WriteDgusBytes(HmiSlvBuff.Item.RegisterAddr,HmiSlvBuff.ItemX.uint8,HmiSlvBuff.ItemX.ByteCount);		
		HmiSlvCmd.SendLength=8;
		for (i=0;i<6;i++)
		{
			HmiSlvSendBuff[i]=HmiSlvBuff.Value[i];
		}
		retVal=1;
		break;
	default:
		return 0;
	}
	return retVal;
}
//远程通讯响应子程序 返回2 解析命令
unsigned char T5L_HMIslvSub()
{
	unsigned char i;
	unsigned int tmp;
	unsigned char retVal;
	if (HmiSlvCmd.Status!=2) return 0;
	retVal=HMICommand();
	if (retVal==0)
	{
		HmiSlvCmd.Status=0;//无返回
		HmiSlvCmd.Step=0;
		return 0;
	}
	
	HmiSlvCmd.JsCrc.Data=0xffff;
	for (i=0;i<HmiSlvCmd.SendLength-2;i++){//计算包头块CRC
		tmp=CRC16_TABLE[(HmiSlvCmd.JsCrc.Data^HmiSlvSendBuff[i])&0x00ff];
		HmiSlvCmd.JsCrc.Data=(HmiSlvCmd.JsCrc.Data>>8)^tmp;
	}
	HmiSlvSendBuff[HmiSlvCmd.SendLength-2]=HmiSlvCmd.JsCrc.Value[1];
	HmiSlvSendBuff[HmiSlvCmd.SendLength-1]=HmiSlvCmd.JsCrc.Value[0];

	//发送数据
	HmiSlvCmd.Step=0;
	HmiSlvCmd.Length=1;
	HmiSlvCmd.Status=3;
	SBUF2_TX=HmiSlvSendBuff[0];
	return retVal;
}

