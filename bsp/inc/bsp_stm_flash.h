#ifndef __BSP_STMFLASH_H__
#define __BSP_STMFLASH_H__
//#include "sys.h"
#include "bsp.h"

//用户根据自己的需要设置
#define STM32_FLASH_SIZE 256 	 		//所选STM32的FLASH容量大小(单位为K)
#define STM32_FLASH_WREN 1              //使能FLASH写入(0，不使能;1，使能)
//////////////////////////////////////////////////////////////////////////////////////////////////////

typedef union
{
	u32 u32_data;
	u8 arr_data[4];
}u32_DataTypedef;

typedef union 
{
	u16 u16_data;
	u8 arr_data[2];
}u16_DataTypedef;

typedef union 
{
	float float_data;
	u8 arr_data[4];
}float_DataTypedef;

typedef struct InfoStruct
{
	u8 InfoByte;
	u16_DataTypedef Year;
	u8 Month;
	u8 Day;
	u8 Hour;
	u8 humi_int;
	u8 humi_deci;
	u8 temp_int;
	u8 temp_deci;
	float_DataTypedef DustDensity;
	u32_DataTypedef CH2ODensity;
}InfoStructTypedef;

typedef struct
{
	u32_DataTypedef Write;
	u32_DataTypedef Read;
}RW_Ptr_Typedef;


//FLASH起始地址
#define STM32_FLASH_BASE 0x08000000 	//STM32 FLASH的起始地址

#define STM32_FLASH_WRITE_START 0x08020000

u16 STMFLASH_ReadHalfWord(u32 faddr);		  //读出半字  
void STMFLASH_WriteLenByte(u32 WriteAddr,u32 DataToWrite,u16 Len);	//指定地址开始写入指定长度的数据
u32 STMFLASH_ReadLenByte(u32 ReadAddr,u16 Len);						//指定地址开始读取指定长度数据
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite);		//从指定地址开始写入指定长度的数据
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead);   		//从指定地址开始读出指定长度的数据

void STMFlash_WriteStruct( InfoStructTypedef *InfoStruct, u32_DataTypedef *WriteAddr );
void STMFlash_ReadStruct( InfoStructTypedef *InfoStruct, u32_DataTypedef *ReadAddr );

void STMFlash_Get_WritePointer( RW_Ptr_Typedef *RW_Ptr_Struct );
void STMFlash_Get_ReadPointer( RW_Ptr_Typedef *RW_Ptr_Struct );

//测试写入
void Test_Write(u32 WriteAddr,u16 WriteData);								   
#endif

