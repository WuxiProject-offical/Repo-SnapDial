#include "./Lib/CH552.H"
#include "./Lib/Debug.H"
#include "./Lib/GPIO.H"
#include <string.h>
#include <stdio.h>

sbit EC_A = P3 ^ 2;
sbit EC_B = P3 ^ 3;
sbit EC_KEY = P3 ^ 4;

// #define Fullspeed
#define THIS_ENDP0_SIZE DEFAULT_ENDP0_SIZE
#define ENDP1_IN_SIZE 4

// �˵�0 OUT&IN��������������ż��ַ
UINT8X Ep0Buffer[MIN(64, THIS_ENDP0_SIZE + 2)] _at_ 0x0000;
// �˵�1 IN������,������ż��ַ
UINT8X Ep1Buffer[MIN(64, ENDP1_IN_SIZE + 2)] _at_ MIN(64, THIS_ENDP0_SIZE + 2);

/*�������*/
UINT8 HIDDial[4] = {0x0, 0x0, 0x0, 0x0};

UINT16 SetupLen;
UINT8 SetupReq, Ready, Count, FLAG_EP1, UsbConfig;
PUINT8 pDescr;             // USB���ñ�־
USB_SETUP_REQ SetupReqBuf; // �ݴ�Setup��

#define UsbSetupBuf ((PUSB_SETUP_REQ)Ep0Buffer)
#define DEBUG 0
#pragma NOAREGS

#include "USBDesc.h"

/*******************************************************************************
 * Function Name  : CH552SoftReset()
 * Description    : CH552����λ
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
 * Description    : CH552�豸ģʽ��������������K�ź�
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
 * Description    : USB�豸ģʽ����,�豸ģʽ�������շ��˵����ã��жϿ���
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void USBDeviceInit()
{
    IE_USB = 0;
    USB_CTRL = 0x00;        // ���趨USB�豸ģʽ
    UDEV_CTRL = bUD_PD_DIS; // ��ֹDP/DM��������

    UDEV_CTRL &= ~bUD_LOW_SPEED; // ѡ��ȫ��12Mģʽ��Ĭ�Ϸ�ʽ
    USB_CTRL &= ~bUC_LOW_SPEED;

    UEP0_DMA = Ep0Buffer;                       // �˵�0���ݴ����ַ
    UEP4_1_MOD &= ~(bUEP4_RX_EN | bUEP4_TX_EN); // �˵�0��64�ֽ��շ�������

    UEP1_DMA = Ep1Buffer;                                   // �˵�1���ݴ����ַ
    UEP4_1_MOD = UEP4_1_MOD & ~bUEP1_BUF_MOD | bUEP1_TX_EN; // �˵�1����ʹ�� 64�ֽڻ�����

    USB_DEV_AD = 0x00;
    USB_CTRL |= bUC_DEV_PU_EN | bUC_INT_BUSY | bUC_DMA_EN; // ����USB�豸��DMA�����ж��ڼ��жϱ�־δ���ǰ�Զ�����NAK
    UDEV_CTRL |= bUD_PORT_EN;                              // ����USB�˿�
    USB_INT_FG = 0xFF;                                     // ���жϱ�־
    USB_INT_EN = bUIE_SUSPEND | bUIE_TRANSFER | bUIE_BUS_RST;
    IE_USB = 1;
}

void USBDeviceReset()
{
    IE_USB = 0;
    USB_CTRL = 0x06;
    UDEV_CTRL = bUD_PD_DIS; // ��ֹDP/DM��������
    mDelaymS(100);
    USBDeviceInit();
}

/*******************************************************************************
 * Function Name  : Enp1IntIn()
 * Description    : USB�豸ģʽ�˵�1���ж��ϴ�
 * Input          : None
 * Output         : None
 * Return         : None
 *******************************************************************************/
void Enp1IntIn()
{
    memcpy(Ep1Buffer, HIDDial, sizeof(HIDDial));             // �����ϴ�����
    UEP1_T_LEN = sizeof(HIDDial);                            // �ϴ����ݳ���
    UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK; // ������ʱ�ϴ����ݲ�Ӧ��ACK
}

/*******************************************************************************
 * Function Name  : DeviceInterrupt()
 * Description    : CH552USB�жϴ�������
 *******************************************************************************/
void DeviceInterrupt(void) interrupt INT_NO_USB using 1 // USB�жϷ������,ʹ�üĴ�����1
{
    UINT8 len = 0, i, failFlag = 0;
    if (UIF_TRANSFER) // USB������ɱ�־
    {
        switch (USB_INT_ST & (MASK_UIS_TOKEN | MASK_UIS_ENDP))
        {
        case UIS_TOKEN_IN | 1:                                       // endpoint 1# �ж϶˵��ϴ�
            UEP1_T_LEN = 0;                                          // Ԥʹ�÷��ͳ���һ��Ҫ���
            UEP1_CTRL ^= bUEP_T_TOG;                                 // �ֶ���תͬ����־λ
            UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK; // Ĭ��Ӧ��NAK
            FLAG_EP1 = 1;                                            /*������ɱ�־*/
            break;
        case UIS_TOKEN_SETUP | 0: // SETUP����
            UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK;
            len = USB_RX_LEN;
            failFlag = 0;
            if (len == (sizeof(USB_SETUP_REQ)))
            {
                SetupLen = ((UINT16)UsbSetupBuf->wLengthH << 8) | (UsbSetupBuf->wLengthL);
                len = 0; // Ĭ��Ϊ�ɹ������ϴ�0����
                SetupReq = UsbSetupBuf->bRequest;
                if ((UsbSetupBuf->bRequestType & USB_REQ_TYP_MASK) != USB_REQ_TYP_STANDARD) /* HID������ */
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
                        failFlag = 1; /*���֧��*/
                        break;
                    }
                }
                else
                {                     // ��׼����
                    switch (SetupReq) // ������
                    {
                    case USB_GET_DESCRIPTOR:
                        switch (UsbSetupBuf->wValueH)
                        {
                        case 1:               // �豸������
                            pDescr = DevDesc; // ���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(DevDesc);
                            break;
                        case 2:               // ����������
                            pDescr = CfgDesc; // ���豸�������͵�Ҫ���͵Ļ�����
                            len = sizeof(CfgDesc);
                            break;
                        case 3: // �ַ���������
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
                                failFlag = 1; // �����ڵ�������
                                break;
                            }
                            break;
                        case 0x22:                         // ����������
                            if (UsbSetupBuf->wIndexL == 0) // �ӿ�0����������
                            {
                                pDescr = HIDRepDesc; // ����׼���ϴ�
                                len = sizeof(HIDRepDesc);
                            }
                            else
                            {
                                failFlag = 1; // ������ֻ��3���ӿڣ���仰����������ִ��
                            }
                            break;
                        default:
                            failFlag = 1; // ��֧�ֵ�������߳���
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
                                SetupLen = len; // �����ܳ���
                            }
                            len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; // ���δ��䳤��
                            memcpy(Ep0Buffer, pDescr, len);                                 // �����ϴ�����
                            SetupLen -= len;
                            pDescr += len;
                        }
                        break;
                    case USB_SET_ADDRESS:
                        SetupLen = UsbSetupBuf->wValueL; // �ݴ�USB�豸��ַ
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
                            Ready = 1; // set config����һ�����usbö����ɵı�־
                        }
                        break;
                    case 0x0A:
                        break;
                    case USB_CLEAR_FEATURE:                                                         // Clear Feature
                        if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_ENDP) // �˵�
                        {
                            switch (UsbSetupBuf->wIndexL)
                            {
                            case 0x81:
                                UEP1_CTRL = UEP1_CTRL & ~(bUEP_T_TOG | MASK_UEP_T_RES) | UEP_T_RES_NAK;
                                break;
                            default:
                                failFlag = 1; // ��֧�ֵĶ˵�
                                break;
                            }
                        }
                        if ((UsbSetupBuf->bRequestType & USB_REQ_RECIP_MASK) == USB_REQ_RECIP_DEVICE) // �豸
                        {
                            break;
                        }
                        else
                        {
                            failFlag = 1; // ���Ƕ˵㲻֧��
                        }
                        break;
                    case USB_SET_FEATURE:                               /* Set Feature */
                        if ((UsbSetupBuf->bRequestType & 0x1F) == 0x00) /* �����豸 */
                        {
                            if ((((UINT16)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x01)
                            {
                                if (CfgDesc[7] & 0x20)
                                {
                                    /* ���û���ʹ�ܱ�־ */
                                }
                                else
                                {
                                    failFlag = 1; /* ����ʧ�� */
                                }
                            }
                            else
                            {
                                failFlag = 1; /* ����ʧ�� */
                            }
                        }
                        else if ((UsbSetupBuf->bRequestType & 0x1F) == 0x02) /* ���ö˵� */
                        {
                            if ((((UINT16)UsbSetupBuf->wValueH << 8) | UsbSetupBuf->wValueL) == 0x00)
                            {
                                switch (((UINT16)UsbSetupBuf->wIndexH << 8) | UsbSetupBuf->wIndexL)
                                {
                                case 0x83:
                                    UEP3_CTRL = UEP3_CTRL & (~bUEP_T_TOG) | UEP_T_RES_STALL; /* ���ö˵�3 IN Stall */
                                    break;
                                default:
                                    failFlag = 1; // ����ʧ��
                                    break;
                                }
                            }
                            else
                            {
                                failFlag = 1; // ����ʧ��
                            }
                        }
                        else
                        {
                            failFlag = 1; // ����ʧ��
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
                        failFlag = 1; // ����ʧ��
                        break;
                    }
                }
            }
            else
            {
                failFlag = 1; // �����ȴ���
            }
            if (failFlag)
            {
                SetupReq = 0xFF;
                len = 0;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_STALL | UEP_T_RES_STALL; // STALL
            }
            else if (len) // �ϴ����ݻ���״̬�׶η���0���Ȱ�
            {
                UEP0_T_LEN = len;
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // Ĭ�����ݰ���DATA1������Ӧ��ACK
            }
            else
            {
                UEP0_T_LEN = 0;                                                      // ��Ȼ��δ��״̬�׶Σ�������ǰԤ���ϴ�0�������ݰ��Է�������ǰ����״̬�׶�
                UEP0_CTRL = bUEP_R_TOG | bUEP_T_TOG | UEP_R_RES_ACK | UEP_T_RES_ACK; // Ĭ�����ݰ���DATA1,����Ӧ��ACK
            }
            break;
        case UIS_TOKEN_IN | 0: // endpoint0 IN
            switch (SetupReq)
            {
            case USB_GET_DESCRIPTOR:
                len = SetupLen >= THIS_ENDP0_SIZE ? THIS_ENDP0_SIZE : SetupLen; // ���δ��䳤��
                memcpy(Ep0Buffer, pDescr, len);                                 // �����ϴ�����
                SetupLen -= len;
                pDescr += len;
                UEP0_T_LEN = len;
                UEP0_CTRL ^= bUEP_T_TOG; // ͬ����־λ��ת
                break;
            case USB_SET_ADDRESS:
                USB_DEV_AD = USB_DEV_AD & bUDA_GP_BIT | SetupLen;
                UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
                break;
            default:
                UEP0_T_LEN = 0; // ״̬�׶�����жϻ�����ǿ���ϴ�0�������ݰ��������ƴ���
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
            UEP0_CTRL ^= bUEP_R_TOG; // ͬ����־λ��ת
            break;
        default:
            break;
        }
        UIF_TRANSFER = 0; // д0����ж�
    }
    if (UIF_BUS_RST) // �豸ģʽUSB���߸�λ�ж�
    {
        UEP0_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        UEP1_CTRL = UEP_T_RES_NAK;
        UEP2_CTRL = UEP_T_RES_NAK;
        UEP3_CTRL = UEP_R_RES_ACK | UEP_T_RES_NAK;
        USB_DEV_AD = 0x00;
        UIF_SUSPEND = 0;
        UIF_TRANSFER = 0;
        UIF_BUS_RST = 0; // ���жϱ�־
    }
    if (UIF_SUSPEND) // USB���߹���/�������
    {
        UIF_SUSPEND = 0;
        if (USB_MIS_ST & bUMS_SUSPEND) // ����
        {
            // Zzz
        }
    }
    else
    {                      // ������ж�,�����ܷ��������
        USB_INT_FG = 0xFF; // ���жϱ�־
        //      printf("UnknownInt  N");
    }
}

void HIDValueHandle()
{
    UINT8 i = 0;
    printf("%c", (UINT8)i);
    switch (i)
    {
    case 'Q': // Num Lock��
        FLAG_EP1 = 0;
        // HIDKey[2] = 0x39;
        Enp1IntIn();
        // HIDKey[2] = 0; // ��������
        while (FLAG_EP1 == 0)
        {
            ; /*�ȴ���һ���������*/
        }
        Enp1IntIn();
        break;
    default:                                                     // ����
        UEP1_CTRL = UEP1_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK; // Ĭ��Ӧ��NAK
        break;
    }
}

main()
{
    CfgFsys();
    mDelaymS(5); // �޸���Ƶ�ȴ��ڲ������ȶ�,�ؼ�

    Port3Cfg(0, 4); // P3.4 EC_KEY
    Port3Cfg(0, 2); // P3.2 EC_A
    Port3Cfg(0, 3); // P3.3 EC_B

    USBDeviceReset();
    // USBDeviceInit(); // USB�豸ģʽ��ʼ��
    EA = 1;         // ������Ƭ���ж�
    UEP1_T_LEN = 0; // Ԥʹ�÷��ͳ���һ��Ҫ���
    FLAG_EP1 = 0;
    Ready = 0;
    while (1)
    {
        if (Ready)
        {
            // Enp1IntIn(); // �����ͼ��̼�ֵ��̧�𡱲���
            //if (EC_KEY == 0)
            {
                FLAG_EP1 = 0;
                HIDDial[0] = 0x01;
                Enp1IntIn();
                while (FLAG_EP1 == 0)
                {
                    ; /*�ȴ���һ���������*/
                }
            }
						mDelaymS(10); // ģ�ⵥƬ����������
            //else
            {
                FLAG_EP1 = 0;
                HIDDial[0] = 0x00;
                Enp1IntIn();
                while (FLAG_EP1 == 0)
                {
                    ; /*�ȴ���һ���������*/
                }
            }
						mDelaymS(1000); // ģ�ⵥƬ����������
        }
    }
}