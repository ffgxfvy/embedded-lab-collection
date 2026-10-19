#ifndef __DS1302_H__
#define __DS1302_H__

#include <reg52.h>

#ifndef uchar
#define uchar unsigned char
#endif

#ifndef uint
#define uint unsigned int
#endif

// DS1302引脚定义
sbit DS1302_CLK = P1^0;
sbit DS1302_IO  = P1^1;
sbit DS1302_CE  = P1^2;

// 时间结构体
typedef struct {
    uchar year;    // 年 00-99
    uchar month;   // 月 1-12
    uchar day;     // 日 1-31
    uchar hour;    // 时 0-23
    uchar minute;  // 分 0-59
    uchar second;  // 秒 0-59
    uchar weekday; // 星期 1-7
} DS1302_TIME;

// 函数声明
void DS1302_Init(void);
void DS1302_GetTime(DS1302_TIME *time);
void DS1302_SetTime(DS1302_TIME *time);      // 确保有这个函数
void DS1302_SetDefaultTime(void);            // 可选

#endif
