/**
  ******************************************************************************
  * @file    bsp_xxx.c
  * @author  Chowie
  * @version V1.0
  * @date    2016--3--25
  * @brief   烟尘检测传感器
  ******************************************************************************/
		
//#include "bsp_gyp.h"
#include "bsp.h"

Air_Quality_TypeDef g_IndoorAir;
float ADC_ConvertedValueLocal = 0;
static void GYP_GPIO_Config(void);
static void GYP_LED_Control(void);
static void Air_Struct_Config( Air_Quality_TypeDef *Air_Struct );


#define ADC1_DR_Address    ((u32)0x40012400+0x4c)

__IO uint16_t ADC_ConvertedValue;

/*
 * 函数名：GYP_GPIO_Config
 * 描述  ：配置GYP用到的I/O口  PB1为数据管脚
 * 输入  ：无
 * 输出  ：无
 */
static void GYP_GPIO_Config(void)
{
	/*定义一个GPIO_InitTypeDef类型的结构体*/
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*开启GYP的外设时钟*/
	RCC_APB2PeriphClockCmd(GYP_CLK, ENABLE); 
	
	/*选择要控制的GYP引脚*/															   
	GPIO_InitStructure.GPIO_Pin = GYP_PIN;
	
	/*设置引脚模式为通用推挽输出*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*设置引脚速率为50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*调用库函数，初始化GPIOB*/
	GPIO_Init(GYP_PORT, &GPIO_InitStructure);		  

	/* 拉低GPIOB1 */
	GPIO_ResetBits(GYP_PORT, GYP_PIN);
}

static void Air_Struct_Config( Air_Quality_TypeDef *Air_Struct )
{
	Air_Struct->Dust_Density = 0;
	Air_Struct->absolute_voltage = 0;
	Air_Struct->relative_voltage = 0;
	Air_Struct->typical_voltage = 0.5;
	Air_Struct->smoke = 0;
	Air_Struct->no_smoke = 0;
	Air_Struct->quality = 0;
	Air_Struct->Dust_Density_Disp = 0;
}

void bsp_InitGYP( void )
{
	GYP_GPIO_Config();
	Air_Struct_Config( &g_IndoorAir );
	ADC1_Init();
}

static void GYP_LED_Control(void)
{
	//拉高PB1
	GPIO_SetBits(GYP_PORT, GYP_PIN);	
	//延时 0.32ms
	//DelayUS(320);
	DelayUS(280);
	if(GPIO_ReadOutputDataBit(GYP_PORT,GYP_PIN) == HIGH)
	{
		
		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值
	}
	DelayUS(40);
	//拉低PB1
	GPIO_ResetBits(GYP_PORT, GYP_PIN);	
	//延时 9.68ms
	DelayUS(9680);
}

int Read_GYP( Air_Quality_TypeDef *Air_Struct )
{
	char Dust_Density_Str[20];
	/*控制LED亮灭*/
	GYP_LED_Control();		
	//printf("\r\n The current AD value = %f V \r\n",ADC_ConvertedValueLocal); 
	//DelayUS(280);
//	if(GPIO_ReadOutputDataBit(GYP_PORT,GYP_PIN) == HIGH)
//	{
//		
//		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // 读取转换的AD值
//	}
	Air_Struct->absolute_voltage = ADC_ConvertedValueLocal;
	
	//更新标准电压值
	if( Air_Struct->absolute_voltage < Air_Struct->typical_voltage) 
	{
		Air_Struct->typical_voltage = Air_Struct->absolute_voltage;
	}
	
	Air_Struct->relative_voltage = Air_Struct->absolute_voltage - Air_Struct->typical_voltage;  //相对电压
	Air_Struct->Dust_Density = Air_Struct->relative_voltage / COEFFICIENT / 10;  //粉尘浓度
	
	Air_Struct->Dust_Density_Disp = Air_Struct->Dust_Density*1000;
	
	sprintf(Dust_Density_Str,"%f",Air_Struct->Dust_Density);
	printf("\r\n 当前检测到的浓度为 %f 毫克/立方米 \r\n", Air_Struct->Dust_Density); 
	
	if ( Air_Struct->relative_voltage > DANGEROUS_JUDGE )
	{
		Air_Struct->smoke++;
		Air_Struct->no_smoke = 0;
	}
	else 
	{
		Air_Struct->no_smoke++;
		Air_Struct->smoke = 0;
	}
	
	if( Air_Struct->smoke > CIGARETTE_JUDGE )    //持续高电平，判定为烟
	{
		//printf("\r\n  当前检测到的为烟,浓度为 %f 毫克/立方米 \r\n", Air_Struct->Dust_Density);
		Air_Struct->smoke = 0;
		Air_Struct->quality = Air_Bad;
		//OLED_ShowChinese(90,6,14);   //差
	}
	else if( Air_Struct->no_smoke > NO_CIGARETTE_JUDGE)   //持续为低电平，判定为无灰尘
	{
		//printf("\r\n  当前空气质量较好，灰尘浓度为 %f 毫克/立方米  \r\n ",Air_Struct->Dust_Density);
		Air_Struct->no_smoke = 0;
		Air_Struct->quality = Air_Good;
		//OLED_ShowChinese(90,6,15);   //优
	}
	else
	{
		Air_Struct->quality = Air_Medium;
		//printf("\r\n 当前检测到的为粉尘，浓度为 %f 毫克/立方米 \r\n", Air_Struct->Dust_Density); 
		//OLED_ShowChinese(90,6,12);   //良
	}
	return 1;
}
