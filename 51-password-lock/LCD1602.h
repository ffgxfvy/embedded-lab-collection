#ifndef __LCD1602_H__
#define __LCD1602_H__

#include <reg52.h>
#include "delay.h"  // 添加这行

#define uchar unsigned char
#define uint unsigned int

// LCD1602接口定义
sbit LCD_RS = P2^6;
sbit LCD_RW = P2^5;
sbit LCD_EN = P2^7;
#define LCD_DATA P0

// 函数声明
void LCD_Init(void);
void LCD_Write_Cmd(uchar cmd);
void LCD_Write_Data(uchar dat);
void LCD_Clear(void);
void LCD_Set_Cursor(uchar line, uchar col);
void LCD_ShowString(uchar line, uchar col, uchar *str);
//void LCD_ShowNum(uchar line, uchar col, uint num, uchar len);
void LCD_ShowChar(uchar line, uchar col, uchar ch);

// 删除原来的 delay_ms 声明

#endif
