/*
*********************************************************************************************************

��Ŀ���ƣ�����STM32��Free RTOS�Ŀ��������Ƕ��ʽϵͳ
���ߣ�Alan Xia
���ڣ�2016��12��6�մ���
�޸ļ�¼��
1��12��7��
��1���޸�bsp_gyp.c����cigarette��ȫ�ֱ����ŵ�һ���ṹ����ߣ�����smoke���cigarette
��2���޸�bsp_oled.c��bsp_oled.h���ļ����ú궨�����ԭ���Ķ˿ں͹ܽţ������Ժ����
��3����TIM2���SysTick����ʱ
��4���ֱ�Ϊ�ĸ���Χ�豸��������

2��12��28��
��1����������ϵͳ֮ǰ�������ж϶��ǹرյģ�������ǲ���ʹ���ж���ʱ���������û��ָ��ʵ�������ʱ
��2������һ��ϵͳ���������sysMngTask�����ڸ�������ߴ�������������
	 �����������߼����£�
	 vTaskBT05���ȼ�Ϊ3���ȴ����ڿ������жϱ����������жϷ����ӳ����ö�ֵ�ź�����������������
	 vTaskMsgPro���ȼ�Ϊ3���ȴ�OLED�޸��¼���־���������������������ȡ��������Ϣ
	 vTaskOLED���ȼ�Ϊ1����Ȼ���ȼ���ͣ����������ϱ��������񶼱�����������������ʱ���
��3����ֲ�˰������Ĵ������̣��Ѵ���1��2����Ϊ�������жϼ�DMA���صؽ�����2�Ĳ���������Ϊ9600����Ӧ����ģ��
��4���鿴ʱ��ͼ�޸ĳ��򣬽����STM32��ADͨ���޷���ȡ�۳���������ģ����������
*********************************************************************************************************
*/

/*
Ӳ������
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


Ӳ�����ӣ�12��19���Ժ�
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

�ܽŸĶ���12��29�գ�
	DHT11	DHT11_DATA->PB12	VCC->5V
	BT05	VCC->5V
	GYP		LED_VCC->5V
*/

#include "includes.h"

#define BIT_0	(1 << 0)
#define BIT_1	(1 << 1)
#define BIT_2	(1 << 2)

#define		SYSMNG_TASK_STACK_SIZE			(configMINIMAL_STACK_SIZE*4)	//ϵͳ��������ջ�ռ��С
#define		SYSMNG_TASK_PRIORITIE			(configMAX_PRIORITIES - 1)	//ϵͳ�����������������ȼ�
/*
**********************************************************************************************************
											��������
**********************************************************************************************************
*/

static void vTaskMsgPro(void *pvParameters);
static void vTaskBT05(void *pvParameters);
static void vTaskOLED(void *pvParameters);
static void sysMngTask(void *pvParameters);
static void AppObjCreate (void);

/*
**********************************************************************************************************
											��������
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
*	�� �� ��: main
*	����˵��: ��׼c������ڡ�
*	��    �Σ���
*	�� �� ֵ: ��
*********************************************************************************************************
*/
int main(void)
{
	/* 
	  ����������ǰ��Ϊ�˷�ֹ��ʼ��STM32����ʱ���жϷ������ִ�У������ֹȫ���ж�(����NMI��HardFault)��
	  �������ĺô��ǣ�
	  1. ��ִֹ�е��жϷ����������FreeRTOS��API������
	  2. ��֤ϵͳ�������������ܱ���ж�Ӱ�졣
	  3. �����Ƿ�ر�ȫ���жϣ���Ҹ����Լ���ʵ��������ü��ɡ�
	  ����ֲ�ļ�port.c�еĺ���prvStartFirstTask�л����¿���ȫ���жϡ�ͨ��ָ��cpsie i������__set_PRIMASK(1)
	  ��cpsie i�ǵ�Ч�ġ�
     */
	//__set_PRIMASK(1);  
	
	/* Ӳ����ʼ�� */
	bsp_Init(); 

	printf("This is a system test\r\n");
	
	//vSetupSysInfoTest();
	AppObjCreate();
	/* �������� */
	//AppTaskCreate();
	xTaskCreate(sysMngTask, "vSysMngTask", SYSMNG_TASK_STACK_SIZE, NULL, SYSMNG_TASK_PRIORITIE , &xHandlesysMngTask);
    /* �������ȣ���ʼִ������ */
    vTaskStartScheduler();
	/* 
	  ���ϵͳ���������ǲ������е�����ģ����е����Ｋ�п��������ڶ�ʱ��������߿��������
	  heap�ռ䲻����ɴ���ʧ�ܣ���Ҫ�Ӵ�FreeRTOSConfig.h�ļ��ж����heap��С��
	  #define configTOTAL_HEAP_SIZE	      ( ( size_t ) ( 17 * 1024 ) )
	*/
	while(1);
}


static void sysMngTask(void *pvParameters)
{
	BaseType_t xReturned;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(4000); /* �������ȴ�ʱ��Ϊ2000ms */
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
//		RTC_Get();//����ʱ��
//		printf("calendar.sec = %d\r\n",calendar.sec);
		vTaskDelay(xMaxBlockTime);
	}
}

static void vTaskMsgPro(void *pvParameters)
{
	BaseType_t xResult;
	uint32_t ulValue;
	const TickType_t xMaxBlockTime = pdMS_TO_TICKS(2000); /* �������ȴ�ʱ��Ϊ1000ms */
	const TickType_t xTaskDelayTime = pdMS_TO_TICKS(300); /* ����ϵͳ��ʱʱ��Ϊ300ms */
	while(1)
	{
		xResult = xTaskNotifyWait(0x00000000,      
						  0xFFFFFFFF,      
						  &ulValue,        /* ����ulNotifiedValue������ulValue�� */
						  xMaxBlockTime);  /* ��������ӳ�ʱ�� */
		
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
		xTaskNotify(xHandleTaskMsgPro, /* Ŀ������ */
			BIT_0 | BIT_1 | BIT_2,             /* ����Ŀ�������¼���־λbit0  */
			eSetBits);         /* ��Ŀ��������¼���־λ��BIT_0���л������ 
								  �������ֵ���¼���־λ��*/
	}
}

static void AppObjCreate (void)
{
	/* ������ֵ�ź������״δ����ź�������ֵ��0 */
	xSemaphore = xSemaphoreCreateBinary();
	
	if(xSemaphore == NULL)
    {
        /* û�д����ɹ����û�������������봴��ʧ�ܵĴ������ */
    }
}

void USART_CallBack(void)
{
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
		
	/* ����ͬ���ź� */
	xSemaphoreGiveFromISR(xSemaphore, &xHigherPriorityTaskWoken);

	/* ���xHigherPriorityTaskWoken = pdTRUE����ô�˳��жϺ��е���ǰ������ȼ�����ִ�� */
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
	uint8_t  humi_int;		//ʪ�ȵ���������
	uint8_t  humi_deci;	 	//ʪ�ȵ�С������
	uint8_t  temp_int;	 	//�¶ȵ���������
	uint8_t  temp_deci;	 	//�¶ȵ�С������
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
//	SPI_Flash_Init();  		//SPI FLASH ��ʼ��
//	printf("%d\r\n",SPI_Flash_ReadID());

//	while(SPI_Flash_ReadID()!=W25Q64)
//	{
//		printf("25Q64 Check Failed!");
//	}
///*
//void SPI_Flash_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   //��ȡflash
//void SPI_Flash_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);//д��flash
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
/***************************** ���������� www.armfly.com (END OF FILE) *********************************/
