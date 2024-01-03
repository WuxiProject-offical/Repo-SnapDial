
/********************************** (C) COPYRIGHT *******************************
* File Name          : GPIO.C
* Author             : WCH
* Version            : V1.0
* Date               : 2017/01/20
* Description        : CH552 IO ���ýӿں�����GPIO�жϺ���  
*******************************************************************************/

#include "../Lib/CH552.H"                                                          
#include "../Lib/Debug.H"
#include "../Lib/GPIO.H"
#include "stdio.h"

#pragma  NOAREGS

/*******************************************************************************
* Function Name  : Port1Cfg()
* Description    : �˿�1����
* Input          : Mode  0 = �������룬������
                         1 = �����������
                         2 = ��©���������������
                         3 = ��51ģʽ����©������������������ڲ���·���Լ����ɵ͵��ߵĵ�ƽ����		
                   ,UINT8 Pin	(0-7)											 
* Output         : None
* Return         : None
*******************************************************************************/
void Port1Cfg(UINT8 Mode,UINT8 Pin)
{
  switch(Mode){
    case 0:
      P1_MOD_OC = P1_MOD_OC & ~(1<<Pin);
      P1_DIR_PU = P1_DIR_PU &	~(1<<Pin);	
      break;
    case 1:
      P1_MOD_OC = P1_MOD_OC & ~(1<<Pin);
      P1_DIR_PU = P1_DIR_PU |	(1<<Pin);				
      break;		
    case 2:
      P1_MOD_OC = P1_MOD_OC | (1<<Pin);
      P1_DIR_PU = P1_DIR_PU &	~(1<<Pin);				
      break;		
    case 3:
      P1_MOD_OC = P1_MOD_OC | (1<<Pin);
      P1_DIR_PU = P1_DIR_PU |	(1<<Pin);			
      break;
    default:
      break;			
  }
}

/*******************************************************************************
* Function Name  : Port3Cfg()
* Description    : �˿�3����
* Input          : Mode  0 = �������룬������
                         1 = �����������
                         2 = ��©���������������
                         3 = ��51ģʽ����©������������������ڲ���·���Լ����ɵ͵��ߵĵ�ƽ����		
                   ,UINT8 Pin	(0-7)											 
* Output         : None
* Return         : None
*******************************************************************************/
void Port3Cfg(UINT8 Mode,UINT8 Pin)
{
  switch(Mode){
    case 0:
      P3_MOD_OC = P3_MOD_OC & ~(1<<Pin);
      P3_DIR_PU = P3_DIR_PU &	~(1<<Pin);	
      break;
    case 1:
      P3_MOD_OC = P3_MOD_OC & ~(1<<Pin);
      P3_DIR_PU = P3_DIR_PU |	(1<<Pin);				
      break;		
    case 2:
      P3_MOD_OC = P3_MOD_OC | (1<<Pin);
      P3_DIR_PU = P3_DIR_PU &	~(1<<Pin);				
      break;		
    case 3:
      P3_MOD_OC = P3_MOD_OC | (1<<Pin);
      P3_DIR_PU = P3_DIR_PU |	(1<<Pin);			
      break;
    default:
      break;			
  }
}

/*******************************************************************************
* Function Name  : GPIOInterruptCfg()
* Description    : GPIO�ж�����
* Input          : None									 
* Output         : None
* Return         : None
*******************************************************************************/
void GPIOInterruptCfg()
{
   GPIO_IE &= ~bIE_IO_EDGE;                                                    //��/�͵�ƽ����
//    GPIO_IE |= bIE_IO_EDGE;                                                  //����/�½�����  
//    GPIO_IE |= bIE_RXD1_LO;                                                     //RXD1�͵�ƽ���½��ش���
   GPIO_IE |= bIE_P1_5_LO | bIE_P1_4_LO | bIE_P1_3_LO | bIE_RST_HI;            
   //P15\P14\P13�͵�ƽ������RST�ߵ�ƽ����
//    GPIO_IE |= bIE_P3_1_LO;                                                     //P31�͵�ƽ���½��ش���
//    GPIO_IE |= bIE_RXD0_LO;                                                     //RXD0�͵�ƽ���½��ش���	
   IE_GPIO  = 1;                                                               //GPIO�жϿ���
}

#ifdef GPIO_INTERRUPT
/*******************************************************************************
* Function Name  : GPIOInterrupt(void)
* Description    : GPIO �жϷ������
*******************************************************************************/
void	GPIOInterrupt( void ) interrupt INT_NO_GPIO  using 1                      //GPIO�жϷ������,ʹ�üĴ�����1
{ 
#ifdef DE_PRINTF
      printf("GPIO_STATUS: %02x\n",(UINT16)(PIN_FUNC&bIO_INT_ACT));             
#endif
}
#endif
