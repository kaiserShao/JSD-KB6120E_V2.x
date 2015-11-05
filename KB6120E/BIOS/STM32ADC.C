/**************** (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: CLOCK.C
* �� �� ��: ����
* ��  ��  : ��дSTM32F10x��RTCʱ��
* ����޸�: 2014��5��5��
*********************************** �޶���¼ ***********************************
* ��  ��: V2.0
* �޶���: ����
* ˵  ��: see http://www.epochconverter.com
*******************************************************************************/
#include "BSP.H"
#include "Pin.H"
#include "BIOS.H"

/********************************** ����˵�� ***********************************
*	���� STM32 �� ADC ģ��
*******************************************************************************/
volatile	uint16_t	ADCResultBuf[16][5];

static	void	STM32_ADC_Init ( void )
{
	//	DMA1 channel1 configuration
	SET_BIT ( RCC->AHBENR,  RCC_AHBENR_DMA1EN );
	//	DMAģ�����, ��������
	DMA1_Channel1->CCR   = 0u;
	DMA1_Channel1->CCR   = DMA_CCR1_PL | DMA_CCR1_MINC | DMA_CCR1_PSIZE_0 | DMA_CCR1_MSIZE_0 | DMA_CCR1_CIRC;
	//	����DMA�����ַ, �ڴ��ַ, ��������
	DMA1_Channel1->CPAR  = ( uint32_t ) &ADC1->DR;
	DMA1_Channel1->CMAR  = ( uint32_t ) ADCResultBuf;
	DMA1_Channel1->CNDTR = ( sizeof ( ADCResultBuf ) / sizeof ( ADCResultBuf[0][0] ) );
	SET_BIT ( DMA1_Channel1->CCR, DMA_CCR1_EN );			//	DMAͨ��ʹ��

	SET_BIT ( RCC->APB2ENR, RCC_APB2ENR_ADC1EN );
	MODIFY_REG ( RCC->CFGR , RCC_CFGR_ADCPRE, RCC_CFGR_ADCPRE_DIV6 );

	// ���ÿ��ƼĴ���
	ADC1->CR1 = ADC_CR1_SCAN;
	ADC1->CR2 = ADC_CR2_ADON | ADC_CR2_TSVREFE | ADC_CR2_CONT | ADC_CR2_DMA;

	//	����ͨ������ʱ�䣨ͨ����Ŵ�0��ʼ��
	#define	ADC1_SMP_1p5_C		0	// 1.5 cycles
	#define	ADC1_SMP_7p5_C		1	// 7.5 cycles
	#define	ADC1_SMP_13p5_C		2	// 13.5 cycles
	#define	ADC1_SMP_28p5_C		3	// 28.5 cycles
	#define	ADC1_SMP_41p5_C		4	// 41.5 cycles
	#define	ADC1_SMP_55p5_C		5	// 55.5 cycles
	#define	ADC1_SMP_71p5_C		6	// 71.5 cycles
	#define	ADC1_SMP_239p5_C	7	// 239.5 cycles

	#define	ADC1_SMPR1_BASE		10	// R1 ��ʼ���
	#define	ADC1_SMPR2_BASE		0	// R2 ��ʼ���
	#define	ADC1_SMPR_WIDTH		3	// ÿ������3λ

    // ...............  ������Ӧ����ʱ�� ...........................................................
	ADC1->SMPR2 =	( ( ADC1_SMP_239p5_C ) << ( ADC1_SMPR_WIDTH * (  0 - ADC1_SMPR2_BASE ) ) )	//	ͨ��0
				|	( ( ADC1_SMP_239p5_C ) << ( ADC1_SMPR_WIDTH * (  1 - ADC1_SMPR2_BASE ) ) )	//	ͨ��1
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  2 - ADC1_SMPR2_BASE ) ) )	//	ͨ��2
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  3 - ADC1_SMPR2_BASE ) ) )	//	ͨ��3
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  4 - ADC1_SMPR2_BASE ) ) )	//	ͨ��4
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  5 - ADC1_SMPR2_BASE ) ) )	//	ͨ��5
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  6 - ADC1_SMPR2_BASE ) ) )	//	ͨ��6
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  7 - ADC1_SMPR2_BASE ) ) )	//	ͨ��7
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * (  8 - ADC1_SMPR2_BASE ) ) )	//	ͨ��8
				|	( ( ADC1_SMP_239p5_C ) << ( ADC1_SMPR_WIDTH * (  9 - ADC1_SMPR2_BASE ) ) )	//	ͨ��9
                ;
	ADC1->SMPR1 =	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * ( 10 - ADC1_SMPR1_BASE ) ) )	//	ͨ��10
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * ( 11 - ADC1_SMPR1_BASE ) ) )	//	ͨ��11
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * ( 12 - ADC1_SMPR1_BASE ) ) )	//	ͨ��12
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * ( 13 - ADC1_SMPR1_BASE ) ) )	//	ͨ��13
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * ( 14 - ADC1_SMPR1_BASE ) ) )	//	ͨ��14
				|	( ( ADC1_SMP_1p5_C   ) << ( ADC1_SMPR_WIDTH * ( 15 - ADC1_SMPR1_BASE ) ) )	//	ͨ��15
				|	( ( ADC1_SMP_41p5_C  ) << ( ADC1_SMPR_WIDTH * ( 16 - ADC1_SMPR1_BASE ) ) )	//	ͨ��16(�ڲ��¶�)
				|	( ( ADC1_SMP_41p5_C  ) << ( ADC1_SMPR_WIDTH * ( 17 - ADC1_SMPR1_BASE ) ) )	//	ͨ��17(�ڲ���׼)
				;

	//	���ù���ת�����У����б�Ŵ�1��ʼ��
	#define	ADC1_SQR1_BASE		13	// R1 ��ʼ���
	#define	ADC1_SQR2_BASE		7	// R2 ��ʼ���
	#define	ADC1_SQR3_BASE		1	// R3 ��ʼ���
	#define	ADC1_SQR_WIDTH		5	// ÿ������5λ

    // ............����ͨ���ţ����һ���������г���.................................................
	ADC1->SQR3  =	( ( 16 ) << ( ADC1_SQR_WIDTH * (  1 - ADC1_SQR3_BASE ) ) )	//	����1
				|	( ( 17 ) << ( ADC1_SQR_WIDTH * (  2 - ADC1_SQR3_BASE ) ) )	//	����2
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * (  3 - ADC1_SQR3_BASE ) ) )	//	����3
				|	( ( 1  ) << ( ADC1_SQR_WIDTH * (  4 - ADC1_SQR3_BASE ) ) )	//	����4
				|	( ( 9  ) << ( ADC1_SQR_WIDTH * (  5 - ADC1_SQR3_BASE ) ) )	//	����5
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * (  6 - ADC1_SQR3_BASE ) ) )	//	����6
				;
	ADC1->SQR2  =	( ( 0  ) << ( ADC1_SQR_WIDTH * (  7 - ADC1_SQR2_BASE ) ) )	//	����7
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * (  8 - ADC1_SQR2_BASE ) ) )	//	����8
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * (  9 - ADC1_SQR2_BASE ) ) )	//	����9
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 10 - ADC1_SQR2_BASE ) ) )	//	����10
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 11 - ADC1_SQR2_BASE ) ) )	//	����11
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 12 - ADC1_SQR2_BASE ) ) )	//	����12
				;
	ADC1->SQR1  =	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 13 - ADC1_SQR1_BASE ) ) )	//	����13
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 14 - ADC1_SQR1_BASE ) ) )	//	����14
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 15 - ADC1_SQR1_BASE ) ) )	//	����15
				|	( ( 0  ) << ( ADC1_SQR_WIDTH * ( 16 - ADC1_SQR1_BASE ) ) )	//	����16
				|( ( 5 - 1 ) << ( ADC1_SQR_WIDTH * ( 17 - ADC1_SQR1_BASE ) ) )	//	����(0��ʾ1��)
                ;

	__NOP();	//	delay_us ( 1u );
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	__NOP();
	SET_BIT ( ADC1->CR2, ADC_CR2_ADON );						//	Enable ADC1
	SET_BIT ( ADC1->CR2, ADC_CR2_RSTCAL );						//	Reset calibration register
	while ( READ_BIT ( ADC1->CR2, ADC_CR2_RSTCAL ) ) {}			//	Check the end of ADC1 reset calibration register
	SET_BIT ( ADC1->CR2, ADC_CR2_CAL );							//	Start ADC1 calibration
	while ( READ_BIT ( ADC1->CR2, ADC_CR2_CAL ) ) {}			//	Check the end of ADC1 calibration
	SET_BIT ( ADC1->CR2, ADC_CR2_SWSTART | ADC_CR2_EXTTRIG );  	//	Start ADC1 Software Conversion

	//	 ������Ӧ��AD����ܽ�Ϊģ������״̬
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPAEN );
	MODIFY_REG( GPIOA->CRL, 0x0000000Fu, 0x00000000u );	//	PA0 ģ������ģʽ
	MODIFY_REG( GPIOA->CRL, 0x000000F0u, 0x00000000u );	//	PA1 ģ������ģʽ
	SET_BIT( RCC->APB2ENR, RCC_APB2ENR_IOPBEN );
	MODIFY_REG( GPIOB->CRL, 0x000000F0u, 0x00000000u );	//	PB1 ģ������ģʽ
}

static	uint16_t	STM32_ADC_fetch ( uint8_t Channel )
{
	const uint8_t len = sizeof ( ADCResultBuf ) / sizeof ( ADCResultBuf[0] );
	uint8_t		i;
	uint32_t	sum = 0u;

	for ( i = 0u; i < len; ++ i )
	{
		sum += ADCResultBuf[i][Channel];
	}
	return	( uint16_t ) ( sum / len );
}



void	ADC12_Init( void )
{
	STM32_ADC_Init();
}

uint16_t	ADC12_Readout( enum enumADC12_Channel ADC_Channel )
{
	return	STM32_ADC_fetch( ADC_Channel );
}

/********  (C) COPYRIGHT 2014 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/