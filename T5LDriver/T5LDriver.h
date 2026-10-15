#ifndef _T5L_DRIVER_H_
#define _T5L_DRIVER_H_
#include "T5LOS8051.h"


//º¯ÊıÔ¤¶¨Òå
void T5L_InitCPU();
unsigned char T5L_ReadDgusBytes(unsigned int dgus_addr,unsigned char* dataPtr,unsigned int len);
unsigned char T5L_WriteDgusBytes(unsigned int dgus_addr,unsigned char* dataPtr,unsigned int len);
#endif