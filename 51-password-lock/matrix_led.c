#include "matrix_led.h"
#include <intrins.h>

// 引脚定义
sbit MATRIX_SER = P2^1;
sbit MATRIX_RCLK = P2^2;  
sbit MATRIX_SRCLK = P2^3;

// 明显的笑脸图案（16×16） - 整体上移版本
// 关键：嘴巴在第12-13行，而不是第14-15行
uchar code smile_data[16][2] = {
    // 头部轮廓（整体上移）
    {0x00, 0x00},  // 第0行
    {0x03, 0xC0},  // 第1行
    {0x0F, 0xF0},  // 第2行
    {0x1F, 0xF8},  // 第3行
    {0x3F, 0xFC},  // 第4行
    {0x7F, 0xFE},  // 第5行
    
    // 眼睛区域
    {0x7F, 0xFE},  // 第6行
    {0x79, 0x9E},  // 第7行：眼睛
    {0x79, 0x9E},  // 第8行：眼睛
    {0x7F, 0xFE},  // 第9行：眼睛下方
    
    // 嘴巴区域
    {0x7F, 0xFE},  // 第10行
    {0x3B, 0xDC},  // 第11行, 微笑
    {0x1C, 0x38},  // 第12行，微笑
    {0x0F, 0xF0},  // 第13行
    {0x03, 0xC0},  // 第14行：底部空白
    {0x00, 0x00}   // 第15行：底部空白
};

static uchar display_row = 0;

// 发送字节
static void Send_Byte(uchar dat) {
    uchar i;
    for(i = 0; i < 8; i++) {
        MATRIX_SER = dat & 0x80;
        dat <<= 1;
        MATRIX_SRCLK = 1;
        _nop_();
        MATRIX_SRCLK = 0;
    }
}

// 初始化
void Matrix_LED_Init(void) {
    MATRIX_SER = 0;
    MATRIX_RCLK = 0;
    MATRIX_SRCLK = 0;
    display_row = 0;
}

// 显示笑脸
void Display_Smile(void) {
    uchar col1, col2;
    
    col1 = smile_data[display_row][0];
    col2 = smile_data[display_row][1];
    
    // 重要：如果显示不对，交换下面两行
    Send_Byte(col1);  // 不加~
    Send_Byte(col2);
    
    // 发送行选择
    if(display_row < 8) {
        Send_Byte(0x01 << display_row);
        Send_Byte(0x00);
    } else {
        Send_Byte(0x00);
        Send_Byte(0x01 << (display_row - 8));
    }
    
    // 锁存
    MATRIX_RCLK = 1;
    _nop_();
    MATRIX_RCLK = 0;
    
    // 更新行号
    display_row++;
    if(display_row >= 16) {
        display_row = 0;
    }
}

// 清除显示
void Clear_Matrix(void) {
    uchar i;
    
    for(i = 0; i < 8; i++) {
        Send_Byte(0xFF);
        Send_Byte(0xFF);
        Send_Byte(0x01 << i);
        Send_Byte(0x00);
        MATRIX_RCLK = 1;
        _nop_();
        MATRIX_RCLK = 0;
    }
    
    for(i = 0; i < 8; i++) {
        Send_Byte(0xFF);
        Send_Byte(0xFF);
        Send_Byte(0x00);
        Send_Byte(0x01 << i);
        MATRIX_RCLK = 1;
        _nop_();
        MATRIX_RCLK = 0;
    }
    
    display_row = 0;
}
