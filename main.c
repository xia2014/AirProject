/*
*********************************************************************************************************

项目名称：基于STM32与Free RTOS的空气检测仪嵌入式系统
作者：Alan Xia
日期：2016年12月6日创建
修改记录：
1、12月7日
（1）修改bsp_gyp.c，将cigarette等全局变量放到一个结构体里边，并用smoke替代cigarette
（2）修改bsp_oled.c和bsp_oled.h等文件，用宏定义替代原来的端口和管脚，方便以后更改
（3）用TIM2替代SysTick做延时
（4）分别为四个外围设备创建任务

2、12月28日
（1）开启操作系统之前，所有中断都是关闭的，因此我们不能使用中断延时，这里我用汇编指令实现软件延时
（2）创建一个系统管理的任务sysMngTask，并在该任务里边创建三个子任务
	 三个子任务逻辑如下：
	 vTaskBT05优先级为3，等待串口空闲线中断被触发，从中断服务子程序获得二值信号量，解除自身的阻塞
	 vTaskMsgPro优先级为3，等待OLED修改事件标志，解除自身阻塞，进而读取传感器信息
	 vTaskOLED优先级为1，虽然优先级最低，但是由于上边两个任务都被阻塞，所以它运行时间最长
（3）移植了安富莱的串口例程，把串口1和2都改为空闲线中断加DMA，特地将串口2的波特率设置为9600以适应蓝牙模块
（4）查看时序图修改程序，解决了STM32的AD通道无法读取粉尘传感器的模拟量的问题
*********************************************************************************************************
*/

/*
硬件连接
GYP
	LED_VCC->3.3V	LED_GND->GND		LED->PB1
	S-GND->GND		Vo->PC0
DHT11
	VDD->3.3V		DHT11_DATA->PB10	GND->GND
BT05
	VCC->3.3V		GND->GND			RX->PA2(TX)		TX->PA3(RX)
OLED
	VCC->3.3V		GND->GND			CS->PD3			RST->PD4
	DC->PD5			SCLK->PD6			SDIN->PD7


硬件连接（12月19日以后）
GYP
	LED_VCC->3.3V	LED_GND->GND		LED->PB1
	S-GND->GND		Vo->PC0
DHT11
	VDD->3.3V		DHT11_DATA->PB10	GND->GND
BT05
	VCC->3.3V		GND->GND			RX->PA2(TX)		TX->PA3(RX)
OLED
	GND->GND		VCC->3.3V			SCLK(D0)->PB5		SDIN(D1)->PB6
	RST(RES)->PB7	DC->PB8				CS->PB9
CH2O
	GND->GND		VCC->5V				RX->PB10(TX)	TX->PB11(RX)

管脚改动（12月29日）
	DHT11	DHT11_DATA->PB12	VCC->5V
	BT05	VCC->5V
	GYP		LED_VCC->5V
*/

#include "includes.h"

#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_2	(1 << 2)

#define		SYSMNG_TASK_STACK_SIZE			(configMINIMAL_STACK_SIZE*4)	//系统管理任务栈空间大小
#define		SYSMNG_TASK_PRIORITIE			(configMAX_PRIORITIES - 1)	//系统管理任务具有最高优先级
/*
**********************************************************************************************************
											函数声明
**********************************************************************************************************
*/

static void vTaskMsgPro(void *pvParameters);
static void vTaskBT05(void *pvParameters);
static void vTaskOLED(void *pvParameters);
static void sysMngTask(void *pvParameters);
static void AppObjCreate (void);

/*
**********************************************************************************************************
											变量声明
**********************************************************************************************************
*/

static TaskHandle_t xHandleTaskMsgPro = NULL;
static TaskHandle_t xHandleBT05 = NULL;
static TaskHandle_t xHandleOLED = NULL;
static TaskHandle_t xHandlesysMngTask = NULL;
static SemaphoreHandle_t  xSemaphore = NULL;

static uint32_t OLED_Count = 0;
static uint8_t OLED_Flag = 0;

static void ClearStruct( InfoStructTypedef *InfoStruct );
static void FlashTest(void);

static void STM32_FlashTest(void);

#define INFO_FLAG 0
#define CH2O_FLAG 1

/*
*********************************************************************************************************
*	函 数 名: main
*	功能说明: 标准c程序入口。
*	形    参：无
*	返 回 值: 无
*********************************************************************************************************
*/
int main(void)
{
	/* 
	  在启动调度前，为了防止初始化STM32外设时有中断服务程序执行，这里禁止全局中断(除了NMI和HardFault)。
	  这样做的好处是：
	  1. 防止执行的中断服务程序中有FreeRTOS的API函数。
	  2. 保证系统正常启动，不受别的中断影响。
	  3. 关于是否关闭全局中断，大家根据自己的实际情况设置即可。
	  在移植文件port.c中的函数prvStartFirstTask中会重新开启全局中断。通过指令cpsie i开启，__set_PRIMASK(1)
	  和cpsie i是等效的。
     */
	//__set_PRIMASK(1);  
	
	/* 硬件初始化 */
	bsp_Init(); 

	printf("This is a system test\r\n");
	
	//vSetupSysInfoTest();
	AppObjCreate();
	/* 创建任务 */
	//AppTaskCreate();
	xTaskCreate(sysMngTask, "vSysMngTask", SYSMNG_TASK_STACK_SIZE, NULL, SYSMNG_TASK_PRIORITIE , &xHandlesysMngTask);
    /* 启动调度，开始执行任务 */
    vTaskStartScheduler();
	/* 
	  如果系统正常启动是不会运行到这里的，运行到这里极有可能是用于定时器任务或者空闲任务的
	  heap空间不足造成创建失败，此要加大FreeRTOSConfig.h文件中定义的heap大小：
	  #define configTOTAL_HEAP_SIZE	      ( ( size_t ) ( 17 * 1024 ) )
	*/
	while(1);
}


static void sysMngTask(void *pvParameters)
{
	BaseType_t xReturned;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(4000); /* 设置最大等待时间为2000ms */
	printf("sysmgtask begin\r\n");
//	xReturned = xTaskCreate( vTaskBT05, "vTaskBT05",512,NULL,3,&xHandleBT05 );
//#if 1
//	xReturned = xTaskCreate( vTaskMsgPro,"vTaskMsgPro",512,NULL,3, &xHandleTaskMsgPro );

//	xReturned = xTaskCreate( vTaskOLED,"vTaskOLED", 512,NULL,1,&xHandleOLED );

//#endif
//	bsp_InitRTC();
	// FlashTest();
	STM32_FlashTest();
	while(1)
	{
//		RTC_Get();//更新时间
//		printf("calendar.sec = %d\r\n",calendar.sec);
		vTaskDelay(xMaxBlockTime);
	}
}

static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	uint32_t ulValue;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(2000); /* 设置最大等待时间为1000ms */
	const TickType_t xTaskDelayTime = pdMS_TO_TICKS(300); /* 设置系统延时时间为300ms */
	while(1)
	{
		xResult = xTaskNotifyWait(0x00000000,      
						  0xFFFFFFFF,      
						  &ulValue,        /* 保存ulNotifiedValue到变量ulValue中 */
						  xMaxBlockTime);  /* 最大允许延迟时间 */
		
		if( xResult == pdPASS )
		{
			if((ulValue & BIT_2) != 0)
			{
				Read_CH2O( &g_CH2O_mg_m3 );
			}
			if((ulValue & BIT_1) != 0)
			{
				Read_DHT11( &g_DHT11_Data );
			}
			if((ulValue & BIT_0) != 0)
			{
				Read_GYP( &g_IndoorAir );
			}
			vTaskDelay(xTaskDelayTime);
		}
		else
		{
		}
	}
}
static void vTaskBT05(void *pvParameters)
{
	char buf[64];
	//char temp[] = "hello\r\n";
	BaseType_t xResult;
    while(1)
    {
		xResult = xSemaphoreTake(xSemaphore, portMAX_DELAY);
		
		if(comGetBuff(COM2,(uint8_t *)buf))
		{
			//comSendBuf(COM1, (uint8_t *)buf, strlen(buf));
			//comSendBuf(COM1, (uint8_t *)temp, strlen(temp));
			//printf("%s\r\n",buf);
			//if(strcmp(buf,"Air"))
			if(strcmp(buf,"#A") == 0)
			{
				char temp[20],tempDensity[20];
				sprintf(temp,"Quality=%d\r\n",g_IndoorAir.quality);
				sprintf(tempDensity,"Density=%f\r\n",g_IndoorAir.Dust_Density);
				comSendBuf(COM1, (uint8_t *)temp, strlen(temp));
				comSendBuf(COM2, (uint8_t *)temp, strlen(temp));
				comSendBuf(COM1, (uint8_t *)tempDensity, strlen(tempDensity));
				//printf("Quality=%d\r\n",g_IndoorAir.quality);
			}
			//else if(strcmp(buf,"Humi&Temp"))
			else if(strcmp(buf,"#H") == 0)
			{
				char temp1[20],temp2[20];
				sprintf(temp1,"Humi=%d.%d\r\n",g_DHT11_Data.humi_int, g_DHT11_Data.humi_deci);
				comSendBuf(COM1, (uint8_t *)temp1, strlen(temp1));
				comSendBuf(COM2, (uint8_t *)temp1, strlen(temp1));
				sprintf(temp2,"Temp=%d.%d\r\n",g_DHT11_Data.temp_int, g_DHT11_Data.temp_deci);
				comSendBuf(COM1, (uint8_t *)temp2, strlen(temp2));
				comSendBuf(COM2, (uint8_t *)temp2, strlen(temp2));
//				printf("Humi=%d.%d\r\n",g_DHT11_Data.humi_int, g_DHT11_Data.humi_deci);
//				printf("Temp=%d.%d\r\n",g_DHT11_Data.temp_int, g_DHT11_Data.temp_deci);
			}
			else if(strcmp(buf,"#O") == 0)
			{
//				if( NRF24L01_Check() )
//				{
//					NRF24L01_TX_Mode();
//					while(NRF24L01_TxPacket("O") == 0xFF);
//					NRF24L01_RX_Mode();
//					while(NRF24L01_RxPacket(g_Outdoor_data) == 0xFF);
//				}
			}
			else
			{
				printf("Wrong command!\r\n");
			}
		}
		
		//printf("BT05\r\n");
    }
}

static void vTaskOLED(void *pvParameters)
{
	while(1)
	{
		if( OLED_Count < 250 )
		{
			++OLED_Count;
		}else
		{
			OLED_Count = 0;
			OLED_Clear();
			OLED_Flag = ~OLED_Flag;
		}
		if( OLED_Flag == INFO_FLAG )
		{
			OLED_Show_Info( &g_DHT11_Data, &g_IndoorAir );
		}
		else
		{
			OLED_Show_CH2O( &g_CH2O_mg_m3, &g_IndoorAir );
		}
		//printf("OLED_Show_Info\r\n");
		xTaskNotify(xHandleTaskMsgPro, /* 目标任务 */
			BIT_0 | BIT_1 | BIT_2,             /* 设置目标任务事件标志位bit0  */
			eSetBits);         /* 将目标任务的事件标志位与BIT_0进行或操作， 
								  将结果赋值给事件标志位。*/
	}
}

static void AppObjCreate (void)
{
	/* 创建二值信号量，首次创建信号量计数值是0 */
	xSemaphore = xSemaphoreCreateBinary();
	
	if(xSemaphore == NULL)
    {
        /* 没有创建成功，用户可以在这里加入创建失败的处理机制 */
    }
}

void USART_CallBack(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
	/* 发送同步信号 */
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	/* 如果xHigherPriorityTaskWoken = pdTRUE，那么退出中断后切到当前最高优先级任务执行 */
	portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
}

/*
typedef struct InfoStruct
{
	u8 InfoByte;
	union
	{
		u16 u16_Year;
		u8 YearArr[2];
	}Year;
	u8 Month;
	u8 Day;
	u8 Hour;
	uint8_t  humi_int;		//湿度的整数部分
	uint8_t  humi_deci;	 	//湿度的小数部分
	uint8_t  temp_int;	 	//温度的整数部分
	uint8_t  temp_deci;	 	//温度的小数部分
	union
	{
		float DustDensity;
		u8 DensityArr[4];
	}Dust;
	union
	{
		u32 CH2ODensity;
		u8 DensityArr[4];
	}CH2O;
}InfoStructTypedef;
*/
static void ClearStruct( InfoStructTypedef *InfoStruct )
{
	InfoStruct->InfoByte = 0;
	InfoStruct->Year.u16_data = 0;
	InfoStruct->Month = 0;
	InfoStruct->Day = 0;
	InfoStruct->Hour = 0;
	InfoStruct->humi_int = 0;
	InfoStruct->humi_deci = 0;
	InfoStruct->temp_int = 0;
	InfoStruct->temp_deci = 0;
	InfoStruct->DustDensity.float_data = 0;
	InfoStruct->CH2ODensity.u32_data = 0;
}

//static void FlashTest(void)
//{	
//	//u8 datatemp[4];

//	InfoStructTypedef InfoStruct;

//	RW_Ptr_Typedef RW_Ptr_Struct;
//	u32_DataTypedef datatemp;
//	
//	InfoStruct.InfoByte = 15;
//	InfoStruct.Year.u16_data = 2017;
//	InfoStruct.Month = 2;
//	InfoStruct.Day = 24;
//	InfoStruct.Hour = 4;
//	InfoStruct.humi_int = 5;
//	InfoStruct.humi_deci = 6;
//	InfoStruct.temp_int = 7;
//	InfoStruct.temp_deci = 8;
//	InfoStruct.DustDensity.float_data = 3.14;
//	InfoStruct.CH2ODensity.u32_data = 1628;
//	
//	SPI_Flash_Init();  		//SPI FLASH 初始化
//	printf("%d\r\n",SPI_Flash_ReadID());

//	while(SPI_Flash_ReadID()!=W25Q64)
//	{
//		printf("25Q64 Check Failed!");
//	}
///*
//void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //读取flash
//void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//写入flash
//*/
///*	//write pointer is saved in address from 0 to 3
//	SPI_Flash_Read( datatemp.arr_data, 0, 4 );
//	//if write address has not been initialized
//	if( datatemp.u32_data%18 != 0 )
//	{
//		datatemp.u32_data = 18;
//		SPI_Flash_Write( datatemp.arr_data, 0, 4 );
//		RW_Ptr_Struct.Write.u32_data = datatemp.u32_data;
//	}else
//	{
//		RW_Ptr_Struct.Write.u32_data = datatemp.u32_data;
//	}*/
//	SPI_Flash_Get_WritePointer( &RW_Ptr_Struct );

//	SPI_Flash_WriteStruct( &InfoStruct, RW_Ptr_Struct.Write );

//	ClearStruct( &InfoStruct );
//	printf("Before Reading\r\n");
//	printf("InfoStruct.Dust.DustDensity = %f\r\n",InfoStruct.DustDensity.float_data);
//	printf("InfoStruct.CH2O.CH2ODensity = %d\r\n",InfoStruct.CH2ODensity.u32_data);

///*	//read pointer is saved in address from 4 to 7
//	SPI_Flash_Read( datatemp.arr_data, 4, 4 );
//	if( datatemp.u32_data%18 != 0 )
//	{
//		datatemp.u32_data = 18;
//		SPI_Flash_Write( datatemp.arr_data, 4, 4 );
//		RW_Ptr_Struct.Read.u32_data = datatemp.u32_data;
//	}else
//	{
//		RW_Ptr_Struct.Read.u32_data = datatemp.u32_data;
//	}*/

//	SPI_Flash_Get_ReadPointer( &RW_Ptr_Struct );

//	SPI_Flash_ReadStruct( &InfoStruct, RW_Ptr_Struct.Read );
//	printf("After Reading\r\n");
//	printf("InfoStruct.Dust.DustDensity = %f\r\n",InfoStruct.DustDensity.float_data);
//	printf("InfoStruct.CH2O.CH2ODensity = %d\r\n",InfoStruct.CH2ODensity.u32_data);
//}

static void STM32_FlashTest(void)
{
//	const u8 TEXT_Buffer[]={"STM32 FLASH TEST"};
//	#define SIZE sizeof(TEXT_Buffer)
//	u8 datatemp[SIZE];
//	STMFLASH_Write(STM32_FLASH_WRITE_START,(u16*)TEXT_Buffer,SIZE);
//	STMFLASH_Read(STM32_FLASH_WRITE_START,(u16*)datatemp,SIZE);
//	printf("The Data Readed Is:  %s\r\n", datatemp);

	InfoStructTypedef InfoStruct;

	RW_Ptr_Typedef RW_Ptr_Struct;
	//u32_DataTypedef datatemp;
	u32 i;
	
	InfoStruct.InfoByte = 1;
	InfoStruct.Year.u16_data = 2017;
	InfoStruct.Month = 2;
	InfoStruct.Day = 24;
	InfoStruct.Hour = 4;
	InfoStruct.humi_int = 5;
	InfoStruct.humi_deci = 6;
	InfoStruct.temp_int = 7;
	InfoStruct.temp_deci = 8;
	InfoStruct.DustDensity.float_data = 3.14;
	InfoStruct.CH2ODensity.u32_data = 1628;
	
	RW_Ptr_Struct.Write.u32_data = 0;
	RW_Ptr_Struct.Read.u32_data = 0;
	
	for( i = 0; i < 14; ++i )
	{
		printf("\r\n******Circle start******\r\n");
		STMFlash_Get_WritePointer( &RW_Ptr_Struct );
		STMFlash_WriteStruct( &InfoStruct, &(RW_Ptr_Struct.Write) );
		STMFlash_Get_ReadPointer( &RW_Ptr_Struct );
		STMFlash_ReadStruct( &InfoStruct, &(RW_Ptr_Struct.Read) );

		printf("InfoStruct.InfoByte = %d\r\n",InfoStruct.InfoByte);
		InfoStruct.InfoByte++;
		printf("\r\n************************\r\n");
	}
}
/***************************** 安富莱电子 www.armfly.com (END OF FILE) *********************************/
