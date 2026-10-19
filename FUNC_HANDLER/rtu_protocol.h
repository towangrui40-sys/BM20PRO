#ifndef __RTU_PROTOCOL_H_
#define __RTU_PROTOCOL_H_

#define RTU_PROTORCAL_VERSION		1

#define RTU_MEM(x) (x-RTU_MEM_BASE)

#if (RTU_PROTORCAL_VERSION == 1)
enum RTU_MOTOR_ADDR
{
/////////////motor work parameters////////
	ADDR_GET_MEMSX=0,
	ADDR_GET_MEMSY,
	ADDR_GET_MEMSZ,
///////////////////////////////////////	
	ADDR_SET_MOTOR,
	ADDR_SET_SPEED,
	ADDR_READ_SPEED,
	ADDR_MOTOR_STATE,
	ADDR_MOTOR_CURRENT,
///////////////////////////////////////		
	ADDR_SET_LED,
	
	ADDR_SET_AC1,
	ADDR_SET_AC2,
	ADDR_SET_AC3,

	ADDR_SET_CASE,
	ADDR_SET_LOCK,
	ADDR_CASE_STATE,
	ADDR_LOCK_STATE,
	ADDR_LOCATION_TEST,
	ADDR_TANK_LOCK,
	
	ADDR_HREG_TOTAL,
	RTU_MEM_BASE= 0x1000,
};
#endif
typedef struct
{
	unsigned short* dat;
}RTU_MemDef;


#define MAX_BUFFER_NUMBER 8
typedef struct 
{
	char rxdata_buffer[MAX_BUFFER_NUMBER][256];
	unsigned char pending_data;
	unsigned char processed_data;
	unsigned char event;
	unsigned char repeat;	
	unsigned char cmd[MAX_BUFFER_NUMBER];
	unsigned int 	cmd_data[MAX_BUFFER_NUMBER];
	unsigned char pending;
	unsigned char processed;
	unsigned char sta;
	unsigned char funcode;
	unsigned short addr;
}RTU_TypeDef;

typedef enum
{
    EV_READY            = 1<<0,         /*!< Startup finished. */
    EV_FRAME_RECEIVED   = 1<<1,         /*!< Frame received. */
    EV_EXECUTE          = 1<<2,         /*!< Execute function. */
    EV_FRAME_SENT       = 1<<3,          /*!< Frame sent. */
    EV_TIME_OUT      	 	= 1<<4,          /*!< receive timer out. */
	EV_REPEAT			  = 1<<5,
} RTU_EventType;


extern RTU_MemDef usRegHoldingBuf[ADDR_HREG_TOTAL];
extern RTU_TypeDef xdata rtu_structure;//main and module


void rtu_protocol_format_rd(u16 addr,u16 length);

void rtu_protocol_format_wr(u16 addr,u16 length,u16* senddata);

void rtu_data_init(void);

u8  rtu_response_handler(void);

#endif 
