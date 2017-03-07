#ifndef __BT05_H
#define __BT05_H
#include "stm32f10x.h"
#include <stdbool.h>

extern volatile u8 Usart2_Printf_Flag;

#define BT05_STATE_PIN GPIO_Pin_4
#define BT05_STATE_PORT GPIOA
#define BT05_STATE_CLOCK RCC_APB2Periph_GPIOA

#define BT05_Usart( buf, buf_len ) comSendBuf(COM2, buf, buf_len)

typedef enum{
	NO_PSW,
	SIMPLE_PAIR,
	PSW_PAIR,
	PSW_PAIR_BIND
} ENUM_WorkTypeDef;

typedef enum{
	PERIPHERAL,
	MASTER
} ENUM_WorkModeDef;

//bool BT05_Cmd ( char * cmd, char * reply1, char * reply2, u32 waittime );
bool BT05_Password_Set( char * pPassWord );
bool BT05_Work_Type_Choose( ENUM_WorkTypeDef enumMode );
void BT05_AT_Test ( void );
void BT05_Rst ( void );
void BT05_Set_Default( void );
bool BT05_Start_LowPower( u8 Mode );
void BT05_Set_LowPower( void );
bool BT05_Work_Mode_Choose( ENUM_WorkModeDef WorkMode );
void BT05_Search_Device( void );
void BT05_Connect_Remote_Device( u8 DeviceNum );
void BT05_Set_Power( u8 Level );
void BT05_Get_Help( void );
void BT05_State_Line_Init( void );
void bsp_InitBT05( void );

#endif
