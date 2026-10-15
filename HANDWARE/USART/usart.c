#include "sys.h"
#include "usart.h"
#include "string.h"

STR_USART_TypeDef xdata str_usart_data;

void sys_uart0_init()//115200
{
/****************************************************************************/
	MUX_SEL |= 0x40;    //UART2引出,UART3不引出，WDT关闭
	P0MDOUT|=0x10;	  //P0.4 TXD output pp
	SCON0 = 0x50;		//mode 1:8bit uart,enable rx
	ADCON = 0X80;		//BAUD USE SREL0H:L
	PCON &= 0X7F;			//disable multiple frequency
	SREL0H = 0X03;	//L:H = 1024-CPU_FRQ/64/115200  
	SREL0L = 0XE4;
	
	IEN0 |= 0X10;//enable uart2 inter rx tx
	
	memset(&str_usart_data,0,sizeof(str_usart_data));
}

/****************************************************************************/
void  OneSendData0(u8 Dat)
{           
	while (str_usart_data.tx_busy);               
	str_usart_data.tx_busy = 1;
	SBUF0 = Dat; 
	//while(TI0 == 0);
	//TI0=0;
}
/*******************************************
向串口发送数组，查询方式
参数1:数组指针;
参数2:数组长度;
*******************************************/
void TxArrayUart(unsigned char *ptr,unsigned int number) 
{
	u8 idata i;
	for(i = 0; i < number; i++)
	{
	 OneSendData0(ptr[i]);
	}
}
u8  Uart2_Receive_Data(u8 *ptr,u16 *cnt)
{			
	u8 idata res;
	u8 idata i;
	if(str_usart_data.rx_busy == 1)
	{		
		if(str_usart_data.overtime == 0)
		{
			for(i = 0;i<str_usart_data.rx_cnt;i++)
			{
				ptr[i] = str_usart_data.rx_buf[i];
			}
			*cnt = str_usart_data.rx_cnt;
			str_usart_data.rx_busy = 0;
			str_usart_data.rx_cnt = 0;
			res = 1;//success		
		}
		else 
		{
			res = 2;//busy
		}
	}
	else
	{
		res =0;//noting
	}
	return res;
}	
	

/****************************************************************************/
 
//********************************************************* 
void uart2_Tisr() interrupt 4 using 2 //串口2中断
{
	unsigned int data rData;
	if(TI0 == 1)
	{
		TI0=0;
//		if(str_usart_data.tx_len < str_usart_data.tx_length)
//		{		
//			SBUF0=str_usart_data.tx_buf[str_usart_data.tx_len++];					
//		}
//		else	
			str_usart_data.tx_busy=0;	
			str_usart_data.rx_cnt = 0;	
			str_usart_data.timeout=200; 		
	}
	if(RI0 == 1)
	{
		rData = SBUF0;
		RI0=0;	
		str_usart_data.rx_buf[str_usart_data.rx_cnt] =rData;
		
		str_usart_data.rx_busy=1;
		str_usart_data.rx_cnt++;
		if(str_usart_data.rx_cnt==256) str_usart_data.rx_cnt = 0;
		str_usart_data.overtime=3; 
	}
}
