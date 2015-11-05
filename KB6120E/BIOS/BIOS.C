/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: BIOS.C
* �� �� ��: ����
* ��  ��  : KB6120E оƬ�˿ڷ���
* ����޸�: 2014��1��14��
*********************************** �޶���¼ ***********************************
* ��  ��: 
* �޶���: 
*******************************************************************************/
#include "BSP.H"
#include "Pin.H"
#include "BIOS.H"

#ifdef UseHSE
#define	SYSCLK_FREQ_24MHz 24000000
#include "system_stm32f10x.c"
#else
#include "SystemInit_HSI.c"
#endif

// 	#define	_DINT()	__disable_irq()
// 	#define	_EINT()	__enable_irq()

#define	PinBB( _Port, _Num )	(*(__IO int32_t *)(PERIPH_BB_BASE + ((uint32_t)&(_Port) - PERIPH_BASE) * 32u + (_Num) * 4u ))

//	0��(0000B)ģ������ģʽ
//	4��(0100B)��������ģʽ(��λ���״̬)
//	8��(1000B)����/��������ģʽ
//	C��(1100B)����
//	
//	1��(0001B)ͨ���������ģʽ10MHz
//	2��(0010B)ͨ���������ģʽ2MHz
//	3��(0011B)ͨ���������ģʽ50MHz
//	
//	5��(0101B)ͨ�ÿ�©���ģʽ10MHz
//	6��(0110B)ͨ�ÿ�©���ģʽ2MHz
//	7��(0111B)ͨ�ÿ�©���ģʽ50MHz
//	
//	9��(1001B)���ù����������ģʽ10MHz
//	A��(1010B)���ù����������ģʽ2MHz
//	B��(1011B)���ù����������ģʽ50MHz
//	
//	D��(1101B)���ù��ܿ�©���ģʽ10MHz
//	E��(1110B)���ù��ܿ�©���ģʽ2MHz
//	F��(1111B)���ù��ܿ�©���ģʽ50MHz

/********************************** ����˵�� ***********************************
*	����ʱ��������Ӳ��ͬ������ȷ��ʱ��Ҫ��ֹ�жϵĸ���
*******************************************************************************/
#pragma	push
#pragma O3	Ospace
void	delay_us ( uint32_t us )
{
	while ( us-- )
	{
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
		__NOP(); __NOP(); __NOP(); __NOP(); __NOP();
	}
}
#pragma	pop

/********************************** ����˵�� ***********************************
*	ʹ�� GPIOB �ĸ�8λ ʵ�� 8 λ ����IO
*******************************************************************************/
static	void	P8P_ModeInput( void )
{
	GPIOB->CRH  = 0x44444444u;	//	�������뷽ʽ
}

static	void	P8P_ModeOutput( void )
{
	GPIOB->CRH  = 0x33333333u;	//	���������ʽ
}

static	uint8_t	P8P_PortRead( void )
{
	return	(uint8_t)( GPIOB->IDR >> 8 );
}

static	void	P8P_PortWrite( uint8_t OutByte )
{
	GPIOB->BSRR = ( 0xFF00uL << 16 )
								| ( 0xFF00uL & ((uint32_t)OutByte << 8 ));
}

/********************************** ����˵�� ***********************************
*	�����˿�ɨ��
*******************************************************************************/
uint8_t	Keyboard_PortRead( void )
{
	uint8_t	PortState = 0u;

//	PortState = (( GPIOA->IDR & 0xFCu ) >> 2 ) | (( GPIOB->IDR & 0x03u ) << 6 );
//	return	(uint8_t)(~PortState);
	PortState = (( GPIOA->IDR & 0xFCu ) >> 2 ) | (( GPIOB->IDR & 0x01u ) << 6 );
	return	(uint8_t)(~PortState) & 0x7F;
}

void	Keyboard_PortInit( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
	//	����ʹ����������ģʽ
	GPIOA->BSRR = 0x00FCu;
	GPIOB->BSRR = 0x0003u;
	MODIFY_REG( GPIOA->CRL, 0xFFFFFF00u, 0x88888800u );
//	MODIFY_REG( GPIOB->CRL, 0x000000FFu, 0x00000088u );
	MODIFY_REG( GPIOB->CRL, 0x0000000Fu, 0x00000008u );	
}

/********************************** ����˵�� ***********************************
*	�����жϿ���
*******************************************************************************/
void	Keyboard_IRQ_Enable( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );

	AFIO->EXTICR[0] = AFIO_EXTICR1_EXTI0_PB
	//				| AFIO_EXTICR1_EXTI1_PB
					| AFIO_EXTICR1_EXTI2_PA
					| AFIO_EXTICR1_EXTI3_PA
					;
	AFIO->EXTICR[1] = AFIO_EXTICR2_EXTI4_PA
					| AFIO_EXTICR2_EXTI5_PA
					| AFIO_EXTICR2_EXTI6_PA
					| AFIO_EXTICR2_EXTI7_PA
					;
	CLEAR_BIT( EXTI->EMR,  0x03FFu );	// no event
	CLEAR_BIT( EXTI->IMR,  0x0300u );	// ��ֹ�ж� 8#,9#
	SET_BIT(   EXTI->RTSR, 0x00FFu );	// rising edge trigger
	SET_BIT(   EXTI->FTSR, 0x00FFu );	// falling edge trigger
	WRITE_REG( EXTI->PR,   0x00FFu );	// д1��λ�жϱ�־λ
//	SET_BIT(   EXTI->IMR,  0x00FFu );	// �����ж� 0#~7#
	SET_BIT(   EXTI->IMR,  0x00FDu );	// �����ж� 0#~7#,PB1�������жϣ��Ľ�ģ���źš�
}

void	Keyboard_IRQ_Disable( void )
{
	WRITE_REG( EXTI->PR,   0x00FFu );	// д1��λ�жϱ�־λ
	CLEAR_BIT( EXTI->IMR,  0x03FFu );	// ��ֹ�ж�,���� 8#,9#
}

/********************************** ����˵�� ***********************************
*	OLED9704 �˿ڶ�д
*******************************************************************************/
#define	Pin_OLED9704_RES	PinBB( GPIOC->ODR, 0U )
#define	Pin_OLED9704_CS 	PinBB( GPIOC->ODR, 2U )
#define	Pin_OLED9704_RD 	PinBB( GPIOC->ODR, 3U )
#define	Pin_OLED9704_WR 	PinBB( GPIOC->ODR, 4U )
#define	Pin_OLED9704_DC 	PinBB( GPIOC->ODR, 5U )
#define	Pin_OLED9704_DISB	PinBB( GPIOC->ODR, 6U )

uint8_t	OLED9704_ReadState( void )
{
	uint8_t state;

	P8P_ModeInput();
	Pin_OLED9704_DC = 0;

	Pin_OLED9704_CS = 0;
	Pin_OLED9704_RD = 0;
	delay_us( 1 );	state = P8P_PortRead();
	Pin_OLED9704_RD = 1;
	Pin_OLED9704_CS = 1;
	P8P_ModeOutput();

	return state;
}

uint8_t	OLED9704_ReadData( void )
{
    uint8_t InData;

	P8P_ModeInput();
	Pin_OLED9704_DC = 1;

	Pin_OLED9704_CS = 0;
	Pin_OLED9704_RD = 0;
	delay_us( 1 );
	Pin_OLED9704_RD = 1;
	delay_us( 1 );
	Pin_OLED9704_RD = 0;
	delay_us( 1 );	InData = P8P_PortRead();
	Pin_OLED9704_RD = 1;
	Pin_OLED9704_CS = 1;

    return InData;
}

void  OLED9704_WriteData( uint8_t OutData )
{
	P8P_PortWrite( OutData );
	P8P_ModeOutput();
	Pin_OLED9704_DC = 1;

	Pin_OLED9704_CS = 0;
	Pin_OLED9704_WR = 0;
	delay_us( 1 );
	Pin_OLED9704_WR = 1;
	Pin_OLED9704_CS = 1;
}

void  OLED9704_WriteReg( uint8_t OutCommand )
{
	P8P_PortWrite( OutCommand );
	P8P_ModeOutput();
	Pin_OLED9704_DC = 0;

	Pin_OLED9704_CS = 0;
	Pin_OLED9704_WR = 0;
	delay_us( 1 );
	Pin_OLED9704_WR = 1;
	Pin_OLED9704_CS = 1;
}	

void	OLED9704_DisplayDisable( void )
{
	Pin_OLED9704_DISB = 1;
}

void	OLED9704_DisplayEnable( void )
{
	Pin_OLED9704_DISB = 0;
}

void	OLED9704_PortInit( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
	P8P_PortWrite( 0u );
	P8P_ModeInput( );

	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	GPIOC->BSRR = 0x007Fu;
 	MODIFY_REG( GPIOC->CRL, 0x0FFFFFFFu, 0x03333333u );

 	delay_us( 10u );
 	Pin_OLED9704_RES = 0;
 	delay_us( 10u );
 	Pin_OLED9704_RES = 1;
}

/********************************** ����˵�� ***********************************
*	TM12864 �˿ڶ�д
*******************************************************************************/
#define	Pin_TM12864_RST 	PinBB( GPIOC->ODR, 0U )
#define	Pin_TM12864_CS2 	PinBB( GPIOC->ODR, 1U )
#define	Pin_TM12864_CS1 	PinBB( GPIOC->ODR, 2U )
#define	Pin_TM12864_EN  	PinBB( GPIOC->ODR, 3U )
#define	Pin_TM12864_RW  	PinBB( GPIOC->ODR, 4U )
#define	Pin_TM12864_DI  	PinBB( GPIOC->ODR, 5U )

uint8_t	TM12864_ReadState( void )
{
	uint8_t	state;
	
	P8P_ModeInput();
	Pin_TM12864_DI = 0;				// �����ݿڶ�����
	Pin_TM12864_RW = 1;
	Pin_TM12864_EN = 1;
	delay_us( 1 );	state = P8P_PortRead();
	Pin_TM12864_EN = 0;

	return	state;
}

static	BOOL	TM12864_isReady( void )
{
	uint8_t i;

	for ( i = 200U; i != 0U; --i )
	{	// waitting for Ready
		if ( ! ( TM12864_ReadState() & 0x80U ))
		{
			return TRUE;
		}
	}
	return FALSE;
}

uint8_t	TM12864_ReadData( void )
{
	uint8_t	InData;

	if ( !TM12864_isReady()){	return	0U;	}

	P8P_ModeInput();
	Pin_TM12864_DI = 1;				// ���ݲ���
	Pin_TM12864_RW = 1;				// ������
	Pin_TM12864_EN = 1;
	delay_us( 10 );
	Pin_TM12864_EN = 0;				// �����ݿڶ����ݣ����
	delay_us( 10 );
	Pin_TM12864_EN = 1;
	delay_us( 10 );	InData = P8P_PortRead();
	Pin_TM12864_EN = 0;				// �����ݿڶ����ݣ�ʵ��

	return InData;
}

void	TM12864_WriteCommand( uint8_t OutCommand )
{   // �������
	if ( !TM12864_isReady()){	return;	}

	P8P_PortWrite( OutCommand );	// ������������ݿ�
	P8P_ModeOutput();

	Pin_TM12864_DI = 0;				// �������
	Pin_TM12864_RW = 0;				// д���
	Pin_TM12864_EN = 1;
	delay_us( 1 );
	Pin_TM12864_EN = 0;				// �����LCM
}

void	TM12864_WriteData( uint8_t OutData )
{
	if ( !TM12864_isReady()){	return;	}

	P8P_PortWrite( OutData );		// ������������ݿ�
	P8P_ModeOutput();

	Pin_TM12864_DI = 1;				// �������
	Pin_TM12864_RW = 0;				// д���
	Pin_TM12864_EN = 1;
	delay_us( 1 );
	Pin_TM12864_EN = 0;				// �����LCM
}

void	TM12864_SelectL( void )
{
	Pin_TM12864_CS1 = 1;
	Pin_TM12864_CS2 = 0;
}

void	TM12864_SelectR( void )
{
	Pin_TM12864_CS2 = 1;
	Pin_TM12864_CS1 = 0;
}

void	TM12864_PortInit( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
	P8P_PortWrite( 0u );
	P8P_ModeInput( );

	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	GPIOC->BSRR = 0x003Fu;
	Pin_TM12864_EN  = 0;
 	MODIFY_REG( GPIOC->CRL, 0x00FFFFFFu, 0x00333333u );

	delay_us( 10u );
	Pin_TM12864_RST = 0;
	delay_us( 10u );
	Pin_TM12864_RST = 1;
}

void	TM12864_GrayInit( void )
{
	TIM_TypeDef	* TIMx = TIM3;

	SET_BIT( TIMx->CCER, TIM_CCER_CC1E );	//	T3CH1 ��LCD�ı����������
	SET_BIT( TIMx->CCER, TIM_CCER_CC2E );	//	T3CH2 ��LCD�ĸ�ѹ������·

	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	MODIFY_REG( GPIOC->CRL, 0xFF000000u, 0xBB000000u );		//	PC.6, T3CH1  PC.7  T3CH2
}

/********************************** ����˵�� ***********************************
*	LM2068E Һ��ģ���д������оƬS1D13700��������ͺţ�ʵ�ʰ�SED1335��д��
*******************************************************************************/
#define	Pin_LCD1335_WR  	PinBB( GPIOC->ODR, 4U )
#define	Pin_LCD1335_RD  	PinBB( GPIOC->ODR, 3U )
#define	Pin_LCD1335_CE		PinBB( GPIOC->ODR, 2U )
#define	Pin_LCD1335_A0  	PinBB( GPIOC->ODR, 5U )
#define	Pin_LCD1335_RST 	PinBB( GPIOC->ODR, 0U )
void 	SED1335_Delay()
{
	__NOP();__NOP();__NOP();__NOP();
	__NOP();__NOP();__NOP();__NOP();
	__NOP();__NOP();__NOP();__NOP();
}
void	SED1335_PortInit( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
	P8P_PortWrite( 0u );
	P8P_ModeInput( );

	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	MODIFY_REG( GPIOC->CRL, 0x00FFFFFFu, 0x00333333u );
	GPIOC->BSRR = 0x003Fu;

	delay( 10u );
	Pin_LCD1335_RST = 0;    delay( 10u );
	Pin_LCD1335_RST = 1;    delay( 10u );
	Pin_LCD1335_RST = 0;    delay( 10u );
	Pin_LCD1335_RST = 1;    delay( 10u );
}

void	SED1335_Write_Command( uint8_t OutCommand )
{
	//	Pin_LCD1335_RD = 1;
	//	Pin_LCD1335_WR = 1;
	//	Pin_LCD1335_CE = 1;
	__NOP();	
	Pin_LCD1335_A0 = 1;
	P8P_PortWrite( OutCommand );
	P8P_ModeOutput();
	__NOP();
	Pin_LCD1335_CE = 0;
	Pin_LCD1335_WR = 0;
	__NOP();
	Pin_LCD1335_WR = 1;
	Pin_LCD1335_CE = 1;
	P8P_ModeInput();
	__NOP();
}

void	SED1335_Write_Data( uint8_t OutData )
{
	//	Pin_LCD1335_RD = 1;
	//	Pin_LCD1335_WR = 1;
	//	Pin_LCD1335_CE = 1;
	__NOP();	
	Pin_LCD1335_A0 = 0;
	P8P_PortWrite( OutData );
	P8P_ModeOutput();
	__NOP();
	Pin_LCD1335_CE = 0;
	Pin_LCD1335_WR = 0;
	__NOP();
	Pin_LCD1335_WR = 1;
	Pin_LCD1335_CE = 1;
	P8P_ModeInput();
	__NOP();
}

uint8_t	SED1335_Read_Data( )
{
    uint8_t	InData;

	//	Pin_LCD1335_RD = 1;
	//	Pin_LCD1335_WR = 1;
	//	Pin_LCD1335_CE = 1;
	__NOP();    
	Pin_LCD1335_A0 = 1;
	P8P_ModeInput();
	SED1335_Delay();
	Pin_LCD1335_CE = 0;
	Pin_LCD1335_RD = 0;
	SED1335_Delay();
	InData = P8P_PortRead();	//	READ
	Pin_LCD1335_RD = 1;
	Pin_LCD1335_CE = 1;
	__NOP();

	return	InData;
}

void	LM2068E_GrayInit( void )
{
	TM12864_GrayInit( );
}

/********************************** ����˵�� ***********************************
*	���ʵ�����
*******************************************************************************/
#define	_OW_Pin_0_Input()		(PinBB( GPIOA->IDR, 15U ))
#define	_OW_Pin_0_Output(_b)	(PinBB( GPIOA->ODR, 15U )=(_b))

BOOL	OW_0_Init( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
 	MODIFY_REG( AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE );

	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	MODIFY_REG( GPIOA->CRH, 0xF0000000u, 0x70000000u );

	_OW_Pin_0_Output(1);
	delay_us( 5 );
	return	_OW_Pin_0_Input();
}

static	BOOL	_OW_0_Reset( void )
{
	BOOL	acknowledge;
	// 	DINT();
	delay_us( 1 );						// ��ʱG(0us)
	_OW_Pin_0_Output(0);				// ��������Ϊ�͵�ƽ
	delay_us( 480U );					// ��ʱH(480us)
	_OW_Pin_0_Output(1);				// �ͷ�����		
	delay_us( 70U );					// ��ʱI(70us)
	acknowledge = ! _OW_Pin_0_Input();	// ���������߲���, 0 ��ʾ��������Ӧ��, 1 ��ʾ��Ӧ��
	// 	EINT();
	delay_us( 410U );					// ��ʱJ(410us)
	return	acknowledge;
}

static	BOOL	_OW_0_Slot( BOOL bitOut )
{
	BOOL	bitIn;
	// 	DINT();
	_OW_Pin_0_Output(0);				// ��������������һ��ʱ϶
	delay_us( 6U );						// ��ʱA(6us)
	_OW_Pin_0_Output(bitOut);			// ���Ҫд�����ߵ�λ
	delay_us( 9U );						// ��ʱE(9us)
	bitIn = _OW_Pin_0_Input();			// ���������߲���, �Զ�ȡ�ӻ��ϵ�����
	delay_us( 45U );					// ��ʱC(60us)-A-E == 45us
	_OW_Pin_0_Output(1);				// ʱ϶��ʼ���60us���߸���
	// 	EINT();
	delay_us( 10U );					// ��ʱD(10us), �ȴ����߻ָ�

	return	bitIn;
}


#define	_OW_Pin_1_Input()		(PinBB( GPIOD->IDR,  2U ))
#define	_OW_Pin_1_Output(_b)	(PinBB( GPIOD->ODR,  2U )=(_b))

BOOL	OW_1_Init( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPDEN );
	MODIFY_REG( GPIOD->CRL, 0x00000F00u, 0x00000700u );

	_OW_Pin_1_Output(1);
	delay_us( 5 );
	return	_OW_Pin_1_Input();
}

static	BOOL	_OW_1_Reset( void )
{
	BOOL	acknowledge;
	//	DINT();
	delay_us( 1 );						// ��ʱG(0us)
	_OW_Pin_1_Output(0);				// ��������Ϊ�͵�ƽ
	delay_us( 480U );					// ��ʱH(480us)
	_OW_Pin_1_Output(1);				// �ͷ�����		
	delay_us( 70U );					// ��ʱI(70us)	70
	acknowledge = ! _OW_Pin_1_Input();	// ���������߲���, 0 ��ʾ��������Ӧ��, 1 ��ʾ��Ӧ��
	//	EINT();
	delay_us( 410U );					// ��ʱJ(410us)

	return	acknowledge;
}

static	BOOL	_OW_1_Slot( BOOL bitOut )
{
	BOOL	bitIn;
	// 	DINT();
	_OW_Pin_1_Output(0);				// ��������������һ��ʱ϶
	delay_us( 6U );						// ��ʱA(6us)
	_OW_Pin_1_Output( bitOut );			// ���Ҫд�����ߵ�λ
	delay_us( 9U );						// ��ʱE(9us)
	bitIn = _OW_Pin_1_Input();			// ���������߲���, �Զ�ȡ�ӻ��ϵ�����
	delay_us( 45U );					// ��ʱC(60us)-A-E == 45us
	_OW_Pin_1_Output(1);				// ʱ϶��ʼ���60us���߸���
	// 	EINT();
	delay_us( 10U );					// ��ʱD(10us), �ȴ����߻ָ�

	return	bitIn;
}


__svc_indirect(0)  BOOL	_SVCCall_Reset( BOOL (*)( void ));
__svc_indirect(0)  BOOL	_SVCCall_Slot ( BOOL (*)( BOOL ), BOOL Slot );

BOOL	OW_0_Reset( void )
{
	return	_SVCCall_Reset( _OW_0_Reset );
}

BOOL	OW_0_Slot( BOOL bitOut )
{
	return	_SVCCall_Slot ( _OW_0_Slot, bitOut );
}

BOOL	OW_1_Reset( void )
{
	return	_SVCCall_Reset( _OW_1_Reset );
}

BOOL	OW_1_Slot( BOOL bitOut )
{
	return	_SVCCall_Slot ( _OW_1_Slot, bitOut );
}

/********************************** ����˵�� ***********************************
*	����I2C����
*******************************************************************************/
#define	Pin_I2C_SCL_In		PinBB( GPIOB->IDR, 6U )
#define	Pin_I2C_SCL_Out		PinBB( GPIOB->ODR, 6U )
#define	Pin_I2C_SDA_In		PinBB( GPIOB->IDR, 7U )
#define	Pin_I2C_SDA_Out		PinBB( GPIOB->ODR, 7U )

BOOL	bus_i2c_start ( uint8_t Address8Bit, enum I2C_DirectSet DirectSet )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
 	MODIFY_REG( GPIOB->CRL, 0xFF000000u, 0x77000000u );

	//	Verify bus available.
	Pin_I2C_SDA_Out = 1;
	Pin_I2C_SCL_Out = 1;
	delay_us( 10 );
	if( ! Pin_I2C_SDA_In ){	 return  FALSE; }
	if( ! Pin_I2C_SCL_In ){	 return  FALSE; }

	Pin_I2C_SDA_Out = 0;
	delay_us( 1 );
	Pin_I2C_SCL_Out = 0;

	if ( I2C_Write == DirectSet )
	{
		return	bus_i2c_shout( Address8Bit & 0xFEu );
	}
	else
	{
		return	bus_i2c_shout( Address8Bit | 0x01u );
	}
}

void	bus_i2c_stop ( void )
{
	Pin_I2C_SDA_Out = 0;
	delay_us( 1 );
	Pin_I2C_SCL_Out = 1;
	delay_us( 1 );
	Pin_I2C_SDA_Out = 1;
	delay_us( 1 );
}

BOOL	bus_i2c_shout ( uint8_t cOutByte )
{
	BOOL	AcknowlegeState;
	uint8_t	i;

	for( i = 8U; i != 0U; --i )
	{
		if ( cOutByte & 0x80u )
		{
			Pin_I2C_SDA_Out = 1;
		}
		else
		{
			Pin_I2C_SDA_Out = 0;
		}
		cOutByte <<= 1;

		delay_us( 1 );
		Pin_I2C_SCL_Out = 1;

		delay_us( 1 );
		Pin_I2C_SCL_Out = 0;
	}
	
	Pin_I2C_SDA_Out = 1;
	delay_us( 1 );
	Pin_I2C_SCL_Out = 1;
	delay_us( 1 );
 	AcknowlegeState	= Pin_I2C_SDA_In;
	Pin_I2C_SCL_Out = 0;

	if ( I2C_ACK != AcknowlegeState )
	{
		return	FALSE;
	}
	else
	{
		return	TRUE;
	}
}

uint8_t	bus_i2c_shin( enum I2C_AcknowlegeSet AcknowlegeSet )
{
	uint8_t		cInByte = 0U;
	uint8_t		i;

	Pin_I2C_SDA_Out = 1;		// make SDA an input
	for( i = 8U; i != 0U; --i )
	{
		delay_us( 1 );
		Pin_I2C_SCL_Out = 1;

		delay_us( 1 );
		cInByte <<= 1;
		if ( Pin_I2C_SDA_In )
		{
			cInByte |= 0x01u;
		}
		else 
		{
			cInByte &= 0xFEu;
		}

		Pin_I2C_SCL_Out = 0;
	}

	if ( I2C_ACK == AcknowlegeSet )
	{
		Pin_I2C_SDA_Out = 0;
	}
	else
	{
		Pin_I2C_SDA_Out = 1;
	}
	delay_us( 1 );
	Pin_I2C_SCL_Out = 1;
	delay_us( 1 );
	Pin_I2C_SCL_Out = 0;

	return	cInByte;
}

// /********************************** ����˵�� ***********************************
// *	���� SPI ����( SPI1x )
// *******************************************************************************/
// #ifdef	SimulationSPI

// #define	Pin_SPI1xSCK		PinBB( GPIOB->ODR,  3U )
// #define	Pin_SPI1xMISO		PinBB( GPIOB->IDR,  4U )
// #define	Pin_SPI1xMOSI		PinBB( GPIOB->ODR,  5U )

// uint8_t bus_SPI1xShift( uint8_t OutByte )
// {
// 	uint8_t i;
// 	
// 	for ( i = 8u; i != 0u; --i )
// 	{
// 		delay_us( 1 );
// 		if ( OutByte & 0x80u )
// 		{
// 			Pin_SPI1xMOSI = 1;
// 		}
// 		else
// 		{
// 			Pin_SPI1xMOSI = 0;
// 		}

// 		delay_us( 1 );
// 		Pin_SPI1xSCK = 0;

// 		delay_us( 1 );

// 		OutByte <<= 1;
// 		if ( Pin_SPI1xMISO )
// 		{
// 			OutByte |= 0x01u;
// 		}
// 		else
// 		{
// 			OutByte &= 0xFEu;
// 		}

// 		delay_us( 1 );
// 		Pin_SPI1xSCK = 1;
// 	}
// 	
// 	return	OutByte;
// }


// void	bus_SPI1xPortInit( void )
// {
// 	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
//  	MODIFY_REG( AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE );

//  	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
// 	Pin_SPI1xSCK = 1;
//  	MODIFY_REG( GPIOB->CRL, 0x00FFF000u, 0x00343000u );
// }

// #else

// uint8_t bus_SPI1xShift( uint8_t IOByte )
// {
// 	SPI_TypeDef * SPIx = SPI1;

// 	while ( ! READ_BIT( SPIx->SR, SPI_SR_TXE ));	// Wait if TXE cleared, Tx FIFO is empty.
// 	SPIx->DR = IOByte;
// 	while ( ! READ_BIT( SPIx->SR, SPI_SR_RXNE ));	// Wait if RXNE cleared, Rx FIFO is empty.
// 	IOByte = SPIx->DR;

// 	return	IOByte;
// }

// void	bus_SPI1xPortInit( void )
// {
// 	SPI_TypeDef * SPIx = SPI1;

// 	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_SPI1EN );

// 	SPIx->CR1	= SPI_CR1_MSTR
// 				| SPI_CR1_CPHA
// 				| SPI_CR1_CPOL
// 				| SPI_CR1_SSM
// 				| SPI_CR1_SSI
// 				| SPI_CR1_BR_2 | SPI_CR1_BR_1 | SPI_CR1_BR_0
// 				;
// 	SPIx->CR2   = 0;
// 	SET_BIT( SPIx->CR1, SPI_CR1_SPE );
// 	
// 	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
// 	CLEAR_BIT(  AFIO->MAPR, AFIO_MAPR_SWJ_CFG ); 
// 	SET_BIT(    AFIO->MAPR, AFIO_MAPR_SPI1_REMAP );
// 	MODIFY_REG( AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_JTAGDISABLE );
// 	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
//  	MODIFY_REG( GPIOB->CRL, 0x00FFF000u, 0x00B4B000u );
// }

// #endif

/********************************** ����˵�� ***********************************
*	���� USART1��ʵ��RS232����Ҫ����ʵ����΢�����λ������ͨѶ
*******************************************************************************/
void	UART1_Send( uint8_t OutByte )
{
	USART_TypeDef * USARTx = USART1;

	while ( ! ( READ_BIT( USARTx->SR, USART_SR_TXE )))
	{
		;
	}
	USARTx->DR = OutByte;
}

void	UART1_Init ( void )
{
	USART_TypeDef * USARTx = USART1;
	
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_USART1EN );

	// USART1 configured as follow:
	// 	- BaudRate = 9600 baud  
	// 	- Word Length = 8 Bits
	// 	- One Stop Bit
	// 	- No parity
	// 	- Hardware flow control disabled (RTS and CTS signals)
	// 	- Receive disable and transmit enabled
	USARTx->BRR = SystemCoreClock / 9600u;	/* 9600 bps							*/
	USARTx->CR1 = 0x0000u;					/* 1 start bit, 8 data bits         */
	USARTx->CR2 = 0x0000u;					/* 1 stop bit                       */
	USARTx->CR3 = 0x0000u; 					/* no flow control                  */
	SET_BIT( USARTx->CR1, USART_CR1_TE );	/* enable TX 						*/
	SET_BIT( USARTx->CR1, USART_CR1_UE );	/* Enable USARTx					*/

	/* Configure USART1 Rx (PA10) as input floating */
	/* Configure USART1 Tx (PA9) as alternate function push-pull */
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	MODIFY_REG( GPIOA->CRH, 0x00000FF0u, 0x000004B0u );
}


/********************************** ����˵�� ***********************************
*	���� BKP ����
*******************************************************************************/
void	BackupRegister_Write( uint16_t BKP_DR, uint16_t usData )
{
	uint32_t tmp = (uint32_t)BKP_BASE + BKP_DR;
	SET_BIT( RCC->APB1ENR, RCC_APB1ENR_BKPEN );	//	�������ʱ�������
	*(__IO uint16_t *) tmp = usData;
}

uint16_t	BackupRegister_Read( uint16_t BKP_DR )
{
	uint32_t tmp = (uint32_t)BKP_BASE + BKP_DR;
	SET_BIT( RCC->APB1ENR, RCC_APB1ENR_BKPEN );	//	�������ʱ�������
	return	(*(__IO uint16_t *) tmp );
}


/********************************** ����˵�� ***********************************
*	ʹ�� TIM1 ���� ETR ���壬��������ת��Ȧ����
*******************************************************************************/
// static	void	TIM1_Init( void )
// {
// 	TIM_TypeDef	* TIMx = TIM1;

// 	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_TIM1EN );

// 	TIMx->SMCR	= TIM_SMCR_ETP 
// 				| TIM_SMCR_ETF_3
// 				| TIM_SMCR_ETPS_0
// 				;
// 	
// 	/* Enable the TIM Counter */
// 	SET_BIT( TIMx->SMCR, TIM_SMCR_ECE );
//  	SET_BIT( TIMx->CR1, TIM_CR1_CEN );

// 	// 	//	ʹ�� TI2
// 	//  	CLEAR_BIT( TIMx->CCER,  TIM_CCER_CC2E );
// 	//  	MODIFY_REG( TIMx->CCMR1, TIM_CCMR1_CC2S | TIM_CCMR1_IC2F, TIM_CCMR1_CC2S_0 );
// 	//  	SET_BIT(   TIMx->CCER,  TIM_CCER_CC2P | TIM_CCER_CC2E );

// 	// 	/* Select the Trigger source */
// 	// 	MODIFY_REG( TIMx->SMCR, TIM_SMCR_TS, TIM_SMCR_TS_2 | TIM_SMCR_TS_1 );	//	TIM_TIxExternalCLK1Source_TI2
// 	// 	/* Select the External clock mode1 */
// 	// 	SET_BIT( TIMx->SMCR, TIM_SMCR_SMS );	//	TIM_SlaveMode_External1
// }

/********************************** ����˵�� ***********************************
*	ʹ�� TIM2 ʵ�� ֱ��PWM �������
*******************************************************************************/
#define	_TIM2_PWM_Period	2400u

void	TIM2_Init( void )
{
	TIM_TypeDef	* TIMx = TIM2;

	SET_BIT( RCC->APB1ENR, RCC_APB1ENR_TIM2EN );
	//	TIMx ʱ����ʼ��: ����ʱ��Ƶ��24MHz��PWM�ֱ���2400��PWMƵ��10KHz��
	TIMx->CR1 = 0u;
	TIMx->PSC = 0u;
	TIMx->ARR = ( _TIM2_PWM_Period - 1u );
	TIMx->EGR = TIM_EGR_UG;

	//	���ù�������ʱȫ���ر����
	TIMx->CCER = 0u;
	TIMx->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1
				| TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
	TIMx->CCMR2 = 0u;
	TIMx->CCR1 = 0u;
	TIMx->CCR2 = 0u;
	SET_BIT( TIMx->CCER, TIM_CCER_CC2E );	//	T2CH2 ��ֱ���������
	//	TIMx ʹ��
	SET_BIT( TIMx->CR1, TIM_CR1_CEN );

	//	���ùܽ�: PA.0 <-> T2CH1/ETR, PA.1 <-> T2CH2
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	MODIFY_REG( GPIOA->CRL, 0x000000FFu, 0x000000BBu );
}

void	TIM2_OutCmd( BOOL NewState )
{
	TIM_TypeDef * TIMx = TIM2;

	if ( NewState )
	{
		SET_BIT( TIMx->CCER, TIM_CCER_CC2E );
	}
	else
	{
		CLEAR_BIT( TIMx->CCER, TIM_CCER_CC2E );
	}
}

void	TIM2_SetOutput( uint16_t OutValue )
{
	TIM_TypeDef * TIMx = TIM2;

	TIMx->CCR2 = (uint32_t)OutValue * _TIM2_PWM_Period / PWM_Output_Max;
}

/********************************** ����˵�� ***********************************
*	ʹ�� TIM3 ʵ�� PWM���ܣ������Χ�̶���0 ~ PWM_Output_Max(ͨ����27648u)
*******************************************************************************/
#define	_TIM3_PWM_Period	2400u

static	uint16_t	_TIM3_PWM_Regular( uint16_t OutValue )
{
	return	(uint32_t)OutValue * _TIM3_PWM_Period / PWM_Output_Max;
}

static	void	TIM3_Init( void )
{
	TIM_TypeDef	* TIMx = TIM3;

	SET_BIT( RCC->APB1ENR, RCC_APB1ENR_TIM3EN );
	//	TIMx ʱ����ʼ��: ����ʱ��Ƶ��24MHz��PWM�ֱ���2400, PWMƵ��10KHz
	TIMx->CR1  = 0u;
	TIMx->PSC  = 0u;
	TIMx->ARR  = ( _TIM3_PWM_Period - 1u );
	TIMx->EGR  = TIM_EGR_UG;

	//	���ù�������ʱȫ���ر����
	TIMx->CCER = 0u;
	TIMx->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1
				| TIM_CCMR1_OC2M_2 | TIM_CCMR1_OC2M_1;
	TIMx->CCMR2 = TIM_CCMR2_OC3M_2 | TIM_CCMR2_OC3M_1
				| TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;
	TIMx->CCR1 = 0u;
	TIMx->CCR2 = 0u;
	TIMx->CCR3 = 0u;
	TIMx->CCR4 = 0u;

//	SET_BIT( TIMx->CCER, TIM_CCER_CC3E );	//	T3CH3 �Ӵ���������
//	SET_BIT( TIMx->CCER, TIM_CCER_CC4E );	//	T3CH4 �����������
	//	TIMx ʹ��
	SET_BIT( TIMx->CR1, TIM_CR1_CEN );

	//	��������ܽ�
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
 	MODIFY_REG( AFIO->MAPR, AFIO_MAPR_TIM3_REMAP, AFIO_MAPR_TIM3_REMAP_FULLREMAP );
//	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
//	MODIFY_REG( GPIOC->CRL, 0x0F000000u, 0x0B000000u );		//	PC.6, T3CH1
//	MODIFY_REG( GPIOC->CRL, 0xF0000000u, 0xB0000000u );		//	PC.7, T3CH2
// 	MODIFY_REG( GPIOC->CRH, 0x0000000Fu, 0x0000000Bu );		//	PC.8, T3CH3
// 	MODIFY_REG( GPIOC->CRH, 0x000000F0u, 0x000000B0u );		//	PC.9, T3CH4
}

void	PWM1_SetOutput( uint16_t OutValue )
{
	TIM_TypeDef * TIMx = TIM3;
	
	TIMx->CCR1 = _TIM3_PWM_Regular( OutValue );
}

void	PWM2_SetOutput( uint16_t OutValue )
{
	TIM_TypeDef * TIMx = TIM3;
	
	TIMx->CCR2 = _TIM3_PWM_Regular( OutValue );
}

void	PWM3_SetOutput( uint16_t OutValue )
{
	TIM_TypeDef * TIMx = TIM3;
	
	TIMx->CCR3 = _TIM3_PWM_Regular( OutValue );
}

void	PWM4_SetOutput( uint16_t OutValue )
{
	TIM_TypeDef * TIMx = TIM3;
	
	TIMx->CCR4 = _TIM3_PWM_Regular( OutValue );
}

/********************************** ����˵�� ***********************************
*	���� GPIO  
*******************************************************************************/
void	MeasureBattery_OutCmd( BOOL NewState )
{	//	PC8 �ߵ�ƽ��Ч
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	GPIOC->BSRR = NewState ? GPIO_BSRR_BS8 : GPIO_BSRR_BR8;
 	MODIFY_REG( GPIOC->CRH, 0x0000000Fu, 0x00000003u );
}
void	AIRLightOutCmd( BOOL NewState )
{	//	PC9 �ߵ�ƽ��Ч
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	GPIOC->BSRR = NewState ? GPIO_BSRR_BS9 : GPIO_BSRR_BR9;
 	MODIFY_REG( GPIOC->CRH, 0x000000F0u, 0x00000030u );
}

void	Speaker_OutCmd( BOOL NewState )
{	//	PB.2(BOOT1), �ϵ縴λʱ�ⲿ������GND.
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
	GPIOB->BSRR = NewState ? GPIO_BSRR_BS2 : GPIO_BSRR_BR2;
 	MODIFY_REG( GPIOB->CRL, 0x00000F00u, 0x00000300u );
}

void	beep( void )
{
// 	Speaker_OutCmd( TRUE );
	delay( 200u );
	Speaker_OutCmd( FALSE );
}

void	tick( void )
{
// 	Speaker_OutCmd( TRUE );
	delay( 20u );
	Speaker_OutCmd( FALSE );
}

void	PumpAIR_OutCmd( BOOL NewState )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	GPIOB->BSRR = NewState ? GPIO_BSRR_BS8 : GPIO_BSRR_BR8;
	MODIFY_REG( GPIOB->CRH, 0x0000000Fu, 0x00000003u );
}

BOOL	PumpAIR_GetOutState( void )
{
	return	READ_BIT( GPIOB->IDR, GPIO_IDR_IDR8 ) ? TRUE : FALSE;
}

BOOL	UART1_RX_Pin_State( void )
{
	//	PA.10 for USART1 RX.
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	MODIFY_REG( GPIOA->CRH, 0x00000F00u, 0x00000400u );
	
	return	READ_BIT( GPIOA->IDR, GPIO_IDR_IDR10 ) ? TRUE : FALSE;
}

/********************************** ����˵�� ***********************************
*	��ȡ JTAG �ϵ��������ߣ�����������ߣ�����1�����򷵻�0��
*******************************************************************************/
uint8_t	get_Jumper( void )
{
	uint8_t		JumperState;
	uint16_t	portState, prevState;
	uint8_t 	i, iRetryMax = 100u;

	//	PA.13 - JTMS/SWDIO, Ĭ���ڲ���������������ʱ��VCC����
	//	PA.14 - JTCK/SWCLK, Ĭ���ڲ���������������ʱ��GND����

	//	��ʱ�رյ��Թ���
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
	MODIFY_REG( AFIO->MAPR, AFIO_MAPR_SWJ_CFG, AFIO_MAPR_SWJ_CFG_DISABLE );		/*!< JTAG-DP Disabled and SW-DP Disabled */
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	GPIOA->BSRR = GPIO_BSRR_BR13;	//	��Ϊ�ڲ�����
	GPIOA->BSRR = GPIO_BSRR_BS14;	//	��Ϊ�ڲ�����
	MODIFY_REG( GPIOA->CRH, 0x0FF00000u, 0x08800000u );

	//	��ȡ����״̬������30ms״̬���䣬��ȷ�϶�ȡ�����Ч��
	portState = 0u;
	i = 0u;
	prevState = portState;
	do {
		delay_us( 300u );
		portState = READ_BIT( GPIOA->IDR, GPIO_IDR_IDR13 | GPIO_IDR_IDR14 );
		if ( prevState != portState )
		{
			i = 0u;
			prevState = portState;
		}
	} while ( ++i < iRetryMax );
	
	JumperState = ((( portState >> 13 ) & 0x03u ) ^ 0x02u );	//	�������߽��Ϊ0�������߽��Ϊ1��

	//	���´򿪵��Թ���
	GPIOA->BSRR = GPIO_BSRR_BS13;		//	�Ļ�Ĭ������
	GPIOA->BSRR = GPIO_BSRR_BR14;		//	�Ļ�Ĭ������
	CLEAR_BIT( AFIO->MAPR, AFIO_MAPR_SWJ_CFG );
	SET_BIT(   AFIO->MAPR, AFIO_MAPR_SWJ_CFG_JTAGDISABLE );	/*!< JTAG-DP Disabled and SW-DP Enabled */

	return	JumperState;
}

/********************************** ����˵�� ***********************************
 *��Brief:	Enters SLEEP mode.
*******************************************************************************/
void	NVIC_Sleep( void )
{
	CLEAR_BIT( SCB->SCR, SCB_SCR_SLEEPONEXIT_Msk | SCB_SCR_SLEEPDEEP_Msk );
	__WFE();
}

/********************************** ����˵�� ***********************************
*	NVIC ����
*******************************************************************************/
void	NVIC_IRQ_Cmd( IRQn_Type IRQn, BOOL NewState )
{	//	ʹ�� SVC ���� NVIC��ʵ���жϵĿ��ؿ��ơ�
	__svc_indirect(0) void _SVC_Call( void (*)( IRQn_Type IRQn ), IRQn_Type IRQn );

	if ( NewState )
	{
		_SVC_Call( NVIC_EnableIRQ, IRQn );
	}
	else
	{
		_SVC_Call( NVIC_DisableIRQ, IRQn );
	}
}

static	void	NVIC_Init( void )
{
	// 	��7.4 ��ռ���ȼ��������ȼ��ı��λ�������λ�õĹ�ϵ
	// 	����λ��    ������ռ���ȼ���λ��    ���������ȼ���λ��
	// 		0            [7:1]                 [0:0]
	// 		1            [7:2]                 [1:0]
	// 		2            [7:3]                 [2:0]
	// 		3            [7:4]                 [3:0]
	// 		4            [7:5]                 [4:0]
	// 		5            [7:6]                 [5:0]
	// 		6            [7:7]                 [6:0]
	// 		7            ��                    [7:0]������λ��
	NVIC_SetPriorityGrouping( 7 );	//	��ֹ�ж���ռ����û���ж�Ƕ��

	//	for Keyboard.
	NVIC_EnableIRQ( EXTI0_IRQn );
	NVIC_EnableIRQ( EXTI1_IRQn );
	NVIC_EnableIRQ( EXTI2_IRQn );
	NVIC_EnableIRQ( EXTI3_IRQn );
	NVIC_EnableIRQ( EXTI4_IRQn );
	NVIC_EnableIRQ( EXTI9_5_IRQn );
	//	for CH376
	NVIC_EnableIRQ( EXTI15_10_IRQn );
	//	for MODBUS.
	NVIC_EnableIRQ( TIM4_IRQn );
	NVIC_EnableIRQ( USART3_IRQn );
	
	//	__set_CONTROL( 0x03u );		//	�л���PSP, ��ת�����Ȩģʽ
}

/********************************** ����˵�� ***********************************
*	BIOS ��ʼ��
*******************************************************************************/
void	BIOS_Init( void )
{
	NVIC_Init();		//	����Ȩģʽ��ִ�еͲ����ã�������ɺ�(��ѡ)�л�������Ȩģʽ��
	TIM2_Init();		//	for 100L Motor PWMx1.
	TIM3_Init();		//	for PWMx4��
}


/********************************** ����˵�� ***********************************
 *	for PC-Lint
*******************************************************************************/
//	#ifdef	_lint
//	BOOL	_OW_SVC_Reset( BOOL (*Handler)( void )            ) {  return Handler(      ); }
//	BOOL	_OW_SVC_Slot ( BOOL (*Handler)( BOOL ), BOOL Slot ) {  return Handler( Slot ); }
//	#endif

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/