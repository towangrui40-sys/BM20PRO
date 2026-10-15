/*
T5L 基础数据读取写入功能
*/

#include "T5LDriver.h"

// T5L OS CPU初始化
void T5L_InitCPU()
{
	CKCON=0x00; //CPU 运行在 1T 模式
	T2CON=0x70; //配置扩展中断系统，定时器2 运行在 Autoload 模式
	DPC=0x00;  //C51 开发必须为 0
	PAGESEL=0x01; // 64kb 代码空间
	D_PAGESEL=0x02;	//DATA RAM 08000-0FFFF  xdata 空间
	MUX_SEL=0x60;	//把UART2、UART3 引出来，WDT关闭
	RAMMODE=0x00;	//不占用DGUS变量存储器
	PORTDRV=0x01;   //IO驱动强度 +/-8mA

	IEN0=0x00;      //关闭所有中断
	IEN1=0x00;
	IEN2=0x00;

	IP0=0x00;		//中断优先级用默认值
	IP1=0x00;

	P0=0x00;        //IO配置，输出关闭，是不是输出取决于P0MDOUT
	P1=0x00;
	P2=0x00;
	P3=0x00;

	P0MDOUT=0x50;	//P0 0101 0000
	P1MDOUT=0x00;	//P1 0000 0000
	P2MDOUT=0x00;	//P2 0000 0000
	P3MDOUT=0x00;	//P3 0000 0000

	PCON=0;
	ADCON=0x80;		//UART2 配置 8N1 9600
	SCON0=0x50;
	//波特率设置,公式为:
	//SMOD=0  SREL0H:L=1024-主频/(64*波特率),SMOD=1	 SREL0H:L=1024-主频/(32*波特率)
	//当晶振为11.0592时  主频= 11.0592*56/3=206.4384 MHz
	SREL0H=0x02; //0x03;	//波特率=FCLK/64*(1024-SREL)
	SREL0L=0xb0; //0xE4;
	ES0=1;

	SCON1=0x90;			//UART3 配置 8N1 115200
	SREL1H=0x03;	//波特率=FCLK/32*(1024-SREL1)
	SREL1L=0xC8;

	SCON2T=0x80;		//UART4 配置 8N1 115200
	SCON2R=0x80;
	BODE2_DIV_H=0x00;	//波特率=FCLK/8*DIV  T5L  s4_s5_25804800/9600=2688=0X0A80 S4_S5_ 115200=00E0   s2_115200=03E4 S2_9600=02B0
	BODE2_DIV_L=0xe0;    //波特率为： 115200
	ES2T=1;
	ES2R=1;	
	
	SCON3T=0x80;		//UART5 配置 8N1 115200
	SCON3R=0x80;
	BODE3_DIV_H=0x00;	//波特率=FCLK/8*DIV
	BODE3_DIV_L=0xE0;

	TMOD=0x11;			//T0 16bit定时器
	TH0=0x00;
	TL0=0x00;
	TR0=0;
	TCON=0x05;			//EX0 EX1是下跳沿中断

	TH1=0x00;			//T1 16bit定时器
	TL1=0x00;
	TR1=0;
	//计算方法： CPU_CLK= (晶振频率*56)/3
	//当晶振频率为 11.0592则 CPU_CLK/12=17.2032 MHz 或 CPU_CLK/24 = 8.6016 MHz
	//定时时间设置为： 65536 - 5000*8.6016
	T2CON=0xF1;			//T2 16bit Autoload模式   主频/24
	TH2=0x00;
	TL2=0x00;
	TRL2H=0x58;			//5 mS定时器
	TRL2L=0x00;
	ET2=1;  //允许定时中断
	TF2=0;
	TR2=1; //启动定时中断

}

//读取DGUS变量数据 dgus_addr=DGUS变量地址  startNo=数据缓冲字节位号  len=读取数据字节长度
//返回0 成功 ， 返回非零 失败
unsigned char T5L_ReadDgusBytes(unsigned int dgus_addr,unsigned char* dataPtr,unsigned int len)
{
	unsigned int i;

	if (len==0) return 1; //超出缓冲区
	i=(unsigned char)(dgus_addr & 0x01);
	dgus_addr >>=1;
	
	EA=0;
	ADR_H=0x00;
	ADR_M=(unsigned char)(dgus_addr>>8);
	ADR_L=(unsigned char)(dgus_addr);
	ADR_INC=0x01;
	RAMMODE=0xAF;
	while (APP_ACK==0);
	
	if(i==1) //处理奇数起始地址
	{
		if(len==1)
		{
			RAMMODE=0xa2;
		}
		else
		{
			RAMMODE=0xa3;
		}
		APP_EN=1;
		while (APP_EN==1);
		if(len==1)
		{
			*dataPtr++=DATA1;
			len=0;
		}
		else
		{
			*dataPtr++=DATA1;
			*dataPtr++=DATA0;
			len-=2;
		}
	}
	while (len>0)
	{
		switch(len)
		{
			case 1:
				RAMMODE=0xA8;
				APP_EN=1;
				while (APP_EN==1);				
				*dataPtr++=DATA3;
				len=0;
				break;
			case 2:
				RAMMODE=0xAC;
				APP_EN=1;
				while (APP_EN==1);				
				*dataPtr++=DATA3;
				*dataPtr++=DATA2;
				len=0;
				break;
			case 3:
				RAMMODE=0xAE;
				APP_EN=1;
				while (APP_EN==1);				
				*dataPtr++=DATA3;
				*dataPtr++=DATA2;
				*dataPtr++=DATA1;
				len=0;
				break;
			default:
				RAMMODE=0xAF;
				APP_EN=1;
				while (APP_EN==1);				
				*dataPtr++=DATA3;
				*dataPtr++=DATA2;
				*dataPtr++=DATA1;
				*dataPtr++=DATA0;
				len-=4;
		}
	}
	RAMMODE=0x00;
	EA=1;
	return 0;
}

//写DGUS变量数据 addr=DGUS变量地址  startNo 缓存的起始字节位值  len=写入数据字节长度
unsigned char T5L_WriteDgusBytes(unsigned int dgus_addr,unsigned char* dataPtr,unsigned int len)
{
	unsigned int i;

	if (len==0) return 1; 

	i=(unsigned char)(dgus_addr&0x01); //起始地址奇偶判断
	dgus_addr >>=1;
	EA=0;
	ADR_H=0x00;
	ADR_M=(unsigned char)(dgus_addr>>8);
	ADR_L=(unsigned char)(dgus_addr);
	ADR_INC=0x01;
	RAMMODE=0x8F;
	while (APP_ACK==0);
	if(i==1)
	{
		if(len==1)
		{
			RAMMODE=0x82;
			DATA1=*dataPtr++;
			len=0;
		}
		else
		{
			RAMMODE=0x83;
			DATA1=*dataPtr++;
			DATA0=*dataPtr++;			
			len-=2;
		}
		APP_EN=1;
		while (APP_EN==1);
	}
	
	while(len>0)
	{
		switch(len)
		{
			case 1:
				RAMMODE=0x88;
				DATA3=*dataPtr++;
				len=0;
				break;
			case 2:
				RAMMODE=0x8c;
				DATA3=*dataPtr++;
				DATA2=*dataPtr++;
				len=0;
				break;
			case 3:
				RAMMODE=0x8e;
				DATA3=*dataPtr++;
				DATA2=*dataPtr++;
				DATA1=*dataPtr++;				
				len=0;
				break;
			default:
				RAMMODE=0x8f;
				DATA3=*dataPtr++;
				DATA2=*dataPtr++;
				DATA1=*dataPtr++;				
				DATA0=*dataPtr++;
				len-=4;
				break;
		}
		APP_EN=1;
		while (APP_EN==1);
	}
	RAMMODE=0x00;
	EA=1;
	return 0;
}
