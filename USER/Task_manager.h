//############################################################
// FILE:  Task_manager_H
// Created on: 2019年7月8日
// Author: XY
// summary: Header file  and definition
//本程序只供学习使用，未经作者许可，不得用于其它任何用途
//版权所有，盗版必究
//STM32F4电机控制开发板
//硕历达电子  
//网址: https://shuolidianzi.taobao.com
//Author-QQ: 616264123
//电机控制QQ群：314306105
//############################################################


#ifndef _Task_manager_H_
#define _Task_manager_H_
#include "sys.h"

//void (* data g_pTaskList[TASKLISTCNT])();							// 函数指针
typedef  void ( * FTimer_P) ( void );

typedef struct {
	    u16	 Task_Period;  			//time out for calling function
	    u16  Task_Count;
	    FTimer_P Task_Function;		//Send function defines in application
	   }TaskTime;

		 
void Timer_Task_Count(void);
void Execute_Task_List_RUN(void);
void Task_Manage_List_Init(void);

#define UNLOCK  1
#define LOCK	  2	 
#define LOCK_TO_RUN     3
#define RUN_TO_LOCK   	4

#define OPEN  1
#define CLOSE	2	 
		 
#define CLOSE_PLACE	0	 		 
#define OPEN_PLACE  1
#define MIDDLE_PLACE  2
	 
#endif  //Task_manager_H
