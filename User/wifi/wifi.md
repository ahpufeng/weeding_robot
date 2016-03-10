文档创建时间：2016年3月10日
本文档描述的是如何在`stm32f10x`上用串口转wifi模块`ESP8266`实现与上位机的通信。
# 模块介绍
ESP8266模块，是一款USART转WiFi模块，可以使嵌入式系统串口通过无线网络与上位机进行通信。

## 引脚
这个模块的引脚一般引出来的有6个:
- VCC：电源（建议接3.3v，在5v下也可以工作，据测试发热量稍大）
- GND：电源地
- TXD：模块串口发送引脚（TTL电平，可接单片机的RXD）
- RXD：模块串口接收引脚（TTL电平，可接单片机的TXD）
- RST：复位（低电平有效）
- IO-0：低电平是进入固件烧写模式，高电平为运行状态（默认状态）

## 工作模式
这个模块通常会有三种工作模式：
- AP模式，这种模式下模块相当于一个WiFi热点，其他设备可以接入这个热点，从而实现无线通信。（和蓝牙类似，不需要网络也可以实现）
- STA模式，这个模式下模块相当于一个WiFi设备，需要通过路由器与其他的网络设备通信。
- AP+SAT模式，这种模式是上述两种模式的并集。

每种模式下，都有若干种“身份”。本文档，主要讨论的是模块在STA模式下充当TCP Client的使用方法。

## 指令介绍
模块提供了一套完整的AT指令，供我们来操作，详细的指令请参考文档，这里不逐一列出。
> [ESP8266用户手册 ]()

## 注意事项
1. 波特率115200
2. 输入以回车换行符‘\r\n'结尾
3. 使用双引号表示字符串数据

# 具体实现
实现WiFi的功能，分成硬件和软件部分。硬件上，使用stm32的USART2与ESP8266相连接。软件上，根据手册，封装AT命令，使用中断读取接收数据等。

引脚连接方式：
- ESP8266_RXD连接stm32_USART2_TXD
- ESP8266_TXD连接stm32_USART2_RXD
- ESP8266_VCC连接3.3v
- ESP8266_GND连接GND
- ESP8266_RST悬空
- ESP8266_I0_0悬空

PS：stm32与ESP8266要共地。

软件上，主要由下面几个文件来实现WiFi模块的驱动：
- wifi_config.c 这个文件主要完成USART2的初始化，USART2的中断控制器配置
- wifi_function.c 这个文件主要实现对wifi模块的AT指令操作，并完成STA TCP Client连接

这里不给出详细的代码，具体的代码可以看[这里](https://github.com/LAIHAOTAO/weeding_robot/tree/master/User/wifi)，这里重点讨论`wifi_function.c`这个文件，下面列出它的函数作解释，具体实现自己看看源文件：

```
//使能wifi模块
void ESP8266_Choose ( FunctionalState enumChoose );
//重启模块
void ESP8266_Rst( void );
//执行AT测试
void ESP8266_AT_Test( void );
//发送AT指令
//cmd：AT指令内容， reply1：期望的回应， reply2：期望的回应，waittime：等待时间
bool ESP8266_Cmd( char * cmd, char * reply1, char * reply2, u32 waittime );
//选择wifi模块的模式 AP， STA， AP+STA
bool ESP8266_Net_Mode_Choose( ENUM_Net_ModeTypeDef enumMode );
//开启多客户端连接
bool ESP8266_Enable_MultipleId( FunctionalState enumEnUnvarnishTx );
//连接到服务器（STA模式下需要使用到）
bool ESP8266_Link_Server( ENUM_NetPro_TypeDef enumE, char * ip, char * ComNum, ENUM_ID_NO_TypeDef id);
//开启透传模式
bool ESP8266_UnvarnishSend( void );
//发送数据
bool ESP8266_SendString( FunctionalState enumEnUnvarnishTx, char * pStr, u32 ulStrLength, ENUM_ID_NO_TypeDef ucId );
//接收数据
char *ESP8266_ReceiveString( FunctionalState enumEnUnvarnishTx );
//配置STA_TCP_Client连接模式
bool  ESP8266_STA_TCP_Client( void );
//从透传模式返回普通AT模式
bool ESP8266_Return_At( void );
//失能透传模式
bool ESP8266_Disable_UnvarnishSend( void );
//关闭TCP连接
bool ESP8266_Tcp_Close( void );
//按照配置，开启TCP连接
bool ESP8266_Connect_Tcp( void );
```
具体，是怎么往模块里面发送AT指令的，这里以其中一个为例子进行讲述：

```
//加入某个wifi热点的函数
//对应的AT指令是：AT+CWJAP=“ssid”,“passwor”
bool ESP8266_JoinAP ( char * pSSID, char * pPassWord ){
    //声明一个存放命令的区域
	char cCmd [120];
   //通过sprintf函数，将参数传进来的ssid和password拼接到字符串后面，放入之前声明的区域
	sprintf ( cCmd, "AT+CWJAP=\"%s\",\"%s\"", pSSID, pPassWord );
   //通过ESP8266_Cmd ()函数，往模块里面写入命令
	return ESP8266_Cmd ( cCmd, "OK", NULL, 7000 );	
}
```
下面，分析下如何配置TCP连接，并使能它的：

```
//配置TCP连接
//LCD是为了便于调试添加的，在液晶屏上显示程序的运行信息
bool ESP8266_STA_TCP_Client ( void ){
	bool statusCur = false;
	bool statusLst = true;
	// 开启sta连接模式
	LCD_DispStr(0,0,(uint8_t *)"begin to sta connection ...",BLUE);	
	ESP8266_AT_Test ();
	statusCur = ESP8266_Net_Mode_Choose ( STA );
	if (statusCur == true){
		LCD_DispStr(0,20,(uint8_t *)"sta mode success",0xffff);
		statusLst = true;
	}else{
		LCD_DispStr(0,20,(uint8_t *)"sta mode fail",0xffff);
		statusLst = false;
	}	
	//加入热点的SSID和PASSWORD
	LCD_DispStr(0,40,(uint8_t *)"begin to join ERIC_LAI ...",BLUE);
    //这里写死了热点的ssid和pw，需要根据各自的情况自行修改
	statusCur = ESP8266_JoinAP ( "ERIC_LAI", "lai686868" );
	if (statusCur == true && statusLst == true){
		LCD_DispStr(0,60,(uint8_t *)"join ERIC_LAI success",0xffff);	
		statusLst = true;
	} else{
		LCD_DispStr(0,60,(uint8_t *)"join ERIC_LAI success",0xffff);
		statusLst = false;
	}
}
```
```
//使能TCP连接
bool ESP8266_Connect_Tcp(void){
	bool status = false;
	// 连接到TCP服务器
	ESP8266_Enable_MultipleId ( ENABLE );
	LCD_DispStr(0,120,(uint8_t *)"begin to connect the server ...",BLUE);
	status = ESP8266_Link_Server ( enumTCP, serverIpAddress, "9998", Multiple_ID_0 );
	if (status == true){
		LCD_DispStr(0,140,(uint8_t *)"connect server success",0xffff);
	} else{
		LCD_DispStr(0,140,(uint8_t *)"connect server fail",0xffff);
	}
	return status;
}
```
这一步成功之后，就是接受和发送数据的事情了。因为是串口，所以本质上用的还是串口中断来接收数据的。在wifi_config.c里面，我们完成了串口的使用和中断的配置。但是，串口只能一位一位的发送数据，所以，我们需要写一些代码来实现字符串的接收。上面的注意事项，’\r\n‘是发送完成的标志。所以，我们一直接收数据，知道接收到回车换行符为止。接收到后，标志位置位，退出接收循环。详细代码如下：

```
//以下的代码在stm32f10x_it.c里面
void USART2_IRQHandler( void ){	
	char ch;
	if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
		ch  = USART_ReceiveData( USART2 );
		if( strEsp8266_Fram_Record .InfBit .FramLength < ( RX_BUF_MAX_LEN - 1 ) ){ //预留1个字节写结束符				
        strEsp8266_Fram_Record .Data_RX_BUF [ strEsp8266_Fram_Record .InfBit .FramLength ++ ]  = ch;
		}
	} 
	if ( USART_GetITStatus( USART2, USART_IT_IDLE ) == SET ) {//数据帧接收完毕
        strEsp8266_Fram_Record .InfBit .FramFinishFlag = 1;	
		ch = USART_ReceiveData( USART2 ); //由软件序列清除中断标志位(先读USART_SR，然后读USART_DR)	
  }	
}
```

以上，完成之后，通过上面给出的`ESP8266_ReceiveString()`函数就可以获取接收到的字符串，注意：这是一个堵塞的函数！一旦调用就会等待接收到回车换行符为止。

上面的代码，在发送数据和接收数据的情况，都没有使用透传模式。所以，发送数据的时候需要确定数据的长度（最长不超过1024），接收数据的时候会有报头（+IDP,\*,\*:）。所以，发送图片的时候需要将图片分割开来发送，接收的时候需要做数据截断，把报头去掉。关于图片分割，在图片传输的时候讲述，截断数据的方法在下面给出：
```
char* getRealRecv(char *pStr){
	char *pBuf, *ppStr, *pStrDelimiter[2];
	int uc = 0;
	char cStrInput [100];
	sprintf(cStrInput, "%s", pStr);
    pBuf = cStrInput;
		uc = 0;
		while ( ( ppStr = strtok ( pBuf, ":" ) ) != NULL ){
			pStrDelimiter [ uc ++ ] = ppStr;
			pBuf = NULL;
		} 
	return pStrDelimiter[1];
}
```

以上，是WiFi模块的完成过程。