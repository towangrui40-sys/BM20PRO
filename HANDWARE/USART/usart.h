#ifndef __USART_H_
#define __USART_H_


typedef struct{
	u8 overtime;//口2超时计数器
	u8 timeout;
	u8 tx_busy;
	u8 tx_len;
	u8 tx_length;
	u8 rx_busy;
	u8 rx_cnt;
	u8 tx_buf[64];
	u8 rx_buf[256];
}STR_USART_TypeDef;

extern STR_USART_TypeDef  str_usart_data;

void sys_uart0_init();//115200

void TxArrayUart(unsigned char *ptr,unsigned int number);
u8  Uart2_Receive_Data(u8 *ptr,u16 *cnt);
#endif 




