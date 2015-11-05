/**************** (C) COPYRIGHT 2012 �ൺ���˴���ӿƼ����޹�˾ ****************
* �� �� ��: CGROM.C
* �� �� ��: ����
* ��  ��  : ��ʾ��ģ����
* ����޸�: 
*********************************** �޶���¼ ***********************************
* ��  ��: 
* �޶���: 
*******************************************************************************/
#include "BSP.H"
#include "BIOS.H"

#define	code	const
#include "DOT_EN.TXT"
#include "DOT_CN.TXT"

/*******************************************************************************
* ��������: DotSeekDBC
* ����˵��: Ӣ�İ���ֿ����
* �������: char_t sc
* �������: None
* �� �� ֵ: CGROM Dot Address
*******************************************************************************/
CGROM	DotSeekDBC ( int8_t sc )
{
    uint8_t uc = (uint8_t)sc;

	if ( uc < 0x20U ){	uc = 0x7FU;	}
	return	& chardot[( uc - 0x20U ) * 0x10U ];
}

/*******************************************************************************
* ��������: DotSeekSBC
* ����˵��: ����ȫ���ֿ����
* �������: char_t cc0, char_t cc1
* �������: None
* �� �� ֵ: CGROM Dot Address
*******************************************************************************/
CGROM	DotSeekSBC ( int8_t cc0, int8_t cc1 )
{
	uint16_t	cc16, cm16;
	uint16_t	mid, low, high;

	cc16 = ((uint8_t)cc0 * 0x100U ) + (uint8_t)cc1;

    low  = 1U;				/*	��[1]��ʼ����	*/
	high = (uint16_t)hzNum - 1U;
	while ( low <= high ){

		mid = ( low + high ) / 2U;	/*	ʹ���۰�����Լӿ�����ٶ�	*/

		cm16 = ( hzIndex[ 2U * mid ] * 0x100U ) + hzIndex[( 2U * mid ) + 1U ];

		if ( cc16 == cm16 ){
			return	& hzdot[ mid * 0x20U ];
		}
		if ( cc16 > cm16 ){
			low  = mid + 1U;
		} else {
			high = mid - 1U;
		}
	}
	return	& hzdot[0];		/*	[0]��ſհ��ַ�	*/
}

#define	code	const
#define	hzIndex	DOT_24_hzIndex
#define	hzdot	DOT_24_hzdot
#define	charIndex	DOT_24_charIndex
#define	chardot	DOT_24_chardot
#include "DOT_24CN.TXT"
#include "DOT_24EN.TXT"
#include "BMP.TXT"
// #define  DOT_24_hzNum  314


CGROM	DotSeekSBC_24x24 ( int8_t cc0, int8_t cc1 )
{
	uint16_t	cc16, cm16;
	uint16_t	mid, low, high;

	cc16 = ((uint8_t)cc0 * 0x100U ) + (uint8_t)cc1;

    low  = 1U;				/*	��[1]��ʼ����	*/
	high = (uint16_t)DOT_24_hzNum - 1U;
	while ( low <= high ){

		mid = ( low + high ) / 2U;	/*	ʹ���۰�����Լӿ�����ٶ�	*/

		cm16 = ( hzIndex[ 2U * mid ] * 0x100U ) + hzIndex[( 2U * mid ) + 1U ];

		if ( cc16 == cm16 ){
			return	& hzdot[ mid * ( 24 * 24 / 8 ) ];
		}
		if ( cc16 > cm16 ){
			low  = mid + 1U;
		} else {
			high = mid - 1U;
		}
	}
	return	& hzdot[0];		/*	[0]��ſհ��ַ�	*/
}

CGROM		DotSeekDBC_12x24( int8_t sc )
{
    uint8_t uc = (uint8_t)sc;

	if ( uc < 0x20U ){	uc = 0x7FU;	}
	return	& chardot[( uc - 0x20U ) * 0x30U ];
}
/********  (C) COPYRIGHT 2010 �ൺ���˴���ӿƼ����޹�˾  **** End Of File ****/