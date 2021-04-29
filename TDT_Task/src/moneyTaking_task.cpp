/***************
@brief ׬Ǯ������
@describe	1����������ͨ�ð��߼��������ģ�飬canͨ��
			2����������������ȡ��Ͷһ�
			3��bar_oodeΪ��ط����������ڲ������롣
			ʹ�÷�����ֱ��ʹ�ã�����extern
			���磬��bar_code.Back == 1,���������ں���
@function 



******************/



#include "moneyTaking_task.h"

/**FreeRTOS*START***************/
#include "FreeRTOS.h"					//FreeRTOSʹ��	 
#include "timers.h"
#include "list.h"
#include "queue.h"
#include "task.h"
/**FreeRTOS*END***************/
#include "motor.h"
#include "can.h"
#include "dbus.h"
#include "vision.h"
#include "ore_task.h"

_ExCmd ExCmd;
_ResetCmd ResetCmd;
_STA STA;//���ֶ�����ɵı�־λ����flagCmd��ͬ��������ɺ���1����ͨ�����ڷ��͸�����
extern Motor Climbing[2] ;
extern Motor Turning[2] ;

extern PidParam in_climb[2],out_climb[2];
extern PidParam in_turn[2],out_turn[2];
extern PidParam in_table[2],out_table[2];
extern PidParam in_clip[2],out_clip[2];
	
void SuctionRest(void)
{
 if(ResetCmd.AllDone == 1)
 {
	if( ResetCmd.ClimbDone == 0)
	  {
			ResetCmd.TimeDelay++;
	    Climbing[0].ctrlSpeed(-2000,0);
      Climbing[1].ctrlSpeed(2000,0);
		  if(abs(Climbing[0].pidInner.error) > 1800 && abs(Climbing[1].pidInner.error) > 1800 && ResetCmd.TimeDelay > 255)
	     {
				 ResetCmd.LClimbNowPos = Climbing[0].canInfo.totalEncoder;
				 ResetCmd.RClimbNowPos = Climbing[1].canInfo.totalEncoder;
				 ResetCmd.ClimbDone = 1;
       }		
	  }
	
/*		 
  if(ResetCmd.TurnDone == 0 && ResetCmd.AllDone == 1 )
    {    
      ResetCmd.TimeDelay++;
      Turning[0].ctrlSpeed(-1500,0);
      Turning[1].ctrlSpeed(1500,0);
	    if(abs(Turning[0].pidInner.error)>100 && abs(Turning[1].pidInner.error)>100 && ResetCmd.TimeDelay>30)
       {
			   ResetCmd.LTurnNowPos = Turning[0].canInfo.totalEncoder;
			   ResetCmd.RTurnNowPos = Turning[1].canInfo.totalEncoder;
				 ResetCmd.TurnDone = 1;
       }
    }
*/			
 }
	if(/* ResetCmd.TurnDone == 1 &&*/  ResetCmd.ClimbDone == 1)
  {
	 ResetCmd.AllDone = 0;
   ResetCmd.TurnDone = 0;
	 ResetCmd.ClimbDone = 0;
   __set_FAULTMASK(1); 		
   NVIC_SystemReset(); 	
	}
 
}
	

void Exchange (void)
{
	if(ExCmd.ExStart == 1)
	{
	ExCmd.ExStep++;
	ExCmd.ExStart = 0;
	}
	
	switch (ExCmd.ExStep)
	{
	  case 1:
		flagCmd.climb = 1;
    flagCmd.pumpTurn = 2;
		ExCmd.TimeDelay++;
   	if((abs(Climbing[0].pidOuter.error) < 800) && ExCmd.TimeDelay > 40)
		{	
			//������
			ExCmd.TimeDelay = 0;
			ExCmd.ExStep++;
    }
    break;
		
		case 2:
		ExCmd.TimeDelay++;
		if(ExCmd.TimeDelay >= 150)
		{
		 ExCmd.TimeDelay = 0;
		 ExCmd.ExStep++;
		}
    break;
		case 3:
		out_turn[0].resultMax = 1320;
		out_turn[1].resultMax = 1320;
		flagCmd.climb = 0;
		flagCmd.pumpTurn = 0;
		ExCmd.TimeDelay++;
		if((abs(Climbing[0].pidOuter.error) < 800) && ExCmd.TimeDelay > 40)
		{	
			ExCmd.ExStep++;
			ExCmd.TimeDelay = 0;
    }
		break;
		
		case 4:
		out_turn[0].resultMax = Turning[0].getMotorSpeedLimit()/4;
		out_turn[1].resultMax = Turning[1].getMotorSpeedLimit()/4;
		      switch(ExCmd.OrePos)
					{
						case 1: //���볯��
							switch(ExCmd.OreStep)
							{
		  					case 0:
									flagCmd.pumpTurn = 0;
								  ExCmd.OreStep++;
								break;
								
								case 1:
									ExCmd.OreStep = 0;								
									ExCmd.ExStep = 0;
								break;
							}
						break;
						
						case 2: //���볯ǰ
							switch(ExCmd.OreStep)
							{
		  					case 0:
									flagCmd.pumpTurn = 1;
								  ExCmd.TimeDelay++;
						      if((abs(Turning[0].pidOuter.error) < 800) && ExCmd.TimeDelay > 30 )
	         	       {	
                     ExCmd.OreStep++;
										 ExCmd.TimeDelay = 0;
                   }
								break;
									 
								case 1:
									//flagCmd.pumpTurn = 0;
								  ExCmd.OreStep++;
								break;
								
								case 2:
									ExCmd.OreStep = 0;							
									ExCmd.ExStep = 0;
								break;
							}
            break;	
            
						case 3://���볯��
							switch(ExCmd.OreStep)
							{
		  					case 0:
									flagCmd.pumpTurn = 2;
								  ExCmd.TimeDelay++;
						      if((abs(Turning[0].pidOuter.error) < 800) && ExCmd.TimeDelay > 30)
	         	       {	
                     ExCmd.OreStep++;
										 ExCmd.TimeDelay = 0;
                   }
								break;
									 
								case 1:
									//flagCmd.pumpTurn = 0;
								  ExCmd.OreStep++;
								break;
								
								case 2:
									ExCmd.OreStep = 0;							
									ExCmd.ExStep = 0;
								break;
							}
            break;   							
					}
		break;
					
		default:
		break;
					
	}	
   
}
	

void MoneyTaking_Task(void *pvParameters)
{

	
	while(1)
	{
		SuctionRest();
		Exchange();
		vTaskDelay(pdMS_TO_TICKS(5));
	}

}