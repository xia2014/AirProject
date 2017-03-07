#include "bsp.h"

static bool BT05_Cmd ( char * cmd, uint16_t cmd_len, char * reply1, char * reply2, u32 waittime );

/*
 * ��������BT05_Cmd
 * ����  ����BT05ģ�鷢��ATָ��
 * ����  ��cmd�������͵�ָ��
 *         reply1��reply2���ڴ�����Ӧ��ΪNULL������Ӧ������Ϊ���߼���ϵ
 *         waittime���ȴ���Ӧ��ʱ��
 * ����  : 1��ָ��ͳɹ�
 *         0��ָ���ʧ��
 * ����  �����ⲿ����
 */
static bool BT05_Cmd ( char *cmd, uint16_t cmd_len, char * reply1, char * reply2, u32 waittime )
{
	char buf[20];
	BT05_Usart ( (uint8_t *)cmd, cmd_len );

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //����Ҫ��������
		return true;
	
	DelayMS ( waittime );                 //��ʱ
/*
	if( !Usart2_Printf_Flag )
		DEBUG_PRINTF( "%s", BlueTooth_PC_Str .Data_RX_BUF );
*/
	if(comGetBuff(COM2,(uint8_t *)buf))
	{	
		comSendBuf(COM1, (uint8_t *)buf, strlen(buf));
		return true;
	}else{
		return false;
	}
		
		/*if ( ( reply1 != 0 ) && ( reply2 != 0 ) )
		return ( ( bool ) strstr ( BlueTooth_PC_Str .Data_RX_BUF, reply1 ) || 
						 ( bool ) strstr ( BlueTooth_PC_Str .Data_RX_BUF, reply2 ) ); 
 	
	else if ( reply1 != 0 )
		return ( ( bool ) strstr ( BlueTooth_PC_Str .Data_RX_BUF, reply1 ) );
	
	else
		return ( ( bool ) strstr ( BlueTooth_PC_Str .Data_RX_BUF, reply2 ) );
	*/
}

/*
 * ��������BT05_Password_Set
 * ����  ��ΪBT05ģ�����������
 * ����  ��������ַ���pPassword
 * ����  : ��
 * ����  �����ⲿ����
 */
bool BT05_Password_Set( char * pPassword )
{
	char cCmd [50];

	sprintf ( cCmd, "AT+PIN%s\r\n", pPassword );
	
	return BT05_Cmd ( cCmd, strlen(cCmd), "OK", NULL, 2000 );
}

/*
 * ��������BT05_Work_Type_Choose
 * ����  ��ΪBT05ģ�����ù�������
 * ����  ����������enumMode
 * ����  : 0������ʧ��
		   1�����óɹ�
 * ����  �����ⲿ����
 */
bool BT05_Work_Type_Choose( ENUM_WorkTypeDef enumMode )
{
	char cCmd [50];
	switch ( enumMode )
	{
		case NO_PSW:
			sprintf ( cCmd, "AT+TYPE0\r\n" );
			return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 2000 ); 
		case SIMPLE_PAIR:
			sprintf ( cCmd, "AT+TYPE1\r\n" );
			return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 2000 ); 
		case PSW_PAIR:
			sprintf ( cCmd, "AT+TYPE2\r\n" );
			return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 2000 ); 
		case PSW_PAIR_BIND:
			sprintf ( cCmd, "AT+TYPE3\r\n" );
			return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 2000 ); 		
		default:
			return false;
	}
}

/*
 * ��������BT05_AT_Test
 * ����  ����BT05ģ�����AT��������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_AT_Test ( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT\r\n" );
	while ( ! BT05_Cmd ( cCmd, strlen(cCmd), "OK", NULL, 1000 ) );  	
}

/*
 * ��������BT05_Rst
 * ����  ����BT05ģ�����������500ms���Զ�������
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Rst ( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+RESET\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 600 );
	//Usart2_Printf_Flag = 1;
}

/*
 * ��������BT05_Set_Default
 * ����  ����BT05ģ��ָ�Ĭ�����ã�500ms���Զ����ã�
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Set_Default( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+DEFAULT\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 600 );
}

/*
 * ��������BT05_Start_LowPower
 * ����  ����BT05ģ������Ϊ����ʱ�Ƿ��Զ�����͹���ģʽ
 * ����  ��0���Զ�����͹���ģʽ
		   1����������
 * ����  : 1�����óɹ�
		   0������ʧ��
 * ����  �����ⲿ����
 */
bool BT05_Start_LowPower( u8 Mode )
{
	char cCmd [50];
	
	if( Mode == 0 )
	{
		sprintf ( cCmd, "AT+PWRM\r\n" );
		return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 1000 );
	}
	else if( Mode == 1 )
	{
		sprintf ( cCmd, "AT+PWRM\r\n" );
		return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 1000 );
	}
	else
		return false;
}

/*
 * ��������BT05_Set_LowPower
 * ����  ��ʹBT05ģ��ֱ�ӽ���͹���ģʽ
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Set_LowPower( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+SLEEP\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 1000 );
}

/*
 * ��������BT05_Work_Mode_Choose
 * ����  ����BT05ģ������Ϊ��/��ģʽ
 * ����  ��0�����豸
		   1�����豸
 * ����  : 0������ʧ��
		   1�����óɹ�
 * ����  �����ⲿ����
 */
bool BT05_Work_Mode_Choose( ENUM_WorkModeDef WorkMode )
{
	char cCmd [50];
	
	switch( WorkMode )
	{
		case PERIPHERAL:
			sprintf ( cCmd, "AT+ROLE0\r\n" );
			return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 1000 );
		case MASTER:
			sprintf ( cCmd, "AT+ROLE1\r\n" );
			return BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 1000 );
		default:
			return false;
	}
}

/*
 * ��������BT05_Search_Device
 * ����  �����������豸����ģʽָ�
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Search_Device( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+INQ\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 1000 );
}

/*
 * ��������BT05_Connect_Remote_Device
 * ����  ��ʹBT05ģ������Զ���豸����ģʽָ�
 * ����  ���豸���DeviceNum
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Connect_Remote_Device( u8 DeviceNum )
{
	char cmd[30];
	sprintf(cmd,"AT+CONN%d",DeviceNum);
	BT05_Cmd ( cmd, strlen(cmd), "OK", NULL, 2500 );
}

/*
 * ��������BT05_Set_Power
 * ����  ������BT05ģ��ķ��书��
 * ����  �����ʵȼ�Level
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Set_Power( u8 Level )
{
	char cmd[30];
	sprintf( cmd, "AT+POWE%d\r\n", Level );
	BT05_Cmd( cmd, strlen(cmd), "OK", NULL, 1000 );
}

/*
 * ��������BT05_Get_Help
 * ����  ����ȡATָ�����
 * ����  ����
 * ����  : ��
 * ����  �����ⲿ����
 */
void BT05_Get_Help( void )
{
	char cCmd[30];
	//Usart2_Printf_Flag = 1;
	sprintf( cCmd, "AT+HELP\r\n" );
	BT05_Usart ( (uint8_t*)cCmd, strlen(cCmd) );
}

void BT05_State_Line_Init( void )
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_APB2PeriphClockCmd( BT05_STATE_CLOCK, ENABLE );
	GPIO_InitStructure.GPIO_Pin = BT05_STATE_PIN;	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 

	GPIO_Init( BT05_STATE_PORT, &GPIO_InitStructure );	
}

void bsp_InitBT05( void )
{
	//BT05_Work_Type_Choose( PSW_PAIR_BIND );
	//BT05_Password_Set( "123456" );
	BT05_Rst();
}
