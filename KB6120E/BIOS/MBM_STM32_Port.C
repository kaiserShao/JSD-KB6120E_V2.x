/**************** (C) COPYRIGHT 2013 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: MBM_STM32_Port.C
* �� �� ��: ����
* ��  ��  : KB6120E оƬ�˿ڷ���(MODBUS����)
* ����޸�: 2013��12��27��
*********************************** �޶���¼ ***********************************
* ��  ��: 
* �޶���: 
*******************************************************************************/
#include "BSP.H"
#include "Pin.H"
#include "BIOS.H"
//	#include "MBM_Port.H"
extern	void	vMBM_RTU_T35_ISR( void );
extern	void	vMBM_RTU_Send_ISR( void );
extern	void	vMBM_RTU_Receive_ISR( void );

/********************************** ����˵�� ***********************************
*  ���� RS485 ���ߵķ���
*******************************************************************************/
//	void	RS485_Direct_Output( void )
//	{	//	DE <--> PC.12���ߵ�ƽ��������
//		SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
//		GPIOC->BSRR = ( 1 << 12 );
//		MODIFY_REG( GPIOC->CRH, 0x000F0000u, 0x00030000u );

//	}

//	void	RS485_Direct_Input( void )
//	{	//	RE <--> PC.12���͵�ƽ��������
//		SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
//		GPIOC->BRR  = ( 1 << 12 );
//		MODIFY_REG( GPIOC->CRH, 0x000F0000u, 0x00030000u );
//	}

void	RS485_Direct_OE ( BOOL NewState )
{	//	RE <--> PC.12���͵�ƽ��������
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
	GPIOC->BSRR  = NewState ? ( 1 << 12 ) : ( 1 << ( 12 + 16 ));
	MODIFY_REG( GPIOC->CRH, 0x000F0000u, 0x00030000u );
}

/********************************** ����˵�� ***********************************
*  ���ʶ�ʱ��
*******************************************************************************/
void	TIM4_Init( uint32_t u_sec )
{
	TIM_TypeDef * TIMx = TIM4;
	//	ģ��ʹ��
	SET_BIT( RCC->APB1ENR, RCC_APB1ENR_TIM4EN );
	//	ʱ����ʼ��
	TIMx->CR1  = 0u;									/* Counter Up, Division: 1 */
	TIMx->PSC  = ( SystemCoreClock / 1000000u ) - 1u;	/* Set the Prescaler value */
	TIMx->ARR  = u_sec;									/* Set the Autoreload value */
	TIMx->EGR  = TIM_EGR_UG;           					//	���ɸ����¼����������� PSC
	TIMx->DIER = 0u;
}

void	TIM4_Cmd( BOOL NewState )
{
	TIM_TypeDef * TIMx = TIM4;

	if ( NewState )
	{
		/* Enable the timer with the timeout passed to xMBPortTimersInit( ) */
		CLEAR_BIT( TIMx->SR, TIM_SR_UIF );
		SET_BIT( TIMx->DIER, TIM_DIER_UIE );
		TIMx->CNT = 0u;
		SET_BIT( TIMx->CR1, TIM_CR1_CEN );
	}
	else
	{
		/* Disable any pending timers. */
		CLEAR_BIT( TIMx->CR1, TIM_CR1_CEN );
		TIMx->CNT = 0u;
		CLEAR_BIT( TIMx->DIER, TIM_DIER_UIE );
		CLEAR_BIT( TIMx->SR, TIM_SR_UIF );
	}
}

/********************************** ����˵�� ***********************************
*  ���ʴ���
*******************************************************************************/
void	USART3_Init( uint32_t ulBaudRate )
{
	USART_TypeDef * USARTx = USART3;
	
	SET_BIT( RCC->APB1ENR, RCC_APB1ENR_USART3EN );

	USARTx->CR1 = 0u;
	USARTx->CR2 = 0u;
	USARTx->CR3 = USART_CR3_DMAT;
	USARTx->CR1 = USART_CR1_RE | USART_CR1_TE;
	USARTx->BRR = ( SystemCoreClock / ulBaudRate );

//	switch ( ucDataBits )
//	{
//	case 7:
//		CLEAR_BIT( USARTx->CR1, USART_CR1_M );
//		break;
//	case 8:
		SET_BIT( USARTx->CR1, USART_CR1_M );
//		break;
//	}

//	switch ( eParity )
//	{
//	case MB_PAR_NONE:		/*!< No parity. */
//		CLEAR_BIT( USARTx->CR1, USART_CR1_PCE );
//		break;
//	case MB_PAR_ODD:    	/*!< Odd parity. */
//		SET_BIT( USARTx->CR1, USART_CR1_PS );
//		SET_BIT( USARTx->CR1, USART_CR1_PCE );
//		break;		
//	case MB_PAR_EVEN:   	/*!< Even parity. */
		CLEAR_BIT( USARTx->CR1, USART_CR1_PS );
		SET_BIT( USARTx->CR1, USART_CR1_PCE );
//		break;		
//	}

	SET_BIT( USARTx->CR1, USART_CR1_UE );		// Enable USARTx
}

void	USART3_PutByte( uint8_t cOutByte )
{
	USART3->DR = cOutByte;
}

uint8_t	USART3_GetByte( void )
{
	return	USART3->DR;
}

void	USART3_PortInit ( void )
{
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_AFIOEN );
	MODIFY_REG( AFIO->MAPR, AFIO_MAPR_USART3_REMAP, AFIO_MAPR_USART3_REMAP_PARTIALREMAP );

	//	PC.11 USART3 Rx, 4��(0100B)��������ģʽ(��λ���״̬)
	//	PC.10 USART3 Tx, B��(1011B)���ù����������ģʽ50MHz
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPCEN );
 	MODIFY_REG( GPIOC->CRH, 0x0000FF00u, 0x00004B00u );
}


void	USART3_RX_Cmd( BOOL NewState )
{
	USART_TypeDef * USARTx = USART3;

	if ( NewState )
	{
		SET_BIT  ( USARTx->CR1, USART_CR1_RXNEIE );
	}
	else
	{
		CLEAR_BIT( USARTx->CR1, USART_CR1_RXNEIE );
	}
}

void	USART3_TX_Cmd( BOOL NewState )
{
	USART_TypeDef * USARTx = USART3;

	if ( NewState )
	{
		SET_BIT  ( USARTx->CR1, USART_CR1_TCIE );
	}
	else
	{
		CLEAR_BIT( USARTx->CR1, USART_CR1_TCIE );
	}
}


/********************************** ����˵�� ***********************************
*  ��ʱ���ж�
*******************************************************************************/
__irq	void	TIM4_IRQHandler( void )
{
	TIM_TypeDef * TIMx = TIM4;

	if ( READ_BIT( TIMx->SR, TIM_SR_UIF ))
	{
		/* Clear TIM4 Capture Compare1 interrupt pending bit*/
		CLEAR_BIT( TIMx->SR, TIM_SR_UIF );
		vMBM_RTU_T35_ISR();
	}
}

/********************************** ����˵�� ***********************************
*  �����ж�
*******************************************************************************/
__irq	void	USART3_IRQHandler( void )
{
	USART_TypeDef * USARTx = USART3;
	
	if ( READ_BIT( USARTx->CR1, USART_CR1_RXNEIE ))
	{
		if ( READ_BIT( USARTx->SR, USART_SR_RXNE ))
		{
			//	�������ݴ���
			vMBM_RTU_Receive_ISR( );
		}
	}

//	if ( READ_BIT( USARTx->CR1, USART_CR1_TXEIE ))
// 	{
// 		if ( READ_BIT( USARTx->SR, USART_SR_TXE ))
// 		{
	if ( READ_BIT( USARTx->CR1, USART_CR1_TCIE ))
 	{
 		if ( READ_BIT( USARTx->SR, USART_SR_TC ))
 		{
 			//	�������ݴ���
			vMBM_RTU_Send_ISR( );
 		}
 	}
}

/********  (C) COPYRIGHT 2013 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/