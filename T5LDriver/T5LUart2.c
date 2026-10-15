#include "T5LDriver.h"
#include "T5LUart2.h"
   
void T5L_Uart2() interrupt 4 using 2
{
	unsigned int data tmp;
	if (TI0)
	{
		TI0=0;
	}
	if (RI0)  //RI Ω” ’
	{
		tmp=SBUF0;
		RI0=0;
	}
}
