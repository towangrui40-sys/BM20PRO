#include "sys.h"
#include "usart.h"
#include "crc_calc16.h"
#include "rtu_api.h"
#include "rtu_protocol.h"

#include "start_win.h"

RTU_MemDef xdata usRegHoldingBuf[ADDR_HREG_TOTAL];

RTU_TypeDef xdata rtu_structure;//main and module

void rtu_protocol_format_rd(u16 addr,u16 length)//read hold reg
{
	//rtu protocal build package  
	u8 idata num =0;
	u16 idata tx_crc_value=0;
	u16 xdata addr_temp=0;
	u8 xdata package[128]={0};

	addr_temp = RTU_MEM_BASE +addr;	
	rtu_structure.addr = addr_temp;
	rtu_structure.funcode = RTU_FUNC_CODE_RD;
	package[num++] = rtu_structure.sta ;
	package[num++] = RTU_FUNC_CODE_RD;
	package[num++] = (addr_temp>>8)&0xff;
	package[num++] = addr_temp&0xff;

	package[num++] = (length>>8)&0xff;
	package[num++] = length&0xff;

	tx_crc_value = CRC16_MODBUS(package,num);
	package[num++] = tx_crc_value&0xff;
	package[num++] = (tx_crc_value>>8)&0xff;
	//send
  TxArrayUart(package,num);	
}
void rtu_protocol_format_wr(u16 addr,u16 length,u16* senddata)
{
	//rtu protocal build package  
	u8 num =0;
	u8 i;
	u8 error_code = 0;
	u16 tx_crc_value=0;
	u16 xdata addr_temp=0;
	u8 xdata package[128]={0};
	
	addr_temp = RTU_MEM_BASE +addr;
	package[num++] = rtu_structure.sta ;
	if(length == 1)
	{
		package[num++] = RTU_FUNC_CODE_WR;
		package[num++] = (addr_temp>>8)&0xff;
		package[num++] = addr_temp&0xff;
	}
	else
	{
		package[num++] = RTU_FUNC_CODE_CWR;
		package[num++] = (addr_temp>>8)&0xff;
		package[num++] = addr_temp&0xff;
		package[num++] = (length>>8)&0xff;
		package[num++] = length&0xff;	
	}	
	for(i=0;i<length;i++)
	{
		package[num++] = (senddata[i]>>8)&0xff;	
		package[num++] = senddata[i]&0xff;	
	}

	tx_crc_value = CRC16_MODBUS(package,num);
	
	package[num++] = tx_crc_value&0xff;
	package[num++] = (tx_crc_value>>8)&0xff;
	//send
  TxArrayUart(package,num);	
}	


void rtu_data_init(void)
{
	memset(&rtu_structure,0,sizeof(rtu_structure));
	rtu_structure.sta = 0x03;// rtu slave station no
	rtu_structure.event = EV_READY;
}

static void m_dispose_finished(unsigned char* buffer, unsigned char size)
{
	  memcpy((void*)rtu_structure.rxdata_buffer[rtu_structure.pending_data], buffer, size);
    rtu_structure.pending_data = ++rtu_structure.pending_data % MAX_BUFFER_NUMBER;
}

static u8 rtu_rec_dispose(void)
{
	unsigned short idata reclen,num;
	u8 xdata receivebuf[256];
	unsigned char  ret = 0;
	unsigned char idata rx_data;
		
	if(Uart2_Receive_Data(receivebuf,&reclen) == 0x01)
	{
		for(num = 0;num<reclen;num++)
		{
			rx_data = receivebuf[num];
			
			ret = API_SerailLoop(rx_data, &m_dispose_finished);
		}
	}
	else
	{
		ret = 0;
	}
	return ret;
}
u8 rtu_response_handler(void)
{
  unsigned char idata* response = 0;	
	switch(rtu_structure.event)
	{
		case EV_EXECUTE:
			if(rtu_rec_dispose() ==1)
			{
				
				rtu_structure.event = EV_FRAME_RECEIVED;
			}
			else
			{
				if(str_usart_data.timeout == 0)
					rtu_structure.event = EV_TIME_OUT;			
			}
			
		break;
		case EV_FRAME_RECEIVED:
			if(rtu_structure.processed_data != rtu_structure.pending_data)
			{            
				//rtu_structure.event = C_EV_FRAME_RECEIVED;
				// Dispose Data and make response
				response = API_Response(rtu_structure.rxdata_buffer[rtu_structure.processed_data]);      
				rtu_structure.processed_data = ++rtu_structure.processed_data % MAX_BUFFER_NUMBER;
				
				rtu_structure.addr = 0;
				rtu_structure.funcode = 0;
				rtu_structure.event = EV_READY;
			}
			
		break;
		case EV_REPEAT:				
		break;
		case EV_TIME_OUT:
			rtu_structure.event	=EV_REPEAT;
		break;
		default: break;
	}	
	return 0;
}
