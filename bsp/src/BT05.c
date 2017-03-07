#include "bsp.h"

static bool BT05_Cmd ( char * cmd, uint16_t cmd_len, char * reply1, char * reply2, u32 waittime );

/*
 * 函数名：BT05_Cmd
 * 描述  ：对BT05模块发送AT指令
 * 输入  ：cmd，待发送的指令
 *         reply1，reply2，期待的响应，为NULL表不需响应，两者为或逻辑关系
 *         waittime，等待响应的时间
 * 返回  : 1，指令发送成功
 *         0，指令发送失败
 * 调用  ：被外部调用
 */
static bool BT05_Cmd ( char *cmd, uint16_t cmd_len, char * reply1, char * reply2, u32 waittime )
{
	char buf[20];
	BT05_Usart ( (uint8_t *)cmd, cmd_len );

	if ( ( reply1 == 0 ) && ( reply2 == 0 ) )                      //不需要接收数据
		return true;
	
	DelayMS ( waittime );                 //延时
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
 * 函数名：BT05_Password_Set
 * 描述  ：为BT05模块设置配对码
 * 输入  ：配对码字符串pPassword
 * 返回  : 无
 * 调用  ：被外部调用
 */
bool BT05_Password_Set( char * pPassword )
{
	char cCmd [50];

	sprintf ( cCmd, "AT+PIN%s\r\n", pPassword );
	
	return BT05_Cmd ( cCmd, strlen(cCmd), "OK", NULL, 2000 );
}

/*
 * 函数名：BT05_Work_Type_Choose
 * 描述  ：为BT05模块设置工作类型
 * 输入  ：工作类型enumMode
 * 返回  : 0，设置失败
		   1，设置成功
 * 调用  ：被外部调用
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
 * 函数名：BT05_AT_Test
 * 描述  ：对BT05模块进行AT测试启动
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_AT_Test ( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT\r\n" );
	while ( ! BT05_Cmd ( cCmd, strlen(cCmd), "OK", NULL, 1000 ) );  	
}

/*
 * 函数名：BT05_Rst
 * 描述  ：对BT05模块进行重启（500ms后自动重启）
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_Rst ( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+RESET\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 600 );
	//Usart2_Printf_Flag = 1;
}

/*
 * 函数名：BT05_Set_Default
 * 描述  ：对BT05模块恢复默认设置（500ms后自动重置）
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_Set_Default( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+DEFAULT\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 600 );
}

/*
 * 函数名：BT05_Start_LowPower
 * 描述  ：将BT05模块设置为启动时是否自动进入低功耗模式
 * 输入  ：0，自动进入低功耗模式
		   1，正常工作
 * 返回  : 1，设置成功
		   0，设置失败
 * 调用  ：被外部调用
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
 * 函数名：BT05_Set_LowPower
 * 描述  ：使BT05模块直接进入低功耗模式
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_Set_LowPower( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+SLEEP\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "ready", 1000 );
}

/*
 * 函数名：BT05_Work_Mode_Choose
 * 描述  ：将BT05模块设置为主/从模式
 * 输入  ：0，从设备
		   1，主设备
 * 返回  : 0，设置失败
		   1，设置成功
 * 调用  ：被外部调用
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
 * 函数名：BT05_Search_Device
 * 描述  ：搜索蓝牙设备（主模式指令）
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_Search_Device( void )
{
	char cCmd [50];
	sprintf ( cCmd, "AT+INQ\r\n" );
	BT05_Cmd ( cCmd, strlen(cCmd), "OK", "no change", 1000 );
}

/*
 * 函数名：BT05_Connect_Remote_Device
 * 描述  ：使BT05模块连接远程设备（主模式指令）
 * 输入  ：设备编号DeviceNum
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_Connect_Remote_Device( u8 DeviceNum )
{
	char cmd[30];
	sprintf(cmd,"AT+CONN%d",DeviceNum);
	BT05_Cmd ( cmd, strlen(cmd), "OK", NULL, 2500 );
}

/*
 * 函数名：BT05_Set_Power
 * 描述  ：设置BT05模块的发射功率
 * 输入  ：功率等级Level
 * 返回  : 无
 * 调用  ：被外部调用
 */
void BT05_Set_Power( u8 Level )
{
	char cmd[30];
	sprintf( cmd, "AT+POWE%d\r\n", Level );
	BT05_Cmd( cmd, strlen(cmd), "OK", NULL, 1000 );
}

/*
 * 函数名：BT05_Get_Help
 * 描述  ：获取AT指令帮助
 * 输入  ：无
 * 返回  : 无
 * 调用  ：被外部调用
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
