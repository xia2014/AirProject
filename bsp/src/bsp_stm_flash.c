#include "bsp.h"

//读取指定地址的半字(16位数据)
//faddr:读地址(此地址必须为2的倍数!!)
//返回值:对应数据.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//如果使能了写   
//不检查的写入
//WriteAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//地址增加2.
	}  
} 
//从指定地址开始写入指定长度的数据
//WriteAddr:起始地址(此地址必须为2的倍数!!)
//pBuffer:数据指针
//NumToWrite:半字(16位)数(就是要写入的16位数据的个数.)
#if STM32_FLASH_SIZE<256
	#define STM_SECTOR_SIZE 1024 //字节
#else 
	#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//最多是2K字节
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
	u32 secpos;	   //扇区地址
	u16 secoff;	   //扇区内偏移地址(16位字计算)
	u16 secremain; //扇区内剩余地址(16位字计算)	   
 	u16 i;    
	u32 offaddr;   //去掉0X08000000后的地址
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return;//非法地址
	FLASH_Unlock();						//解锁
	offaddr=WriteAddr-STM32_FLASH_BASE;		//实际偏移地址.
	secpos=offaddr/STM_SECTOR_SIZE;			//扇区地址  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//在扇区内的偏移(2个字节为基本单位.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//扇区剩余空间大小   
	if(NumToWrite<=secremain)
		secremain=NumToWrite;//不大于该扇区范围
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//读出整个扇区的内容
		for(i=0;i<secremain;i++)//校验数据
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)
				break;//需要擦除  	  
		}
		if(i<secremain)//需要擦除
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//擦除这个扇区
			for(i=0;i<secremain;i++)//复制
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//写入整个扇区  
		}else
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//写已经擦除了的,直接写入扇区剩余区间. 				   
		if(NumToWrite==secremain)
			break;//写入结束了
		else//写入未结束
		{
			secpos++;				//扇区地址增1
			secoff=0;				//偏移位置为0 	 
		   	pBuffer+=secremain;  	//指针偏移
			WriteAddr+=secremain;	//写地址偏移	   
		   	NumToWrite-=secremain;	//字节(16位)数递减
			if(NumToWrite>(STM_SECTOR_SIZE/2))
				secremain=STM_SECTOR_SIZE/2;//下一个扇区还是写不完
			else
				secremain=NumToWrite;//下一个扇区可以写完了
		}	 
	};	
	FLASH_Lock();//上锁
}
#endif

//从指定地址开始读出指定长度的数据
//ReadAddr:起始地址
//pBuffer:数据指针
//NumToWrite:半字(16位)数
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//读取2个字节.
		ReadAddr+=2;//偏移2个字节.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:起始地址
//WriteData:要写入的数据
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//写入一个字 
}

void STMFlash_WriteStruct( InfoStructTypedef *InfoStruct, u32_DataTypedef *WriteAddr )
{
	u8 Buffer[18] = {0};

	Buffer[0] = InfoStruct->InfoByte;
	
	Buffer[1] = InfoStruct->Year.arr_data[0];
	Buffer[2] = InfoStruct->Year.arr_data[1];

	Buffer[3] = InfoStruct->Month;
	Buffer[4] = InfoStruct->Day;
	Buffer[5] = InfoStruct->Hour;

	Buffer[6] = InfoStruct->humi_int;
	Buffer[7] = InfoStruct->humi_deci;
	Buffer[8] = InfoStruct->temp_int;
	Buffer[9] = InfoStruct->temp_deci;

	Buffer[10] = InfoStruct->DustDensity.arr_data[0];
	Buffer[11] = InfoStruct->DustDensity.arr_data[1];
	Buffer[12] = InfoStruct->DustDensity.arr_data[2];
	Buffer[13] = InfoStruct->DustDensity.arr_data[3];

	Buffer[14] = InfoStruct->CH2ODensity.arr_data[0];
	Buffer[15] = InfoStruct->CH2ODensity.arr_data[1];
	Buffer[16] = InfoStruct->CH2ODensity.arr_data[2];
	Buffer[17] = InfoStruct->CH2ODensity.arr_data[3];

	STMFLASH_Write(STM32_FLASH_WRITE_START+WriteAddr->u32_data,(u16*)Buffer,9);
	if( WriteAddr->u32_data < 18*7 )
		WriteAddr->u32_data += 18;
	else
		WriteAddr->u32_data = 18;
	STMFLASH_Write(STM32_FLASH_WRITE_START+0, (u16*)WriteAddr->arr_data, 2);
	//printf("WriteStruct Finished.\r\n");
}

/*
	STMFLASH_Write(STM32_FLASH_WRITE_START,(u16*)TEXT_Buffer,SIZE);
	STMFLASH_Read(STM32_FLASH_WRITE_START,(u16*)datatemp,SIZE);
*/

void STMFlash_ReadStruct( InfoStructTypedef *InfoStruct, u32_DataTypedef *ReadAddr )
{
	u8 Buffer[18];

	STMFLASH_Read( STM32_FLASH_WRITE_START+ReadAddr->u32_data, (u16*)Buffer, 9 );
	InfoStruct->InfoByte = Buffer[0];
	
	InfoStruct->Year.arr_data[0] = Buffer[1];
	InfoStruct->Year.arr_data[1] = Buffer[2];

	InfoStruct->Month = Buffer[3];
	InfoStruct->Day = Buffer[4];
	InfoStruct->Hour = Buffer[5];

	InfoStruct->humi_int = Buffer[6];
	InfoStruct->humi_deci = Buffer[7];
	InfoStruct->temp_int = Buffer[8];
	InfoStruct->temp_deci = Buffer[9];

	InfoStruct->DustDensity.arr_data[0] = Buffer[10];
	InfoStruct->DustDensity.arr_data[1] = Buffer[11];
	InfoStruct->DustDensity.arr_data[2] = Buffer[12];
	InfoStruct->DustDensity.arr_data[3] = Buffer[13];

	InfoStruct->CH2ODensity.arr_data[0] = Buffer[14];
	InfoStruct->CH2ODensity.arr_data[1] = Buffer[15];
	InfoStruct->CH2ODensity.arr_data[2] = Buffer[16];
	InfoStruct->CH2ODensity.arr_data[3] = Buffer[17];
	
	if( ReadAddr->u32_data < 18*7 )
		ReadAddr->u32_data += 18;
	else
		ReadAddr->u32_data = 18;

	STMFLASH_Write(STM32_FLASH_WRITE_START+4, (u16*)(ReadAddr->arr_data), 2);
	//printf("ReadStruct Finished.\r\n");
}

void STMFlash_Get_WritePointer( RW_Ptr_Typedef *RW_Ptr_Struct )
{
	u32_DataTypedef datatemp;
	printf("------------WritePointer Start-----------\r\n");
	//write pointer is saved in address from 0 to 3
	STMFLASH_Read( STM32_FLASH_WRITE_START+0, (u16*)datatemp.arr_data, 2 );
	printf("datatemp.u32_data = %d\r\n", datatemp.u32_data);
	//if write address has not been initialized
	if( datatemp.u32_data%18 != 0 )
	{
		datatemp.u32_data = 18;
		STMFLASH_Write( STM32_FLASH_WRITE_START+0, (u16*)datatemp.arr_data, 2 );
		RW_Ptr_Struct->Write.u32_data = datatemp.u32_data;
	}else
	{
		RW_Ptr_Struct->Write.u32_data = datatemp.u32_data;
	}
	printf("WritePointer = %d\r\n", RW_Ptr_Struct->Write.u32_data);
	printf("------------------------------------------\r\n");
}

void STMFlash_Get_ReadPointer( RW_Ptr_Typedef *RW_Ptr_Struct )
{
	u32_DataTypedef datatemp;
	printf("------------ReadPointer Start-----------\r\n");
	//read pointer is saved in address from 4 to 7
	STMFLASH_Read( STM32_FLASH_WRITE_START+4, (u16*)datatemp.arr_data, 2 );
	printf("datatemp.u32_data = %d\r\n", datatemp.u32_data);
	if( datatemp.u32_data%18 != 0 )
	{
		datatemp.u32_data = 18;
		STMFLASH_Write( STM32_FLASH_WRITE_START+4, (u16*)datatemp.arr_data, 2 );
		RW_Ptr_Struct->Read.u32_data = datatemp.u32_data;
	}else
	{
		RW_Ptr_Struct->Read.u32_data = datatemp.u32_data;
	}
	printf("ReadPointer = %d\r\n", RW_Ptr_Struct->Read.u32_data);
	printf("------------------------------------------\r\n");
}
