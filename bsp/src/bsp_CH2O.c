#include "bsp.h"

/*
注：通信模式为主动上传式计算出来气体浓度值的单位是ppb（1000 ppb=1 ppm）
甲醛常用单位为mg/m3 或者 ppm
ppm   ：体积浓度表示法，一百万体积的空气中 所含污染物的体积数；
mg/m3:质量浓度表示法，每立方米空气中所含污染物的质量数；
 
浓度单位ppm与mg/m3的换算：按下式计算：
mg/m3=M气体分子量/22.4·ppm·[273/(273+T气体温度)]*（Ba压力/101325） 
在25℃和1个大气压下：（甲醛分子量30）
即：
mg/m3 = ppm*M气体分子量/22.4   → mg/m3≈ppm*1.3392857≈ppb*0.0013392857

ppm =  mg/m3×22.4/M气体分子量 → ppm≈  mg/m3×0.74666
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
	temp_ppb = ~(UART_Upload[1] + UART_Upload[2] + UART_Upload[3] + UART_Upload[4] + UART_Upload[5] + UART_Upload[6] + UART_Upload[7]) + 1;	//校验和
	if(temp_ppb == UART_Upload[8])
	{
		ch2o = UART_Upload[4]*256 + UART_Upload[5];
		*ch2o_m = ch2o*13.392857;
	}
	printf("ch2o_m\r\n = %d", *ch2o_m);
//	printf("UART_Upload = %s\r\n",UART_Upload);
//	printf("%s\r\n",UART_Upload);

}
