#include "lcd1602.h"
#include "delay.h"

// 写指令
void LCD_Write_Cmd(uchar cmd) {
    LCD_RS = 0;
    LCD_RW = 0;
    LCD_DATA = cmd;
    LCD_EN = 1;
    delay_us(5);
    LCD_EN = 0;
    delay_us(5);
}

// 写数据（只有一个参数）
void LCD_Write_Data(uchar dat) {
    LCD_RS = 1;
    LCD_RW = 0;
    LCD_DATA = dat;
    LCD_EN = 1;
    delay_us(5);
    LCD_EN = 0;
    delay_us(5);
}

// LCD初始化
void LCD_Init(void) {
    delay_ms(15);
    LCD_Write_Cmd(0x38);
    delay_ms(5);
    LCD_Write_Cmd(0x38);
    delay_ms(5);
    LCD_Write_Cmd(0x38);
    
    LCD_Write_Cmd(0x0C);
    LCD_Write_Cmd(0x06);
    LCD_Write_Cmd(0x01);
    delay_ms(5);
}

// 清屏
void LCD_Clear(void) {
    LCD_Write_Cmd(0x01);
    delay_ms(5);
}

// 设置光标位置
void LCD_Set_Cursor(uchar line, uchar col) {
    uchar pos;
    if(line == 1) {
        pos = 0x80 + col - 1;
    } else {
        pos = 0xC0 + col - 1;
    }
    LCD_Write_Cmd(pos);
}

// 显示字符串
void LCD_ShowString(uchar line, uchar col, uchar *str) {
    LCD_Set_Cursor(line, col);
    while(*str != '\0') {
        LCD_Write_Data(*str++);
    }
}

// 显示数字
//void LCD_ShowNum(uchar line, uchar col, uint num, uchar len) {
//    uchar i;
//    uchar str[5];
//    
//    for(i = 0; i < len; i++) {
//        str[len-1-i] = num % 10 + '0';
//        num /= 10;
//    }
//    
//    LCD_Set_Cursor(line, col);
//    for(i = 0; i < len; i++) {
//        LCD_Write_Data(str[i]);
//    }
//}

// 显示单个字符在指定位置
void LCD_ShowChar(uchar line, uchar col, uchar ch) {
    LCD_Set_Cursor(line, col);
    LCD_Write_Data(ch);
}
