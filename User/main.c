/**
  ******************************************************************************
  * @file    main.c
  * @author  fire
  * @version V1.0
  * @date    2013-xx-xx
  * @brief   火眼ov7725照相机实验
  ******************************************************************************
  * @attention
  *
  * 实验平台:野火 iSO STM32 开发板 
  * 论坛    :http://www.chuxue123.com
  * 淘宝    :http://firestm32.taobao.com
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
 * 全局中断优先级的高低：串口 > (按键) > sd卡 > 摄像头场中断   
 */
 
#define IP_ADDRESS "10.0.23.67";

extern volatile uint8_t Ov7725_vsync ;
extern char serverIpAddress[20];
extern char *pIpAddress;
volatile uint8_t screen_flag;


void LcdAndSD_Init()
{
		/* 液晶初始化 */
		LCD_Init();
		
		/* 设置液晶扫描方向为 左下角->右上角 */
		Lcd_GramScan( 2 );
		LCD_Clear(0, 0, 320, 240, BACKGROUND);

		/* 初始化sd卡文件系统，因为汉字的字库和bmp图片放在了sd卡里面 */
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
	  /* 按键初始化 */
		EXTI_PC13_Config();
	
		/* LED 初始化 */
		LED_GPIO_Config();
		LED1_OFF;
		LED2_OFF;
/*--------------------------------------------------------------------------------------------------------*/		
		LcdAndSD_Init();
/*--------------------------------------------------------------------------------------------------------*/	
		/* ov7725 gpio 初始化 */
		Ov7725_GPIO_Config();
	
		/* ov7725 寄存器配置初始化 */
		while(Ov7725_Init() != SUCCESS);
	
		/* ov7725 场信号线初始化 */
		VSYNC_Init();	
		Ov7725_vsync = 0;
/*-------------------------------------------------------------------------------------------------------*/	
		while(flag)
		{
				if( Ov7725_vsync == 2 )
						{
								FIFO_PREPARE;  			/*FIFO准备*/					
								ImagDisp();					/*采集并显示*/
								Ov7725_vsync = 0;			
						}        
				/* screen_flag 在按键中断函数里面置位 */
				if( screen_flag == 1 )
				{                    
						sprintf((char *)&file_name,"/camera%d",num++);           
										
						/* 设置液晶扫描方向为 右下角->左上角 */
						Lcd_GramScan( 3 );
						/* 截屏，并保存到SD卡上 */
						Screen_shot(0,0,320,240,file_name);
						
						/* 截图完毕LED2灭 */
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
	// 液晶初始化
	LCD_Init();
	// 设置液晶扫描方向为 左上角->右下角
	Lcd_GramScan( 1 );
	LCD_Clear(0, 0, 240, 320, BACKGROUND);
	
	pIpAddress = IP_ADDRESS;
	strcpy ( serverIpAddress, ( const char * ) pIpAddress );
	WiFi_Config();
	SysTick_Init();
}

char* getRealRecv(char *pStr)
{
	char *pBuf, *ppStr, *pStrDelimiter[2];
	int uc = 0;
	char cStrInput [100];
	sprintf(cStrInput, "%s", pStr);
    pBuf = cStrInput;
		uc = 0;
		while ( ( ppStr = strtok ( pBuf, ":" ) ) != NULL )
		{
			pStrDelimiter [ uc ++ ] = ppStr;
			pBuf = NULL;
		} 
	return pStrDelimiter[1];
}

int main()
{
	char * pStr;
	
//	showVideo();
	WifiPrepare();
	ESP8266_STA_TCP_Client();
	ESP8266_Connect_Tcp();
	// 发送测试数据
	ESP8266_SendString(DISABLE, "HELLO WORLD\r\n", strlen("HELLO WORLD\r\n"), Multiple_ID_0);
	while(1)
	{
		pStr = NULL;
		pStr = getRealRecv( ESP8266_ReceiveString ( DISABLE ) );
		PC_Usart("origin: %s\r\n", pStr);
		if(strcmp("hello1",pStr)==0)
		{
			PC_Usart("receive hello1\r\n");
		} else
		{
			PC_Usart("receive hello\r\n");
		}
	}

//	LCD_Clear(0, 0, 240, 320, BACKGROUND);	
//	LCD_DispStr(0, 220, (uint8_t *) pStr, 0xffff);

}
/* ---------------------------------------------end of file----------------------------------------------*/


	  

