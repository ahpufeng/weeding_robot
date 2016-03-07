/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   ����ov7725�����ʵ��
  ******************************************************************************
  * @attention
  *
  * ʵ��ƽ̨:Ұ�� iSO STM32 ������ 
  * ��̳    :http://www.chuxue123.com
  * �Ա�    :http://firestm32.taobao.com
  *
  ******************************************************************************
	*/
#include "stm32f10x.h"
#include "stdio.h"
#include "bsp_usart1.h"
#include "bsp_usart2.h"
#include "bsp_ili9341_lcd.h"
#include "bsp_sdfs_app.h"
#include "bsp_bmp.h"
#include "bsp_ov7725.h"
#include "bsp_exti.h"
#include "bsp_led.h"
#include "wifi_config.h"
#include "wifi_function.h"
#include "bsp_SysTick.h"
#include <string.h>

/*
 * ȫ���ж����ȼ��ĸߵͣ����� > (����) > sd�� > ����ͷ���ж�   
 */
 
#define IP_ADDRESS "172.21.30.1";

extern volatile uint8_t Ov7725_vsync ;
extern char serverIpAddress[20];
extern char *pIpAddress;
volatile uint8_t screen_flag;
char * pStr;

void LcdAndSD_Init()
{
		/* Һ����ʼ�� */
		LCD_Init();
		
		/* ����Һ��ɨ�跽��Ϊ ���½�->���Ͻ� */
		Lcd_GramScan( 2 );
		LCD_Clear(0, 0, 320, 240, BACKGROUND);

		/* ��ʼ��sd���ļ�ϵͳ����Ϊ���ֵ��ֿ��bmpͼƬ������sd������ */
		Sd_fs_init();
}

void LCDshowStatus()
{
		LCD_Init();
		Lcd_GramScan(1);
		LCD_Clear(0, 0, 240, 320, BACKGROUND);	
		LCD_DispStr(0,0,(uint8_t *)"finished screenshot",0xffff);
}

void showVideo(void)
{
    uint8_t file_name[20];
    uint8_t num = 0;
		uint8_t flag = 1;
/*--------------------------------------------------------------------------------------------------------*/		
	  /* ������ʼ�� */
		EXTI_PC13_Config();
	
		/* LED ��ʼ�� */
		LED_GPIO_Config();
		LED1_OFF;
		LED2_OFF;
/*--------------------------------------------------------------------------------------------------------*/		
		LcdAndSD_Init();
/*--------------------------------------------------------------------------------------------------------*/	
		/* ov7725 gpio ��ʼ�� */
		Ov7725_GPIO_Config();
	
		/* ov7725 �Ĵ������ó�ʼ�� */
		while(Ov7725_Init() != SUCCESS);
	
		/* ov7725 ���ź��߳�ʼ�� */
		VSYNC_Init();	
		Ov7725_vsync = 0;
/*-------------------------------------------------------------------------------------------------------*/	
		while(flag)
		{
				if( Ov7725_vsync == 2 )
						{
								FIFO_PREPARE;  			/*FIFO׼��*/					
								ImagDisp();					/*�ɼ�����ʾ*/
								Ov7725_vsync = 0;			
						}        
				/* screen_flag �ڰ����жϺ���������λ */
				if( screen_flag == 1 )
				{                    
						sprintf((char *)&file_name,"/camera%d",num++);           
										
						/* ����Һ��ɨ�跽��Ϊ ���½�->���Ͻ� */
						Lcd_GramScan( 3 );
						/* �����������浽SD���� */
						Screen_shot(0,0,320,240,file_name);
						
						/* ��ͼ���LED2�� */
						LED2_ON;
						screen_flag = 0;
						flag = 0;
						VSYNC_NVIC_Disable();
						LCDshowStatus();
				}	
		}
}

void WifiPrepare()
{
	pIpAddress = IP_ADDRESS;
	strcpy ( serverIpAddress, ( const char * ) pIpAddress );
	WiFi_Config();
	SysTick_Init();
}

int main()
{

	USART1_Config();
//	showVideo();
	WifiPrepare();
	ESP8266_STA_TCP_Client();
//	pStr = ESP8266_ReceiveString ( ENABLE );
//	PC_Usart("%s", pStr);
//	LCD_Clear(0, 0, 240, 320, BACKGROUND);	
//	LCD_DispStr(0, 220, (uint8_t *) pStr, 0xffff);

}
/* ---------------------------------------------end of file----------------------------------------------*/


	  

