#ifndef __EEPROM_H__
#define __EEPROM_H__

// 不需要包含reg52.h，因为在main.c中已经包含了
// 使用标准类型定义
#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint  
#define uint unsigned int
#endif

// 函数声明（使用标准C函数原型）
void I2C_Init(void);
void EEPROM_SavePassword(uchar *pwd);
void EEPROM_ReadPassword(uchar *pwd);
void EEPROM_ClearPassword(void);

// 新增的函数声明
uchar EEPROM_ReadByte(uchar addr);
void EEPROM_WriteByte(uchar addr, uchar dat);

#endif
