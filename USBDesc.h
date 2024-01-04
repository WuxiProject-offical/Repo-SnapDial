#ifndef __USBDESC_H__
#define __USBDESC_H__

/*设备描述符*/
UINT8C DevDesc[18] = {
    0x12,            // 描述符大小
    0x01,            // 描述符类型
    0x10, 0x01,      // USB规范版本号（USB1.1）
    0x00,            // 设备类（HID设备此三项全0）
    0x00,            // 设备子类
    0x00,            // 设备协议
    THIS_ENDP0_SIZE, // EP0大小
    0x17, 0x78,      // VID
    0x02, 0x60,      // PID
    0x01, 0x00,      // 设备版本号
    1,               // 制造商字符串描述符索引
    2,               // 产品字符串描述符索引
    3,               // 序列号产品字符串描述符索引
    0x01             // 配置数目
};

UINT8C CfgDesc[] =
    {
        // 配置描述符
        0x09,     // 描述符大小
        0x02,     // 描述符类型（是配置描述符）
        34, 0x00, // 配置总长度
        0x01,     // 配置接口数
        0x01,     // 配置值
        0x00,     // 配置字符串描述符索引
        0xA0,     // 机供，唤醒
        0x32,     // 请求100mA

        // 接口描述符1,Radial Controller
        0x09, // 描述符大小
        0x04, // 描述符类型（是接口描述符）
        0x00, // 接口编号
        0x00, // 接口替用设置（？）
        0x01, // 扣除端点0后的端点数
        0x03, // 接口类（是HID）
        0x00, // 接口子类（是不支持引导服务的）
        0x00, // 无意义
        0x06, // 接口字符串描述符索引

        // HID类描述符1,Radial Controller
        0x09,       // 描述符大小
        0x21,       // 描述符类型（是HID类描述符）
        0x00, 0x01, // HID版本
        0x00,       // 国家代码
        0x01,       // 下挂HID描述符数量
        0x22,       // 下挂描述符1类型（是报表描述符）
        89, 0x00,   // 下挂描述符1长度

        // 端点描述符
        0x07,                // 描述符大小
        0x05,                // 描述符类型（是端点描述符）
        0x81,                // 是EP1IN
        0x03,                // 是中断传输
        ENDP1_IN_SIZE, 0x00, // 端点大小
        0x0a                 // 上报间隔10ms
};

/*字符串描述符*/
UINT8C LangDes[] = {
    // 语言描述符
    0x04,      // 描述符大小
    0x03,      // 描述符类型（是字符串描述符）
    0x09, 0x04 // 语言编码（是英语）
};
UINT8C StrDes1[] = {
    // 字符串描述符 制造商
    26,   // 描述符大小
    0x03, // 描述符类型（是字符串描述符）
    'W', 0x00, 'u', 0x00, 'x', 0x00, 'i', 0x00, ' ', 0x00, 'P', 0x00, 'r', 0x00, 'o', 0x00, 'j', 0x00, 'e', 0x00, 'c', 0x00, 't', 0x00};
UINT8C StrDes2[] = {
    // 字符串描述符 产品
    24,   // 描述符大小
    0x03, // 描述符类型（是字符串描述符）
    'S', 0x00, 'n', 0x00, 'a', 0x00, 'p', 0x00, 'D', 0x00, 'i', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00,
    'V', 0x00, '1', 0x00};
UINT8C StrDes3[] = {
    // 字符串描述符 序列号
    28,   // 描述符大小
    0x03, // 描述符类型（是字符串描述符）
    'T', 0x00, 'S', 0x00, 'H', 0x00, 'E', 0x00, '-', 0x00,
    '0', 0x00, '0', 0x00, '1', 0x00, '1', 0x00, '4', 0x00, '5', 0x00, '1', 0x00, '4', 0x00};
UINT8C StrDes4[] = {
    // 字符串描述符 键盘接口
    62,   // 描述符大小
    0x03, // 描述符类型（是字符串描述符）
    'S', 0x00, 'n', 0x00, 'a', 0x00, 'p', 0x00, 'D', 0x00, 'i', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00,
    'R', 0x00, 'a', 0x00, 'd', 0x00, 'i', 0x00, 'a', 0x00, 'l', 0x00, ' ', 0x00, 'C', 0x00, 't', 0x00, 'r', 0x00, 'l', 0x00, ' ', 0x00,
    'I', 0x00, 'n', 0x00, 't', 0x00, 'e', 0x00, 'r', 0x00, 'f', 0x00, 'a', 0x00, 'c', 0x00, 'e', 0x00};

/*HID类报表描述符*/
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