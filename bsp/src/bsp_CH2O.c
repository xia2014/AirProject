#include "bsp.h"

/*
ע��ͨ��ģʽΪ�����ϴ�ʽ�����������Ũ��ֵ�ĵ�λ��ppb��1000 ppb=1 ppm��
��ȩ���õ�λΪmg/m3 ���� ppm
ppm   �����Ũ�ȱ�ʾ����һ��������Ŀ����� ������Ⱦ����������
mg/m3:����Ũ�ȱ�ʾ����ÿ�����׿�����������Ⱦ�����������
 
Ũ�ȵ�λppm��mg/m3�Ļ��㣺����ʽ���㣺
mg/m3=M���������/22.4��ppm��[273/(273+T�����¶�)]*��Baѹ��/101325�� 
��25���1������ѹ�£�����ȩ������30��
����
mg/m3 = ppm*M���������/22.4   �� mg/m3��ppm*1.3392857��ppb*0.0013392857

ppm =  mg/m3��22.4/M��������� �� ppm��  mg/m3��0.74666
*/


void Read_CH2O(uint32_t* ch2o_m)
{
	uint8_t UART_Upload[9];
	uint8_t temp_ppb;
	uint8_t rx;
    uint8_t i; 
	uint32_t ch2o;
	
	rx = 0x00;
	for( i = 1; i < 9; ++i )
		UART_Upload[i] = 0;

	while(USART_GetFlagStatus(USART3,USART_FLAG_RXNE) != SET);
	rx = USART_ReceiveData(USART3);

	if(rx == 0xFF)
	{
		UART_Upload[0] = rx;
		for( i = 1; i < 9; ++i )
		{
			while(USART_GetFlagStatus(USART3,USART_FLAG_RXNE) != SET);
			rx = USART_ReceiveData(USART3);

			UART_Upload[i] = rx;
		}
	}
	temp_ppb = ~(UART_Upload[1] + UART_Upload[2] + UART_Upload[3] + UART_Upload[4] + UART_Upload[5] + UART_Upload[6] + UART_Upload[7]) + 1;	//У���
	if(temp_ppb == UART_Upload[8])
	{
		ch2o = UART_Upload[4]*256 + UART_Upload[5];
		*ch2o_m = ch2o*13.392857;
	}
	printf("ch2o_m\r\n = %d", *ch2o_m);
//	printf("UART_Upload = %s\r\n",UART_Upload);
//	printf("%s\r\n",UART_Upload);

}
