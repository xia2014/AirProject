#ifndef __BSP_GYP_H
#define __BSP_GYP_H

#include "stm32f10x.h"

#define		GYP_CLK 			RCC_APB2Periph_GPIOB
#define 	GYP_PIN 			GPIO_Pin_1
#define 	GYP_PORT 			GPIOB
#define 	DANGEROUS_JUDGE 	2   //相对电压比较值  待定
#define 	CIGARETTE_JUDGE 	7    //高电平持续时间  待定
#define 	NO_CIGARETTE_JUDGE 	7  //低电平持续时间  待定
#define 	COEFFICIENT 		0.5

#define		Air_Good			2
#define		Air_Medium			1
#define		Air_Bad				0

typedef struct
{
	float Dust_Density;
	float absolute_voltage;
	float relative_voltage;
	float typical_voltage;
	uint8_t smoke;
	uint8_t no_smoke;
	uint8_t quality;
	uint32_t Dust_Density_Disp;
}Air_Quality_TypeDef;

extern float ADC_ConvertedValueLocal;
extern __IO uint16_t ADC_ConvertedValue;

extern Air_Quality_TypeDef g_IndoorAir;

void bsp_InitGYP( void );
int Read_GYP( Air_Quality_TypeDef *Air_Struct );

#endif
