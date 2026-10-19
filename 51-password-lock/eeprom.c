// eeprom.c - 24C02 EEPROM驱动
#include <reg52.h>
#include <intrins.h>

// 使用与main.c相同的类型定义
#define uchar unsigned char
#define uint unsigned int

// 引脚定义
sbit EE_SCL = P1^3;  // I2C时钟线
sbit EE_SDA = P1^4;  // I2C数据线

// 24C02设备地址
#define EEPROM_ADDR_WRITE 0xA0
#define EEPROM_ADDR_READ  0xA1

// 简单延时函数
static void delay_us(uint us) {
    while(us--) {
        _nop_(); _nop_(); _nop_(); _nop_();
        _nop_(); _nop_(); _nop_(); _nop_();
    }
}

static void delay_ms(uint ms) {
    uint i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 112; j++);
}

// I2C初始化
void I2C_Init(void) {
    EE_SCL = 1;
    EE_SDA = 1;
}

// I2C起始信号
static void I2C_Start(void) {
    EE_SDA = 1;
    EE_SCL = 1;
    delay_us(5);
    EE_SDA = 0;
    delay_us(5);
    EE_SCL = 0;
}

// I2C停止信号
static void I2C_Stop(void) {
    EE_SDA = 0;
    EE_SCL = 1;
    delay_us(5);
    EE_SDA = 1;
    delay_us(5);
}

// I2C发送一个字节
static void I2C_SendByte(uchar dat) {
    uchar i;
    
    for(i = 0; i < 8; i++) {
        EE_SDA = (dat & 0x80) ? 1 : 0;
        dat <<= 1;
        delay_us(2);
        EE_SCL = 1;
        delay_us(5);
        EE_SCL = 0;
        delay_us(2);
    }
}

// I2C读取一个字节
static uchar I2C_ReadByte(void) {
    uchar i, dat = 0;
    
    EE_SDA = 1;
    
    for(i = 0; i < 8; i++) {
        dat <<= 1;
        EE_SCL = 1;
        delay_us(5);
        if(EE_SDA) dat |= 0x01;
        EE_SCL = 0;
        delay_us(2);
    }
    
    return dat;
}

// 接收应答
static bit I2C_ReceiveAck(void) {
    bit ack;
    
    EE_SDA = 1;
    EE_SCL = 1;
    delay_us(5);
    ack = EE_SDA;
    EE_SCL = 0;
    delay_us(2);
    
    return ack;
}

// 发送应答
static void I2C_SendAck(bit ack) {
    EE_SDA = ack;
    delay_us(2);
    EE_SCL = 1;
    delay_us(5);
    EE_SCL = 0;
    delay_us(2);
}

// 保存密码到EEPROM
void EEPROM_SavePassword(uchar *pwd) {
    uchar i;
    uchar buffer[8];
    
    for(i = 0; i < 6; i++) {
        buffer[i] = pwd[i];
    }
    buffer[6] = 0xAA;
    buffer[7] = 0x55;
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_ReceiveAck();
    I2C_SendByte(0x00);
    I2C_ReceiveAck();
    
    for(i = 0; i < 8; i++) {
        I2C_SendByte(buffer[i]);
        I2C_ReceiveAck();
    }
    
    I2C_Stop();
    delay_ms(10);
}

// 从EEPROM读取密码
void EEPROM_ReadPassword(uchar *pwd) {
    uchar i;
    uchar buffer[8];
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_ReceiveAck();
    I2C_SendByte(0x00);
    I2C_ReceiveAck();
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_READ);
    I2C_ReceiveAck();
    
    for(i = 0; i < 8; i++) {
        buffer[i] = I2C_ReadByte();
        if(i < 7) {
            I2C_SendAck(0);
        } else {
            I2C_SendAck(1);
        }
    }
    
    I2C_Stop();
    
    if(buffer[6] == 0xAA && buffer[7] == 0x55) {
        for(i = 0; i < 6; i++) {
            pwd[i] = buffer[i];
        }
    } else {
        pwd[0] = 1; pwd[1] = 2; pwd[2] = 3;
        pwd[3] = 4; pwd[4] = 5; pwd[5] = 6;
    }
}

// 清除EEPROM中的密码
void EEPROM_ClearPassword(void) {
    uchar i;
    uchar buffer[8] = {0};
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_ReceiveAck();
    I2C_SendByte(0x00);
    I2C_ReceiveAck();
    
    for(i = 0; i < 8; i++) {
        I2C_SendByte(buffer[i]);
        I2C_ReceiveAck();
    }
    
    I2C_Stop();
    delay_ms(10);
}

// 读取EEPROM一个字节（新增函数）
uchar EEPROM_ReadByte(uchar addr) {
    uchar dat;
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_ReceiveAck();
    I2C_SendByte(addr);
    I2C_ReceiveAck();
    
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_READ);
    I2C_ReceiveAck();
    
    dat = I2C_ReadByte();
    I2C_SendAck(1);
    I2C_Stop();
    
    return dat;
}

// 写入EEPROM一个字节（新增函数）
void EEPROM_WriteByte(uchar addr, uchar dat) {
    I2C_Start();
    I2C_SendByte(EEPROM_ADDR_WRITE);
    I2C_ReceiveAck();
    I2C_SendByte(addr);
    I2C_ReceiveAck();
    I2C_SendByte(dat);
    I2C_ReceiveAck();
    I2C_Stop();
    delay_ms(10);
}
