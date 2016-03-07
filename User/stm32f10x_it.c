/**
  ******************************************************************************
  * @file    Project/STM32F10x_StdPeriph_Template/stm32f10x_it.c 
  * @author  MCD Application Team
  * @version V3.5.0
  * @date    08-April-2011
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and 
  *          peripherals interrupt service routine.
  ******************************************************************************
  * @attention
  *
  * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
  * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
  * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
  * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
  * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
  * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
  *
  * <h2><center>&copy; COPYRIGHT 2011 STMicroelectronics</center></h2>
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"
#include <stdio.h>
#include "bsp_sdio_sdcard.h"	
#include "bsp_ov7725.h"
#include "bsp_led.h"
#include "bsp_usart2.h"
#include "wifi_config.h"
#include "bsp_SysTick.h"

extern volatile uint8_t Ov7725_vsync;
extern volatile uint8_t screen_flag;
volatile char* rec_buf;

/** @addtogroup STM32F10x_StdPeriph_Template
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
	TimingDelay_Decrement();	
}

/******************************************************************************/
/*                 STM32F10x Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f10x_xx.s).                                            */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @}
  */ 

/**
  * @brief  This function handles USART2 Handler.
  * @param  None
  * @retval None
  */
void USART2_IRQHandler( void )
{	
	char ch;
	
	// ���������жϣ������ݴ���λ�����������
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)
	{
		ch  = USART_ReceiveData( USART2 );
		
		if( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) )                       //Ԥ��1���ֽ�д������
		{
				strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ch;

		}
	}
	 	 
	// ����������֮��Ĵ���
	if ( USART_GetITStatus( USART2, USART_IT_IDLE ) == SET )                                         //����֡�������
	{
    strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;
		
		ch = USART_ReceiveData( USART2 );                                                              //�������������жϱ�־λ(�ȶ�USART_SR��Ȼ���USART_DR)
	
  }	

}

/* USART1�����ж� 
void USART1_IRQHandler(void)
{
	uint8_t ch;
	
	if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET)
	{ 	
	    //ch = USART1->DR;
			ch = USART_ReceiveData(USART1);
	  	printf( "%c", ch );    //�����ܵ�������ֱ�ӷ��ش�ӡ
			if (ch == '1')
			{
					printf("set screen flag");
					screen_flag = 1;
			}
	} 
	 
}


/*
 * ��������SDIO_IRQHandler
 * ����  ����SDIO_ITConfig(���������������sdio�ж�	��
 *		     ���ݴ������ʱ�����ж�
 * ����  ����		 
 * ���  ����
 */
void SDIO_IRQHandler(void) 
{
  /* Process All SDIO Interrupt Sources */
  SD_ProcessIRQSrc();
}

/* ov7725 ���ж� ������� */
void EXTI0_IRQHandler(void)
{
    if ( EXTI_GetITStatus(EXTI_Line0) != RESET ) 	//���EXTI_Line0��·�ϵ��ж������Ƿ��͵���NVIC 
    {
        if( Ov7725_vsync == 0 )
        {
            FIFO_WRST_L(); 	                      //����ʹFIFOд(����from����ͷ)ָ�븴λ
            FIFO_WE_H();	                        //����ʹFIFOд����
            
            Ov7725_vsync = 1;	   	
            FIFO_WE_H();                          //ʹFIFOд����
            FIFO_WRST_H();                        //����ʹFIFOд(����from����ͷ)ָ���˶�
        }
        else if( Ov7725_vsync == 1 )
        {
            FIFO_WE_L();                          //����ʹFIFOд��ͣ
            Ov7725_vsync = 2;
        }        
        EXTI_ClearITPendingBit(EXTI_Line0);		    //���EXTI_Line0��·�����־λ        
    }    
}

/* IO ���жϣ��жϿ�ΪPC13 */
void EXTI15_10_IRQHandler(void)
{
	if(EXTI_GetITStatus(EXTI_Line13) != RESET) //ȷ���Ƿ������EXTI Line�ж�
	{
		/* LED1 ȡ�� ��ʾ�а������� */		
		LED1_TOGGLE;
		
		{/* ��ͼ��־��λ��ͬʱLED2�� */
			LED2_OFF;
			screen_flag = 1;
		}
		
		EXTI_ClearITPendingBit(EXTI_Line13);     //����жϱ�־λ
	}  
}
/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
