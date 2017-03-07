#ifndef __BSP_OLED_H 
#define __BSP_OLED_H

#include "stm32f10x.h"
#include "bsp_dht11.h"
#include "bsp_gyp.h"

extern unsigned char BMP1[1024];
extern unsigned char BMP2[1024];

#define OLED_CMD   	0
#define OLED_DATA  	1
#define Max_Column  128

#define CHAR_HEIGHT	16
#define CHAR_WIDTH	8
#define FONT_WIDTH 16

#define RCC_ALL_PIN 		RCC_APB2Periph_GPIOB

#define GPIO_PORT_SCLK    	GPIOB
#define GPIO_PIN_SCLK	  	GPIO_Pin_5

#define GPIO_PORT_SDIN    	GPIOB
#define GPIO_PIN_SDIN     	GPIO_Pin_6

#define GPIO_PORT_RS    	GPIOB
#define GPIO_PIN_RS     	GPIO_Pin_7

#define GPIO_PORT_DC    	GPIOB
#define GPIO_PIN_DC     	GPIO_Pin_8

#define GPIO_PORT_CS    	GPIOB
#define GPIO_PIN_CS     	GPIO_Pin_9

//-----------------OLED¶Ë¿Ú¶¨Òå----------------  

#define OLED_DC_Set() 	GPIO_SetBits( GPIO_PORT_DC, GPIO_PIN_DC )
#define OLED_DC_Clr() 	GPIO_ResetBits( GPIO_PORT_DC, GPIO_PIN_DC )

#define OLED_CS_Set() 	GPIO_SetBits( GPIO_PORT_CS, GPIO_PIN_CS )
#define OLED_CS_Clr() 	GPIO_ResetBits( GPIO_PORT_CS, GPIO_PIN_CS )

#define OLED_SCLK_Set() GPIO_SetBits( GPIO_PORT_SCLK, GPIO_PIN_SCLK )
#define OLED_SCLK_Clr() GPIO_ResetBits( GPIO_PORT_SCLK, GPIO_PIN_SCLK )

#define OLED_SDIN_Set() GPIO_SetBits( GPIO_PORT_SDIN, GPIO_PIN_SDIN )
#define OLED_SDIN_Clr() GPIO_ResetBits( GPIO_PORT_SDIN, GPIO_PIN_SDIN )

#define OLED_RST_Set() 	GPIO_SetBits( GPIO_PORT_RS, GPIO_PIN_RS )
#define OLED_RST_Clr() 	GPIO_ResetBits( GPIO_PORT_RS, GPIO_PIN_RS )

void bsp_InitOLED(void);
void OLED_Show_Info( DHT11_Data_TypeDef *DHT11_Data, Air_Quality_TypeDef *Air_Struct );
void OLED_Show_CH2O( uint32_t* ch2o_m, Air_Quality_TypeDef *Air_Struct );
void OLED_Clear( void );

#endif
