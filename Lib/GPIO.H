
#define GPIO_INTERRUPT 1

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
void Port1Cfg(UINT8 Mode, UINT8 Pin);
void Port3Cfg(UINT8 Mode, UINT8 Pin);

sbit EC_A = P3 ^ 2;
sbit EC_B = P3 ^ 3;
sbit EC_KEY = P3 ^ 4;

sbit LEDA = P1 ^ 4;
sbit LEDB = P1 ^ 5;

void EC_Cfg();
short EC_Read();