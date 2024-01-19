#include "./Lib/CH552.H"
#include "./Lib/Debug.H"
#include "./Lib/GPIO.H"
#include <string.h>
#include <stdio.h>

#define THIS_ENDP0_SIZE DEFAULT_ENDP0_SIZE
#define ENDP1_IN_SIZE 16

// 端点0 OUT&IN缓冲区，必须是偶地址
UINT8X Ep0Buffer[MIN(64, THIS_ENDP0_SIZE + 2)] _at_ 0x0000;
// 端点1 IN缓冲区,必须是偶地址
UINT8X Ep1Buffer[MIN(64, ENDP1_IN_SIZE + 2)] _at_ MIN(64, THIS_ENDP0_SIZE + 2);

/*旋钮数据*/
UINT8 HIDDial[6] = {0x01, 0x0, 0x0, 0x0, 0x0, 0x0};
short DialRotation = 0;

UINT16 SetupLen;
UINT8 SetupReq, Ready, Count, FLAG_EP1, UsbConfig;
PUINT8 pDescr;             // USB配置标志
USB_SETUP_REQ SetupReqBuf; // 暂存Setup包

#define UsbSetupBuf ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0
#pragma NOAREGS

#include "USBDesc.h"

/*******************************************************************************
 * Function Name  : CH552SoftReset()
 * Description    : CH552软复位
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH552SoftReset()
{
    SAFE_MOD = 0x55;
    SAFE_MOD = 0xAA;
    GLOBAL_CFG |= bSW_RESET;
}

/*******************************************************************************
 * Function Name  : CH552USBDevWakeup()
 * Description    : CH552设备模式唤醒主机，发送K信号
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void CH552USBDevWakeup()
{
    UDEV_CTRL |= bUD_LOW_SPEED;
    mDelaymS(2);
    UDEV_CTRL &= ~bUD_LOW_SPEED;
}

/*******************************************************************************
 * Function Name  : USBDeviceInit()
 * Description    : USB设备模式配置,设备模式启动，收发端点配置，中断开启
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USBDeviceInit()
{
    IE_USB = 0;
    USB_CTRL = 0x00;        // 先设定USB设备模式
    UDEV_CTRL = bUD_PD_DIS; // 禁止DP/DM下拉电阻

    UDEV_CTRL &= ~bUD_LOW_SPEED; // 选择全速12M模式，默认方式
    USB_CTRL &= ~bUC_LOW_SPEED;

    UEP0_DMA = Ep0Buffer;                       // 端点0数据传输地址
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN); // 端点0单64字节收发缓冲区

    UEP1_DMA = Ep1Buffer;                                   // 端点1数据传输地址
    UEP4_1_MOD = UEP4_1_MOD & ~bUEP1_BUF_MOD | bUEP1_TX_EN; // 端点1发送使能 64字节缓冲区

    USB_DEV_AD = 0x00;
    USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN; // 启动USB设备及DMA，在中断期间中断标志未清除前自动返回NAK
    UDEV_CTRL |= bUD_PORT_EN;                              // 允许USB端口
    USB_INT_FG = 0xFF;                                     // 清中断标志
    USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
    IP_EX |= bIP_USB; // USB使用高级中断
    IE_USB = 1;
}

void USBDeviceReset()
{
    IE_USB = 0;
    USB_CTRL = 0x06;
    UDEV_CTRL = bUD_PD_DIS; // 禁止DP/DM下拉电阻
    mDelaymS(100);
    USBDeviceInit();
}

/*******************************************************************************
 * Function Name  : Enp1IntIn()
 * Description    : USB设备模式端点1的中断上传
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Enp1IntIn()
{
    memcpy(Ep1Buffer, HIDDial, sizeof(HIDDial));             // 加载上传数据
    UEP1_T_LEN = sizeof(HIDDial);                            // 上传数据长度
    UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK; // 有数据时上传数据并应答ACK
}

/*******************************************************************************
 * Function Name  : DeviceInterrupt()
 * Description    : CH552USB中断处理函数
 *******************************************************************************/
void DeviceInterrupt(void) interrupt INT_NO_USB using 1 // USB中断服务程序,使用寄存器组1
{
    UINT8 len = 0, i, failFlag = 0;
    if (UIF_TRANSFER) // USB传输完成标志
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 1:                                       // endpoint 1# 中断端点上传
            UEP1_T_LEN = 0;                                          // 预使用发送长度一定要清空
            UEP1_CTRL ^= bUEP_T_TOG;                                 // 手动翻转同步标志位
            UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK; // 默认应答NAK
            FLAG_EP1 = 1;                                            /*传输完成标志*/
            break;
        case UIS_TOKEN_SETUP | 0: // SETUP事务
            UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
            len = USB_RX_LEN;
            failFlag = 0;
            if (len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = ((UINT16)UsbSetupBuf->wLengthH << 8) | (UsbSetupBuf->wLengthL);
                len = 0; // 默认为成功并且上传0长度
                SetupReq = UsbSetupBuf->bRequest;
                if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) /* HID类命令 */
                {
                    switch (SetupReq)
                    {
                    case 0x01: // GetReport
                        break;
                    case 0x02: // GetIdle
                        break;
                    case 0x03: // GetProtocol
                        break;
                    case 0x09: // SetReport
                        break;
                    case 0x0A: // SetIdle
                        break;
                    case 0x0B: // SetProtocol
                        break;
                    default:
                        failFlag = 1; /*命令不支持*/
                        break;
                    }
                }
                else
                {                     // 标准请求
                    switch (SetupReq) // 请求码
                    {
                    case USB_GET_DESCRIPTOR:
                        switch (UsbSetupBuf->wValueH)
                        {
                        case 1:               // 设备描述符
                            pDescr = DevDesc; // 把设备描述符送到要发送的缓冲区
                            len = sizeof(DevDesc);
                            break;
                        case 2:               // 配置描述符
                            pDescr = CfgDesc; // 把设备描述符送到要发送的缓冲区
                            len = sizeof(CfgDesc);
                            break;
                        case 3: // 字符串描述符
                            switch (UsbSetupBuf->wValueL)
                            {
                            case 1:
                                pDescr = (PUINT8)(&StrDes1[0]);
                                len = sizeof(StrDes1);
                                break;
                            case 2:
                                pDescr = (PUINT8)(&StrDes2[0]);
                                len = sizeof(StrDes2);
                                break;
                            case 3:
                                pDescr = (PUINT8)(&StrDes3[0]);
                                len = sizeof(StrDes3);
                                break;
                            case 4:
                                pDescr = (PUINT8)(&StrDes4[0]);
                                len = sizeof(StrDes4);
                                break;

                            case 0:
                                pDescr = (PUINT8)(&LangDes[0]);
                                len = sizeof(LangDes);
                                break;
                            default:
                                failFlag = 1; // 不存在的描述符
                                break;
                            }
                            break;
                        case 0x22:                         // 报表描述符
                            if (UsbSetupBuf->wIndexL == 0) // 接口0报表描述符
                            {
                                pDescr = HIDRepDesc; // 数据准备上传
                                len = sizeof(HIDRepDesc);
                            }
                            else
                            {
                                failFlag = 1; // 本程序只有3个接口，这句话正常不可能执行
                            }
                            break;
                        default:
                            failFlag = 1; // 不支持的命令或者出错
                            break;
                        }
                        if (failFlag)
                        {
                            SetupReq = 0xFF;
                            len = 0;
                            UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
                        }
                        else
                        {
                            if (SetupLen > len)
                            {
                                SetupLen = len; // 限制总长度
                            }
                            len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; // 本次传输长度
                            memcpy(Ep0Buffer, pDescr, len);                                 // 加载上传数据
                            SetupLen -= len;
                            pDescr += len;
                        }
                        break;
                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL; // 暂存USB设备地址
                        break;
                    case USB_GET_CONFIGURATION:
                        Ep0Buffer[0] = UsbConfig;
                        if (SetupLen >= 1)
                        {
                            len = 1;
                        }
                        break;
                    case USB_SET_CONFIGURATION:
                        UsbConfig = UsbSetupBuf->wValueL;
                        if (UsbConfig)
                        {
                            Ready = 1; // set config命令一般代表usb枚举完成的标志
                        }
                        break;
                    case 0x0A:
                        break;
                    case USB_CLEAR_FEATURE:                                                         // Clear Feature
                        if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // 端点
                        {
                            switch (UsbSetupBuf->wIndexL)
                            {
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
                                break;
                            default:
                                failFlag = 1; // 不支持的端点
                                break;
                            }
                        }
                        if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE) // 设备
                        {
                            break;
                        }
                        else
                        {
                            failFlag = 1; // 不是端点不支持
                        }
                        break;
                    case USB_SET_FEATURE:                               /* Set Feature */
                        if ((UsbSetupBuf->bRequestType & 0x1F) == 0x00) /* 设置设备 */
                        {
                            if ((((UINT16)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01)
                            {
                                if (CfgDesc[7] & 0x20)
                                {
                                    /* 设置唤醒使能标志 */
                                }
                                else
                                {
                                    failFlag = 1; /* 操作失败 */
                                }
                            }
                            else
                            {
                                failFlag = 1; /* 操作失败 */
                            }
                        }
                        else if ((UsbSetupBuf->bRequestType & 0x1F) == 0x02) /* 设置端点 */
                        {
                            if ((((UINT16)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x00)
                            {
                                switch (((UINT16)UsbSetupBuf->wIndexH << 8) | UsbSetupBuf->wIndexL)
                                {
                                case 0x83:
                                    UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL; /* 设置端点3 IN Stall */
                                    break;
                                default:
                                    failFlag = 1; // 操作失败
                                    break;
                                }
                            }
                            else
                            {
                                failFlag = 1; // 操作失败
                            }
                        }
                        else
                        {
                            failFlag = 1; // 操作失败
                        }
                        break;
                    case USB_GET_STATUS:
                        Ep0Buffer[0] = 0x00;
                        Ep0Buffer[1] = 0x00;
                        if (SetupLen >= 2)
                        {
                            len = 2;
                        }
                        else
                        {
                            len = SetupLen;
                        }
                        break;
                    default:
                        failFlag = 1; // 操作失败
                        break;
                    }
                }
            }
            else
            {
                failFlag = 1; // 包长度错误
            }
            if (failFlag)
            {
                SetupReq = 0xFF;
                len = 0;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
            }
            else if (len) // 上传数据或者状态阶段返回0长度包
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // 默认数据包是DATA1，返回应答ACK
            }
            else
            {
                UEP0_T_LEN = 0;                                                      // 虽然尚未到状态阶段，但是提前预置上传0长度数据包以防主机提前进入状态阶段
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // 默认数据包是DATA1,返回应答ACK
            }
            break;
        case UIS_TOKEN_IN | 0: // endpoint0 IN
            switch (SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; // 本次传输长度
                memcpy(Ep0Buffer, pDescr, len);                                 // 加载上传数据
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG; // 同步标志位翻转
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0; // 状态阶段完成中断或者是强制上传0长度数据包结束控制传输
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            }
            break;
        case UIS_TOKEN_OUT | 0: // endpoint0 OUT
            len = USB_RX_LEN;
            if (SetupReq == 0x09)
            {
                if (Ep0Buffer[0])
                {
                    // printf("Light on Num Lock LED!\n");
                }
                else if (Ep0Buffer[0] == 0)
                {
                    // printf("Light off Num Lock LED!\n");
                }
            }
            UEP0_CTRL ^= bUEP_R_TOG; // 同步标志位翻转
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0; // 写0清空中断
    }
    if (UIF_BUS_RST) // 设备模式USB总线复位中断
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = UEP_T_RES_NAK;
        UEP2_CTRL = UEP_T_RES_NAK;
        UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0; // 清中断标志
    }
    if (UIF_SUSPEND) // USB总线挂起/唤醒完成
    {
        UIF_SUSPEND = 0;
        if (USB_MIS_ST & bUMS_SUSPEND) // 挂起
        {
            // Zzz
        }
    }
    else
    {                      // 意外的中断,不可能发生的情况
        USB_INT_FG = 0xFF; // 清中断标志
        //      printf("UnknownInt  N");
    }
}

main()
{
    CfgFsys();
    mDelaymS(5); // 修改主频等待内部晶振稳定,必加

    Port1Cfg(3, 4);
    Port1Cfg(3, 5);
    LEDA = 1;
    LEDB = 1;
    EC_Cfg();

    // USBDeviceReset();
    USBDeviceInit(); // USB设备模式初始化
    EA = 1;          // 允许单片机中断
    UEP1_T_LEN = 0;  // 预使用发送长度一定要清空
    FLAG_EP1 = 0;
    Ready = 0;
    while (1)
    {
        if (Ready)
        {
            FLAG_EP1 = 0;
            HIDDial[1] = (EC_KEY) ? 0x00 : 0x01;
            DialRotation = EC_Read()*50;
            HIDDial[2] = DialRotation & 0x00ff;
            HIDDial[3] = ((DialRotation & 0xff00) >> 8);
            Enp1IntIn();
            while (FLAG_EP1 == 0)
            {
                ; /*等待上一包传输完成*/
            }

            mDelaymS(50); // 模拟单片机做其它事
        }
    }
}
