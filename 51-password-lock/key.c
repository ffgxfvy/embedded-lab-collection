#include <reg52.h>
#include "key.h"
#include "delay.h"

// 按键引脚定义
#define KEY_PORT P3

// 按键键值定义
uchar code Key_Table[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

// 按键状态变量
static uchar key_pressed = 0;      // 当前按下的键
static uint key_press_time = 0;    // 按键按下时间
static bit key_debouncing = 0;     // 消抖标志

// 按键初始化
void Key_Init(void) {
    KEY_PORT = 0xFF;
}

// 改进的按键扫描（带消抖和状态保持）
uchar Key_Scan(void) {
    uchar i, j;
    uchar key_found = 0;
    static uchar last_key = 0;
    
    // 扫描所有行
    for(i = 0; i < 4; i++) {
        KEY_PORT = 0xF0 | (0xFE << i);
        
        // 检查所有列
        for(j = 0; j < 4; j++) {
            if(!(KEY_PORT & (0x10 << j))) {
                key_found = Key_Table[j][i];
                break;
            }
        }
        
        if(key_found != 0) {
            break;
        }
    }
    
    // 消抖处理
    if(key_found != 0) {
        if(key_debouncing == 0) {
            // 开始消抖
            key_debouncing = 1;
            key_press_time = 0;
            return 0;  // 第一次检测，返回0
        } else {
            key_press_time++;
            if(key_press_time > 5) {  // 消抖时间约50ms
                // 按键稳定按下
                if(key_found == last_key) {
                    // 防止重复触发
                    key_press_time = 6;  // 保持
                    return 0;
                } else {
                    last_key = key_found;
                    return key_found;
                }
            }
            return 0;
        }
    } else {
        // 没有按键
        key_debouncing = 0;
        last_key = 0;
        return 0;
    }
}
