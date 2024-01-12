
/********************************** (C) COPYRIGHT *******************************
 * File Name          : GPIO.C
 * Author             : WCH
 * Version            : V1.0
 * Date               : 2017/01/20
 * Description        : CH552 IO 设置接口函数和GPIO中断函数
 *******************************************************************************/

#include "../Lib/CH552.H"
#include "../Lib/Debug.H"
#include "../Lib/GPIO.H"
#include "stdio.h"

#pragma NOAREGS

/*******************************************************************************
* Function Name  : Port1Cfg()
* Description    : 端口1配置
* Input          : Mode  0 = 浮空输入，无上拉
                         1 = 推挽输入输出
                         2 = 开漏输入输出，无上拉
                         3 = 类51模式，开漏输入输出，有上拉，内部电路可以加速由低到高的电平爬升
                   ,UINT8 Pin	(0-7)
* Output         : None
* Return         : None
*******************************************************************************/
void Port1Cfg(UINT8 Mode, UINT8 Pin)
{
  switch (Mode)
  {
  case 0:
    P1_MOD_OC = P1_MOD_OC & ~(1 << Pin);
    P1_DIR_PU = P1_DIR_PU & ~(1 << Pin);
    break;
  case 1:
    P1_MOD_OC = P1_MOD_OC & ~(1 << Pin);
    P1_DIR_PU = P1_DIR_PU | (1 << Pin);
    break;
  case 2:
    P1_MOD_OC = P1_MOD_OC | (1 << Pin);
    P1_DIR_PU = P1_DIR_PU & ~(1 << Pin);
    break;
  case 3:
    P1_MOD_OC = P1_MOD_OC | (1 << Pin);
    P1_DIR_PU = P1_DIR_PU | (1 << Pin);
    break;
  default:
    break;
  }
}

/*******************************************************************************
* Function Name  : Port3Cfg()
* Description    : 端口3配置
* Input          : Mode  0 = 浮空输入，无上拉
                         1 = 推挽输入输出
                         2 = 开漏输入输出，无上拉
                         3 = 类51模式，开漏输入输出，有上拉，内部电路可以加速由低到高的电平爬升
                   ,UINT8 Pin	(0-7)
* Output         : None
* Return         : None
*******************************************************************************/
void Port3Cfg(UINT8 Mode, UINT8 Pin)
{
  switch (Mode)
  {
  case 0:
    P3_MOD_OC = P3_MOD_OC & ~(1 << Pin);
    P3_DIR_PU = P3_DIR_PU & ~(1 << Pin);
    break;
  case 1:
    P3_MOD_OC = P3_MOD_OC & ~(1 << Pin);
    P3_DIR_PU = P3_DIR_PU | (1 << Pin);
    break;
  case 2:
    P3_MOD_OC = P3_MOD_OC | (1 << Pin);
    P3_DIR_PU = P3_DIR_PU & ~(1 << Pin);
    break;
  case 3:
    P3_MOD_OC = P3_MOD_OC | (1 << Pin);
    P3_DIR_PU = P3_DIR_PU | (1 << Pin);
    break;
  default:
    break;
  }
}

void EC_Cfg()
{
  // INT off
  EX0 = 0;
  EX1 = 0;
  //  Init gpios
  Port3Cfg(0, 4); // P3.4 EC_KEY
  Port3Cfg(0, 2); // P3.2 EC_A
  Port3Cfg(0, 3); // P3.3 EC_B
  // Init ints
  IT0 = 1; // FALLEDGE trig
  IT1 = 1;
  //  INT on
  EX0 = 1;
  EX1 = 1;
}

volatile short data ec_count = 0;
volatile bit data ec_rotating = 0;

void EC_IntA(void) interrupt INT_NO_INT0 using 2 // INT0中断服务程序,使用寄存器组2
{
  if (ec_rotating)
  {
    // A相后动
    if (EC_B == 1)
    {
      // 不合理的，丢步了
      // LEDA = 1;
      // LEDB = 1;
    }
    else
    {
      // 方向1转完了
      ec_count++;
      // LEDA = 1;
    }
    ec_rotating = 0;
  }
  else
  {
    // A相先动
    if (EC_B == 1)
    {
      // 方向1起转
      ec_rotating = 1;
      // LEDA = 0;
    }
    else
    {
      // 不合理的，丢步了
      ec_rotating = 0;
      // LEDA = 1;
      // LEDB = 1;
    }
  }
}

void EC_IntB(void) interrupt INT_NO_INT1 using 3 // INT1中断服务程序,使用寄存器组2
{
  if (ec_rotating)
  {
    // B相后动
    if (EC_A == 1)
    {
      // 不合理的，丢步了
      // LEDA = 1;
      // LEDB = 1;
    }
    else
    {
      // 方向2转完了
      ec_count--;
      // LEDB = 1;
    }
    ec_rotating = 0;
  }
  else
  {
    // B相先动
    if (EC_A == 1)
    {
      // 方向2起转
      ec_rotating = 1;
      // LEDB = 0;
    }
    else
    {
      // 不合理的，丢步了
      ec_rotating = 0;
      // LEDA = 1;
      // LEDB = 1;
    }
  }
}

short EC_Read()
{

  short cnt = ec_count;
  // if (!ec_rotating)
  ec_count = 0;
  return cnt;
}