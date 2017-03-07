#include "bsp.h"

//��ȡָ����ַ�İ���(16λ����)
//faddr:����ַ(�˵�ַ����Ϊ2�ı���!!)
//����ֵ:��Ӧ����.
u16 STMFLASH_ReadHalfWord(u32 faddr)
{
	return *(vu16*)faddr; 
}
#if STM32_FLASH_WREN	//���ʹ����д   
//������д��
//WriteAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��   
void STMFLASH_Write_NoCheck(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)   
{ 			 		 
	u16 i;
	for(i=0;i<NumToWrite;i++)
	{
		FLASH_ProgramHalfWord(WriteAddr,pBuffer[i]);
	    WriteAddr+=2;//��ַ����2.
	}  
} 
//��ָ����ַ��ʼд��ָ�����ȵ�����
//WriteAddr:��ʼ��ַ(�˵�ַ����Ϊ2�ı���!!)
//pBuffer:����ָ��
//NumToWrite:����(16λ)��(����Ҫд���16λ���ݵĸ���.)
#if STM32_FLASH_SIZE<256
	#define STM_SECTOR_SIZE 1024 //�ֽ�
#else 
	#define STM_SECTOR_SIZE	2048
#endif		 
u16 STMFLASH_BUF[STM_SECTOR_SIZE/2];//�����2K�ֽ�
void STMFLASH_Write(u32 WriteAddr,u16 *pBuffer,u16 NumToWrite)
{
	u32 secpos;	   //������ַ
	u16 secoff;	   //������ƫ�Ƶ�ַ(16λ�ּ���)
	u16 secremain; //������ʣ���ַ(16λ�ּ���)	   
 	u16 i;    
	u32 offaddr;   //ȥ��0X08000000��ĵ�ַ
	if(WriteAddr<STM32_FLASH_BASE||(WriteAddr>=(STM32_FLASH_BASE+1024*STM32_FLASH_SIZE)))
		return;//�Ƿ���ַ
	FLASH_Unlock();						//����
	offaddr=WriteAddr-STM32_FLASH_BASE;		//ʵ��ƫ�Ƶ�ַ.
	secpos=offaddr/STM_SECTOR_SIZE;			//������ַ  0~127 for STM32F103RBT6
	secoff=(offaddr%STM_SECTOR_SIZE)/2;		//�������ڵ�ƫ��(2���ֽ�Ϊ������λ.)
	secremain=STM_SECTOR_SIZE/2-secoff;		//����ʣ��ռ��С   
	if(NumToWrite<=secremain)
		secremain=NumToWrite;//�����ڸ�������Χ
	while(1) 
	{	
		STMFLASH_Read(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//������������������
		for(i=0;i<secremain;i++)//У������
		{
			if(STMFLASH_BUF[secoff+i]!=0XFFFF)
				break;//��Ҫ����  	  
		}
		if(i<secremain)//��Ҫ����
		{
			FLASH_ErasePage(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE);//�����������
			for(i=0;i<secremain;i++)//����
			{
				STMFLASH_BUF[i+secoff]=pBuffer[i];	  
			}
			STMFLASH_Write_NoCheck(secpos*STM_SECTOR_SIZE+STM32_FLASH_BASE,STMFLASH_BUF,STM_SECTOR_SIZE/2);//д����������  
		}else
			STMFLASH_Write_NoCheck(WriteAddr,pBuffer,secremain);//д�Ѿ������˵�,ֱ��д������ʣ������. 				   
		if(NumToWrite==secremain)
			break;//д�������
		else//д��δ����
		{
			secpos++;				//������ַ��1
			secoff=0;				//ƫ��λ��Ϊ0 	 
		   	pBuffer+=secremain;  	//ָ��ƫ��
			WriteAddr+=secremain;	//д��ַƫ��	   
		   	NumToWrite-=secremain;	//�ֽ�(16λ)���ݼ�
			if(NumToWrite>(STM_SECTOR_SIZE/2))
				secremain=STM_SECTOR_SIZE/2;//��һ����������д����
			else
				secremain=NumToWrite;//��һ����������д����
		}	 
	};	
	FLASH_Lock();//����
}
#endif

//��ָ����ַ��ʼ����ָ�����ȵ�����
//ReadAddr:��ʼ��ַ
//pBuffer:����ָ��
//NumToWrite:����(16λ)��
void STMFLASH_Read(u32 ReadAddr,u16 *pBuffer,u16 NumToRead)   	
{
	u16 i;
	for(i=0;i<NumToRead;i++)
	{
		pBuffer[i]=STMFLASH_ReadHalfWord(ReadAddr);//��ȡ2���ֽ�.
		ReadAddr+=2;//ƫ��2���ֽ�.	
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
//WriteAddr:��ʼ��ַ
//WriteData:Ҫд�������
void Test_Write(u32 WriteAddr,u16 WriteData)   	
{
	STMFLASH_Write(WriteAddr,&WriteData,1);//д��һ���� 
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
