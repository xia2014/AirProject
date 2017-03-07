/**
  ******************************************************************************
  * @file    bsp_xxx.c
  * @author  Chowie
  * @version V1.0
  * @date    2016--3--25
  * @brief   �̳���⴫����
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
 * ��������GYP_GPIO_Config
 * ����  ������GYP�õ���I/O��  PB1Ϊ���ݹܽ�
 * ����  ����
 * ���  ����
 */
static void GYP_GPIO_Config(void)
{
	/*����һ��GPIO_InitTypeDef���͵Ľṹ��*/
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/*����GYP������ʱ��*/
	RCC_APB2PeriphClockCmd(GYP_CLK, ENABLE); 
	
	/*ѡ��Ҫ���Ƶ�GYP����*/															   
	GPIO_InitStructure.GPIO_Pin = GYP_PIN;
	
	/*��������ģʽΪͨ���������*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   

	/*������������Ϊ50MHz */   
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	/*���ÿ⺯������ʼ��GPIOB*/
	GPIO_Init(GYP_PORT, &GPIO_InitStructure);		  

	/* ����GPIOB1 */
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
	//����PB1
	GPIO_SetBits(GYP_PORT, GYP_PIN);	
	//��ʱ 0.32ms
	//DelayUS(320);
	DelayUS(280);
	if(GPIO_ReadOutputDataBit(GYP_PORT,GYP_PIN) == HIGH)
	{
		
		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // ��ȡת����ADֵ
	}
	DelayUS(40);
	//����PB1
	GPIO_ResetBits(GYP_PORT, GYP_PIN);	
	//��ʱ 9.68ms
	DelayUS(9680);
}

int Read_GYP( Air_Quality_TypeDef *Air_Struct )
{
	char Dust_Density_Str[20];
	/*����LED����*/
	GYP_LED_Control();		
	//printf("\r\n The current AD value = %f V \r\n",ADC_ConvertedValueLocal); 
	//DelayUS(280);
//	if(GPIO_ReadOutputDataBit(GYP_PORT,GYP_PIN) == HIGH)
//	{
//		
//		ADC_ConvertedValueLocal =(float) ADC_ConvertedValue/4096*3.3; // ��ȡת����ADֵ
//	}
	Air_Struct->absolute_voltage = ADC_ConvertedValueLocal;
	
	//���±�׼��ѹֵ
	if( Air_Struct->absolute_voltage < Air_Struct->typical_voltage) 
	{
		Air_Struct->typical_voltage = Air_Struct->absolute_voltage;
	}
	
	Air_Struct->relative_voltage = Air_Struct->absolute_voltage - Air_Struct->typical_voltage;  //��Ե�ѹ
	Air_Struct->Dust_Density = Air_Struct->relative_voltage / COEFFICIENT / 10;  //�۳�Ũ��
	
	Air_Struct->Dust_Density_Disp = Air_Struct->Dust_Density*1000;
	
	sprintf(Dust_Density_Str,"%f",Air_Struct->Dust_Density);
	printf("\r\n ��ǰ��⵽��Ũ��Ϊ %f ����/������ \r\n", Air_Struct->Dust_Density); 
	
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
	
	if( Air_Struct->smoke > CIGARETTE_JUDGE )    //�����ߵ�ƽ���ж�Ϊ��
	{
		//printf("\r\n  ��ǰ��⵽��Ϊ��,Ũ��Ϊ %f ����/������ \r\n", Air_Struct->Dust_Density);
		Air_Struct->smoke = 0;
		Air_Struct->quality = Air_Bad;
		//OLED_ShowChinese(90,6,14);   //��
	}
	else if( Air_Struct->no_smoke > NO_CIGARETTE_JUDGE)   //����Ϊ�͵�ƽ���ж�Ϊ�޻ҳ�
	{
		//printf("\r\n  ��ǰ���������Ϻã��ҳ�Ũ��Ϊ %f ����/������  \r\n ",Air_Struct->Dust_Density);
		Air_Struct->no_smoke = 0;
		Air_Struct->quality = Air_Good;
		//OLED_ShowChinese(90,6,15);   //��
	}
	else
	{
		Air_Struct->quality = Air_Medium;
		//printf("\r\n ��ǰ��⵽��Ϊ�۳���Ũ��Ϊ %f ����/������ \r\n", Air_Struct->Dust_Density); 
		//OLED_ShowChinese(90,6,12);   //��
	}
	return 1;
}
