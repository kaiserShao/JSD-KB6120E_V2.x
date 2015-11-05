/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: CPS120.C
* �� �� ��: ����
* ��  ��  : ����CPS120
* ����޸�: 2014��4��19��
*********************************** �޶���¼ ***********************************
* ��  ��: V2.0
* �޶���: ����
* ˵  ��: ��Load��������������ֻ�ṩRead�ӿڣ�ֱ�ӷ���ѹ��ֵ��
*******************************************************************************/
#include "BSP.H"
#include "BIOS.H"

BOOL	CPS120_Load ( uint8_t IO_Buf[4] )
{
	if ( ! bus_i2c_start( _SLAVE_CPS120, I2C_Read ))
	{
		bus_i2c_stop();
		return	FALSE;
	}
	
	IO_Buf[0] = bus_i2c_shin( I2C_ACK );	// Receive and send ACK
	IO_Buf[1] = bus_i2c_shin( I2C_ACK );	// Receive and send ACK
	IO_Buf[2] = bus_i2c_shin( I2C_ACK );	// Receive and send ACK
	IO_Buf[3] = bus_i2c_shin( I2C_NoACK );	// Receive and send NoACK
	bus_i2c_stop();

	//	����ת���Ա��´ζ�ȡ
	( void )bus_i2c_start( _SLAVE_CPS120, I2C_Write );
	bus_i2c_stop();

	return	TRUE;
}

BOOL	CPS120_Read ( uint16_t * pBa, uint16_t * pTemp )
{
	uint8_t	CPS120_Buf[4];
	BOOL	state = FALSE;
	
	bus_i2c_mutex_apply();
	state = CPS120_Load( CPS120_Buf );
	if ( state )
	{
		* pBa = (( CPS120_Buf[1] + ( CPS120_Buf[0] * 256u )) & 0x3FFFu );
		* pTemp = ((( CPS120_Buf[3] + ( CPS120_Buf[2] * 256u )) >> 2 ) & 0x3FFFu );
	}
	bus_i2c_mutex_release();

	return	state;
}

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/