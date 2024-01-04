#ifndef __USBDESC_H__
#define __USBDESC_H__

/*�豸������*/
UINT8C DevDesc[18] = {
    0x12,            // ��������С
    0x01,            // ����������
    0x10, 0x01,      // USB�淶�汾�ţ�USB1.1��
    0x00,            // �豸�ࣨHID�豸������ȫ0��
    0x00,            // �豸����
    0x00,            // �豸Э��
    THIS_ENDP0_SIZE, // EP0��С
    0x17, 0x78,      // VID
    0x02, 0x60,      // PID
    0x01, 0x00,      // �豸�汾��
    1,               // �������ַ�������������
    2,               // ��Ʒ�ַ�������������
    3,               // ���кŲ�Ʒ�ַ�������������
    0x01             // ������Ŀ
};

UINT8C CfgDesc[] =
    {
        // ����������
        0x09,     // ��������С
        0x02,     // ���������ͣ���������������
        34, 0x00, // �����ܳ���
        0x01,     // ���ýӿ���
        0x01,     // ����ֵ
        0x00,     // �����ַ�������������
        0xA0,     // ����������
        0x32,     // ����100mA

        // �ӿ�������1,Radial Controller
        0x09, // ��������С
        0x04, // ���������ͣ��ǽӿ���������
        0x00, // �ӿڱ��
        0x00, // �ӿ��������ã�����
        0x01, // �۳��˵�0��Ķ˵���
        0x03, // �ӿ��ࣨ��HID��
        0x00, // �ӿ����ࣨ�ǲ�֧����������ģ�
        0x00, // ������
        0x06, // �ӿ��ַ�������������

        // HID��������1,Radial Controller
        0x09,       // ��������С
        0x21,       // ���������ͣ���HID����������
        0x00, 0x01, // HID�汾
        0x00,       // ���Ҵ���
        0x01,       // �¹�HID����������
        0x22,       // �¹�������1���ͣ��Ǳ�����������
        89, 0x00,   // �¹�������1����

        // �˵�������
        0x07,                // ��������С
        0x05,                // ���������ͣ��Ƕ˵���������
        0x81,                // ��EP1IN
        0x03,                // ���жϴ���
        ENDP1_IN_SIZE, 0x00, // �˵��С
        0x0a                 // �ϱ����10ms
};

/*�ַ���������*/
UINT8C LangDes[] = {
    // ����������
    0x04,      // ��������С
    0x03,      // ���������ͣ����ַ�����������
    0x09, 0x04 // ���Ա��루��Ӣ�
};
UINT8C StrDes1[] = {
    // �ַ��������� ������
    26,   // ��������С
    0x03, // ���������ͣ����ַ�����������
    'W', 0x00, 'u', 0x00, 'x', 0x00, 'i', 0x00, ' ', 0x00, 'P', 0x00, 'r', 0x00, 'o', 0x00, 'j', 0x00, 'e', 0x00, 'c', 0x00, 't', 0x00};
UINT8C StrDes2[] = {
    // �ַ��������� ��Ʒ
    24,   // ��������С
    0x03, // ���������ͣ����ַ�����������
    'S', 0x00, 'n', 0x00, 'a', 0x00, 'p', 0x00, 'D', 0x00, 'i', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00,
    'V', 0x00, '1', 0x00};
UINT8C StrDes3[] = {
    // �ַ��������� ���к�
    28,   // ��������С
    0x03, // ���������ͣ����ַ�����������
    'T', 0x00, 'S', 0x00, 'H', 0x00, 'E', 0x00, '-', 0x00,
    '0', 0x00, '0', 0x00, '1', 0x00, '1', 0x00, '4', 0x00, '5', 0x00, '1', 0x00, '4', 0x00};
UINT8C StrDes4[] = {
    // �ַ��������� ���̽ӿ�
    62,   // ��������С
    0x03, // ���������ͣ����ַ�����������
    'S', 0x00, 'n', 0x00, 'a', 0x00, 'p', 0x00, 'D', 0x00, 'i', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00,
    'R', 0x00, 'a', 0x00, 'd', 0x00, 'i', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00, 'C', 0x00, 't', 0x00, 'r', 0x00, 'l', 0x00, ' ', 0x00,
    'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00};

/*HID�౨��������*/
UINT8C HIDRepDesc[] =
    {
        0x05, 0x01, // USAGE_PAGE (Generic Desktop)
        0x09, 0x0e, // USAGE (System Multi-Axis Controller)
        0xa1, 0x01, // COLLECTION (Application)
        0x85, 0x01, //   REPORT_ID (Radial Controller)
        0x05, 0x0d, //   USAGE_PAGE (Digitizers)
        0x09, 0x21, //   USAGE (Puck)
        0xa1, 0x00, //   COLLECTION (Physical)

        0x05, 0x09, //     USAGE_PAGE (Buttons)
        0x09, 0x01, //     USAGE (Button 1)
        0x95, 0x01, //     REPORT_COUNT (1)
        0x75, 0x01, //     REPORT_SIZE (1)
        0x15, 0x00, //     LOGICAL_MINIMUM (0)
        0x25, 0x01, //     LOGICAL_MAXIMUM (1)
        0x81, 0x02, //     INPUT (Data,Var,Abs)
        0x95, 0x01, //     REPORT_COUNT (1)
        0x75, 0x07, //     REPORT_SIZE (7)
        0x81, 0x03, //     INPUT (Cnst,Var,Abs)

        0x05, 0x01,       //     USAGE_PAGE (Generic Desktop)
        0x09, 0x37,       //     USAGE (Dial)
        0x95, 0x01,       //     REPORT_COUNT (1)
        0x75, 0x10,       //     REPORT_SIZE (16)
        0x55, 0x0f,       //     UNIT_EXPONENT (-1)
        0x65, 0x14,       //     UNIT (Degrees, English Rotation)
        0x36, 0xf0, 0xf1, //     PHYSICAL_MINIMUM (-3600)
        0x46, 0x10, 0x0e, //     PHYSICAL_MAXIMUM (3600)
        0x16, 0xf0, 0xf1, //     LOGICAL_MINIMUM (-3600)
        0x26, 0x10, 0x0e, //     LOGICAL_MAXIMUM (3600)
        0x81, 0x06,       //     INPUT (Data,Var,Rel)

        0x09, 0x30, //     USAGE (X)
        0x75, 0x08, //     REPORT_SIZE (8)
        0x55, 0x0d, //     UNIT_EXPONENT (-3)
        0x65, 0x13, //     UNIT (Inch,EngLinear)
        0x35, 0x00, //     PHYSICAL_MINIMUM (0)
        0x45, 0xff, //     PHYSICAL_MAXIMUM (255)
        0x15, 0x20, //     LOGICAL_MINIMUM (32)
        0x25, 0x40, //     LOGICAL_MAXIMUM (64)
        0x81, 0x42, //     INPUT (Data,Var,Abs,Null)

        0x09, 0x31, //     USAGE (Y)
        0x81, 0x42, //     INPUT (Data,Var,Abs,Null)

        0xc0, //   END_COLLECTION
        0xc0, // END_COLLECTION

        0x09, 0x80, // Usage (System Control)
        0xa1, 0x01, // Collection (Application)
        0xc0        // End Collection
};

#endif