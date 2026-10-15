#include "main.h"

static idata u16 delay_tick = 0; //用来实现精确延时的
//核心寄存器初始化
void sys_cpu_init()
{
	CKCON = 0x00;
	T2CON = 0x70;
	DPC = 0x00;
	PAGESEL = 0x01;
	D_PAGESEL = 0x02; //DATA RAM  0x8000-0xFFFF
	MUX_SEL = 0x00;   //UART2,UART3关闭，WDT关闭
	RAMMODE = 0x00;
	PORTDRV = 0x01;   //驱动强度+/-8mA
	IEN0 = 0x00;      //关闭所有中断
	IEN1 = 0x00;
	IEN2 = 0x00;
	IP0 = 0x00;       //中断优先级默认
	IP1 = 0x00;
	
	WDT_OFF();      	//关闭开门狗
}


//定时器2初始化,定时间隔为1ms
void sys_timer2_init()
{
	T2CON = 0x70;
	TH2 = 0x00;
	TL2 = 0x00;

	TRL2H = 0xBC;	//1ms的定时器
	TRL2L = 0xCD;       

	IEN0 |= 0x20;	//启动定时器2
	TR2 = 0x01;
	EA = 1;
}


//系统初始化
void sys_init()
{
	sys_cpu_init();//核心寄存器初始化
	
	sys_uart0_init();//串口2初始化 115200bps
	
	sys_uart4_init();//串口4初始化 115200bps
	
	sys_timer2_init();//定时器2初始化

}


//软件大致延时,单位ms
//如果修改了优化等级,那么此函数内部的参数需要重新调试
void sys_delay_about_ms(u16 ms)
{
	u16 i,j;
	for(i=0;i<ms;i++)
			for(j=0;j<3000;j++);    
}


//软件大致延时,单位us
//如果修改了优化等级,那么此函数内部的参数需要重新调试
void sys_delay_about_us(u8 us)
{
	u8 i,j;
	for(i=0;i<us;i++)
			for(j=0;j<5;j++);    
}


//利用定时器2进行精确延时,单位ms
void sys_delay_ms(u16 ms)
{
	delay_tick = ms;
	while(delay_tick);
}


//读DGUS中的VP变量数据
//addr:就是直接传入DGUS中的地址
//buf:缓冲区
//len:读取的字数,一个字等于2个字节
void sys_read_vp(u16 addr,u8* buf,u16 len)
{   
	u8 i; 
	
	i = (u8)(addr&0x01);
	addr >>= 1;
	ADR_H = 0x00;
	ADR_M = (u8)(addr>>8);
	ADR_L = (u8)addr;
	ADR_INC = 0x01;
	RAMMODE = 0xAF;
	while(APP_ACK==0);
	while(len>0)
	{   
		APP_EN=1;
		while(APP_EN==1);
		if((i==0)&&(len>0))   
		{   
			*buf++ = DATA3;
			*buf++ = DATA2;                      
			i = 1;
			len--;	
		}
		if((i==1)&&(len>0))   
		{   
			*buf++ = DATA1;
			*buf++ = DATA0;                      
			i = 0;
			len--;	
		}
	}
	RAMMODE = 0x00;
}


//写DGUS中的VP变量数据
//addr:就是直接传入DGUS中的地址
//buf:缓冲区
//len:被发送数据的字数,一个字等于2个字节
void sys_write_vp(u16 addr,u8* buf,u16 len)
{   
	u8 i;  
	
	i = (u8)(addr&0x01);
	addr >>= 1;
	ADR_H = 0x00;
	ADR_M = (u8)(addr>>8);
	ADR_L = (u8)addr;    
	ADR_INC = 0x01;
	RAMMODE = 0x8F;
	while(APP_ACK==0);
	if(i && len>0)
	{	
		RAMMODE = 0x83;	
		DATA1 = *buf++;		
		DATA0 = *buf++;	
		APP_EN = 1;		
		len--;
	}
	RAMMODE = 0x8F;
	while(len>=2)
	{	
		DATA3 = *buf++;		
		DATA2 = *buf++;
		DATA1 = *buf++;		
		DATA0 = *buf++;
		APP_EN = 1;		
		len -= 2;
	}
	if(len)
	{	
		RAMMODE = 0x8C;
		DATA3 = *buf++;		
		DATA2 = *buf++;
		APP_EN = 1;
	}
	RAMMODE = 0x00;
} 

//定时器2中断服务程序
void sys_timer2_isr()	interrupt 5 //1ms
{
	TF2=0;//清除定时器2的中断标志位
	
	Timer_Task_Count( );
	if(str_usart_data.overtime)	str_usart_data.overtime--;
	if(str_usart_data.timeout)	str_usart_data.timeout--;
	//精准延时处理
	if(delay_tick)		delay_tick--;
	//产生按钮点击扫描信号
	btn_click_tick();

	//通讯接收超时
	if (HmiSlvCmd.Status==1){
		HmiSlvCmd.Delay++; 
		if (HmiSlvCmd.Delay>1) HmiSlvCmd.Status=2; 
	}
	if (HmiSlvCmd.Step>0 || HmiSlvCmd.Status>2) //ModBus 接收数据超时判断
	{
		HmiSlvCmd.TxTimeOut++; //超时接收发送时间>100ms
		if (HmiSlvCmd.TxTimeOut>20)
		{
			HmiSlvCmd.TxTimeOut=0;
			HmiSlvCmd.Step=0;
			HmiSlvCmd.Status=0;
		}
	}
	else HmiSlvCmd.TxTimeOut=0;	
}


//*********************************************************

//绘制2D形状
void sys_draw_shape(u16 * shape_data)
{
	#define SHAPE_NUM		1				//只绘制一个
	#define END_FLAG		0xff00	//绘制结束标记
	u8 len;
	
	switch(shape_data[1])
	{
		case SHAPE_TYPE_POINT://绘制点
			((SHAPE_DATA_POINT*)shape_data)->shape_num = SHAPE_NUM;
			((SHAPE_DATA_POINT*)shape_data)->end_flag = END_FLAG;
			len = sizeof(SHAPE_DATA_POINT)-2;
			break;
		case SHAPE_TYPE_LINE://端点连线或线段
			((SHAPE_DATA_LINE*)shape_data)->shape_num = SHAPE_NUM;
			((SHAPE_DATA_LINE*)shape_data)->end_flag = END_FLAG;
			len = sizeof(SHAPE_DATA_LINE)-2;
			break;
		case SHAPE_TYPE_RECT://画矩形
		case SHAPE_TYPE_FILL_RECT://画填充矩形
			((SHAPE_DATA_RECT*)shape_data)->shape_num = SHAPE_NUM;
			((SHAPE_DATA_RECT*)shape_data)->end_flag = END_FLAG;
			len = sizeof(SHAPE_DATA_RECT)-2;
			break;
		case SHAPE_TYPE_AREA_COPY://界面区域拷贝
			((SHAPE_DATA_AREA_COPY*)shape_data)->shape_num = SHAPE_NUM;
			((SHAPE_DATA_AREA_COPY*)shape_data)->end_flag = END_FLAG;
			len = sizeof(SHAPE_DATA_AREA_COPY)-2;
			break;
		case SHAPE_TYPE_VER_LINE://频谱,垂直线
			((SHAPE_DATA_VER_LINE*)shape_data)->shape_num = SHAPE_NUM;
			((SHAPE_DATA_VER_LINE*)shape_data)->end_flag = END_FLAG;
			len = sizeof(SHAPE_DATA_VER_LINE)-2;
			break;		
	}
	
	sys_write_vp(shape_data[0],(u8*)(shape_data+1),len/2);
}
//切换界面
void sys_disp_page(u16 page)
{
	u16 buf[2];
	buf[0]= 0x5a01;
	buf[1] =page; 
	sys_write_vp(0X0084,(u8*)buf,2);	
	//sys_write_vp(0X0086,(u8*)buf,1);	
}
void sys_enable_speaker(u16 enable)
{
	u16 buf[2];	
	sys_read_vp(0x0080,(u8*)buf,2);
	buf[0]= 0x5a00;
	if(enable == 1) buf[1] = buf[1]|(1<<3);
	else buf[1] = buf[1]&(~(1<<3));
	sys_write_vp(0x0080,(u8*)buf,2);//debug
}
void sys_speaker_work(u16 timer)//unit 8ms
{
	sys_write_vp(0x00A0,(u8*)&timer,1);
}
void sys_setdis_light(u16 light_val)
{
	u16 buf[2];	
	if(light_val>100) light_val =100;
	
	sys_read_vp(0x0080,(u8*)buf,2);	
	buf[0]= 0x5a00;
	// turn off black light contrl
//	if(light_val == 100)
	{
		buf[1] = buf[1]&(~(1<<2));	
		sys_write_vp(0x0080,(u8*)buf,2);//debug
		buf[0]= (light_val<<8)+0x32;
		buf[1] = 100;//10s unit 10ms	
		sys_write_vp(0x0082,(u8*)buf,2);//debug
	}
//	else
//	{
//		buf[1] = buf[1]|(1<<2);
//		sys_write_vp(0x0080,(u8*)buf,2);//debug
//		buf[0]= (light_val)+(0x64<<8);
//		buf[1] = 100*CFG_SYS_SLEEP_TIME;//10s unit 10ms	
//		sys_write_vp(0x0082,(u8*)buf,2);//debug
//	}

	

}



//void sys_readflash(u16 addr,u8*buf,u16 num)
//{

//}
//void sys_wirteflash(u16 addr,u8*buf,u16 num)
//{

//}
///////////////////////////////////////////////////	
