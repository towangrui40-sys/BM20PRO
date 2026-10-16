//############################################################
// FILE:  Task_manager_H
// Created on: 2019��7��8��
// Author: XY
// summary: Header file  and definition
//������ֻ��ѧϰʹ�ã�δ���������ɣ��������������κ���;
//��Ȩ���У�����ؾ�
//STM32F4������ƿ�����
//˶�������  
//��ַ: https://shuolidianzi.taobao.com
//Author-QQ: 616264123
//�������QQȺ��314306105
//############################################################


#ifndef _Task_manager_H_
#define _Task_manager_H_
#include "sys.h"

//void (* data g_pTaskList[TASKLISTCNT])();							// ����ָ��
typedef  void ( * FTimer_P) ( void );

typedef struct {
	    u16	 Task_Period;  			//time out for calling function
	    u16  Task_Count;
	    FTimer_P Task_Function;		//Send function defines in application
	   }TaskTime;

		 
void Timer_Task_Count(void);
void Execute_Task_List_RUN(void);
void Task_Manage_List_Init(void);

#define UNLOCK          1
#define LOCK	        2	 
#define LOCK_TO_RUN     3
#define RUN_TO_LOCK   	4

#define OPEN    1
#define CLOSE	2	 
		 
#define CLOSE_PLACE 	0	 		 
#define OPEN_PLACE      1
#define MIDDLE_PLACE    2
	 
#endif  //Task_manager_H
