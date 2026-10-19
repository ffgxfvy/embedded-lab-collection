#include "ds1302.h"
#include <intrins.h>

// 短延时函数
static void DS1302_Delay(uint n) {
    while(n--) {
        _nop_(); _nop_(); _nop_(); _nop_();
    }
}

// 向DS1302写入一个字节
static void DS1302_WriteByte(uchar addr, uchar dat) {
    uchar i;
    
    DS1302_CE = 0;
    DS1302_CLK = 0;
    DS1302_Delay(1);
    
    // 使能芯片
    DS1302_CE = 1;
    DS1302_Delay(1);
    
    // 发送地址（先发最低位）
    for(i = 0; i < 8; i++) {
        DS1302_IO = addr & 0x01;
        DS1302_Delay(1);
        DS1302_CLK = 1;
        DS1302_Delay(1);
        DS1302_CLK = 0;
        DS1302_Delay(1);
        addr >>= 1;
    }
    
    // 发送数据
    for(i = 0; i < 8; i++) {
        DS1302_IO = dat & 0x01;
        DS1302_Delay(1);
        DS1302_CLK = 1;
        DS1302_Delay(1);
        DS1302_CLK = 0;
        DS1302_Delay(1);
        dat >>= 1;
    }
    
    // 结束传输
    DS1302_CE = 0;
    DS1302_Delay(1);
}

// 从DS1302读取一个字节
static uchar DS1302_ReadByte(uchar addr) {
    uchar i, dat = 0;
    
    DS1302_CE = 0;
    DS1302_CLK = 0;
    DS1302_Delay(1);
    
    // 使能芯片
    DS1302_CE = 1;
    DS1302_Delay(1);
    
    // 发送地址
    for(i = 0; i < 8; i++) {
        DS1302_IO = addr & 0x01;
        DS1302_Delay(1);
        DS1302_CLK = 1;
        DS1302_Delay(1);
        DS1302_CLK = 0;
        DS1302_Delay(1);
        addr >>= 1;
    }
    
    // 释放数据线
    DS1302_IO = 1;
    
    // 读取数据
    for(i = 0; i < 8; i++) {
        dat >>= 1;
        if(DS1302_IO) dat |= 0x80;
        DS1302_Delay(1);
        DS1302_CLK = 1;
        DS1302_Delay(1);
        DS1302_CLK = 0;
        DS1302_Delay(1);
    }
    
    // 结束传输
    DS1302_CE = 0;
    DS1302_Delay(1);
    
    return dat;
}

// BCD码转十进制
static uchar BCD_To_Dec(uchar bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

// 十进制转BCD码
static uchar Dec_To_BCD(uchar dec) {
    return ((dec / 10) << 4) | (dec % 10);
}

// 初始化DS1302
void DS1302_Init(void) {
    uchar second, hour;
    
    // 初始化引脚
    DS1302_CE = 0;
    DS1302_CLK = 0;
    DS1302_IO = 0;
    
    // 关闭写保护
    DS1302_WriteByte(0x8E, 0x00);
    
    // 关闭涓流充电
    DS1302_WriteByte(0x90, 0x00);
    
    // 读取秒寄存器
    second = DS1302_ReadByte(0x81);
    
    // 检查时钟是否停止（bit7=1表示停止）
    if(second & 0x80) {
        // 启动时钟（清除bit7）
        DS1302_WriteByte(0x80, second & 0x7F);
    }
    
    // 设置24小时制（确保小时寄存器的bit7为0）
    hour = DS1302_ReadByte(0x85);
    DS1302_WriteByte(0x84, hour & 0x7F);
    
    // 开启写保护
    DS1302_WriteByte(0x8E, 0x80);
}

// 读取时间
void DS1302_GetTime(DS1302_TIME *time) {
    uchar temp;
    
    // 读取秒（注意是读地址0x81）
    temp = DS1302_ReadByte(0x81);
    time->second = BCD_To_Dec(temp & 0x7F);
    
    // 读取分（注意是读地址0x83）
    temp = DS1302_ReadByte(0x83);
    time->minute = BCD_To_Dec(temp & 0x7F);
    
    // 读取时（注意是读地址0x85，24小时制取低6位）
    temp = DS1302_ReadByte(0x85);
    time->hour = BCD_To_Dec(temp & 0x3F);
    
    // 读取日（注意是读地址0x87）
    temp = DS1302_ReadByte(0x87);
    time->day = BCD_To_Dec(temp & 0x3F);
    
    // 读取月（注意是读地址0x89）
    temp = DS1302_ReadByte(0x89);
    time->month = BCD_To_Dec(temp & 0x1F);
    
    // 读取星期（注意是读地址0x8B）
    temp = DS1302_ReadByte(0x8B);
    time->weekday = BCD_To_Dec(temp & 0x07);
    
    // 读取年（注意是读地址0x8D）
    temp = DS1302_ReadByte(0x8D);
    time->year = BCD_To_Dec(temp);
}

// 设置时间（确保秒为00）
// 修改 DS1302_SetTime 函数，确保写保护正确关闭
void DS1302_SetTime(DS1302_TIME *time) {
    uchar second;
    
    // 关键修复：确保写保护完全关闭
    DS1302_WriteByte(0x8E, 0x00);  // 关闭写保护
    DS1302_Delay(10);              // 增加延时确保写入完成
    
    // 第一步：停止时钟（设置秒寄存器的CH位为1）
    DS1302_WriteByte(0x80, Dec_To_BCD(time->second) | 0x80);  // bit7=1停止时钟
    
    // 第二步：设置其他时间寄存器
    DS1302_WriteByte(0x82, Dec_To_BCD(time->minute));     // 分
    DS1302_WriteByte(0x84, Dec_To_BCD(time->hour) & 0x3F); // 时，24小时制
    DS1302_WriteByte(0x86, Dec_To_BCD(time->day));        // 日
    DS1302_WriteByte(0x88, Dec_To_BCD(time->month));      // 月
    DS1302_WriteByte(0x8A, Dec_To_BCD(time->weekday));    // 星期
    DS1302_WriteByte(0x8C, Dec_To_BCD(time->year));       // 年
    
    // 第三步：启动时钟（清除秒寄存器的CH位）
    DS1302_WriteByte(0x80, Dec_To_BCD(time->second) & 0x7F);  // bit7=0启动时钟
    
    // 关键修复：确保写保护开启
    DS1302_WriteByte(0x8E, 0x80);  // 开启写保护
    DS1302_Delay(10);              // 增加延时确保写入完成
}

// 设置默认时间（2024年5月25日 12:00:00 星期六）
void DS1302_SetDefaultTime(void) {
    DS1302_TIME default_time;
    default_time.year = 24;       // 2024年
    default_time.month = 5;       // 5月
    default_time.day = 25;        // 25日
    default_time.hour = 12;       // 12时
    default_time.minute = 0;      // 0分
    default_time.second = 0;      // 0秒
    default_time.weekday = 7;     // 7 = 星期六（显示为SAT，但仍然用数字7存储）
    
    DS1302_SetTime(&default_time);
}
