// time_log.c
#include "time_log.h"
#include "eeprom.h"
#include "lcd1602.h"
#include "delay.h"

// 星期缩写数组
code char Weekday_Names[8][4] = {
    "",      // 0 - 不使用
    "SUN",   // 1 - 星期日
    "MON",   // 2 - 星期一
    "TUE",   // 3 - 星期二
    "WED",   // 4 - 星期三
    "THU",   // 5 - 星期四
    "FRI",   // 6 - 星期五
    "SAT"    // 7 - 星期六
};

// 初始化时间记录
void TimeLog_Init(void) {
    // uchar i;  // 删除这个未使用的变量
    uchar write_ptr;  // C51要求变量声明在函数开头
    
    DS1302_Init();
    
    // 初始化写指针（如果未初始化）
    write_ptr = EEPROM_ReadByte(WRITE_POINTER_ADDR);
    if(write_ptr >= MAX_LOGS) {
        EEPROM_WriteByte(WRITE_POINTER_ADDR, 0);
    }
}


// 添加新的时间记录（修复循环队列版本）
void TimeLog_AddRecord(void) {
    uchar i, addr;
    TIME_LOG new_log;
    uchar write_pointer;
    
    // 读取当前时间
    DS1302_GetTime(&new_log.time);
    new_log.valid = 1;
    
    // 读取当前写指针
    write_pointer = EEPROM_ReadByte(WRITE_POINTER_ADDR);
    if(write_pointer >= MAX_LOGS) {
        write_pointer = 0;
    }
    
    // 计算存储地址
    addr = LOG_START_ADDR + write_pointer * 8;
    
    // 写入新记录
    EEPROM_WriteByte(addr, new_log.time.year);
    EEPROM_WriteByte(addr + 1, new_log.time.month);
    EEPROM_WriteByte(addr + 2, new_log.time.day);
    EEPROM_WriteByte(addr + 3, new_log.time.hour);
    EEPROM_WriteByte(addr + 4, new_log.time.minute);
    EEPROM_WriteByte(addr + 5, new_log.time.second);
    EEPROM_WriteByte(addr + 6, new_log.time.weekday);
    EEPROM_WriteByte(addr + 7, new_log.valid);
    
    // 更新写指针（循环）
    write_pointer++;
    if(write_pointer >= MAX_LOGS) {
        write_pointer = 0;
    }
    
    // 保存写指针
    EEPROM_WriteByte(WRITE_POINTER_ADDR, write_pointer);
}

// 读取所有时间记录（按时间顺序排列，0=最新）
void TimeLog_ReadRecords(TIME_LOG logs[]) {
    uchar i, j;
    uchar write_pointer;
    uchar year, month, day, hour, minute, second, weekday, valid;
    uchar addr;
    
    // 1. 读取当前写指针
    write_pointer = EEPROM_ReadByte(WRITE_POINTER_ADDR);
    if(write_pointer >= MAX_LOGS) {
        write_pointer = 0;
    }
    
    // 2. 直接按时间顺序读取到logs数组
    j = 0;
    for(i = 0; i < MAX_LOGS; i++) {
        // 计算索引：从最新记录开始
        uchar index = (write_pointer + MAX_LOGS - 1 - i) % MAX_LOGS;
        addr = LOG_START_ADDR + index * 8;
        
        // 读取单个记录
        year = EEPROM_ReadByte(addr);
        month = EEPROM_ReadByte(addr + 1);
        day = EEPROM_ReadByte(addr + 2);
        hour = EEPROM_ReadByte(addr + 3);
        minute = EEPROM_ReadByte(addr + 4);
        second = EEPROM_ReadByte(addr + 5);
        weekday = EEPROM_ReadByte(addr + 6);
        valid = EEPROM_ReadByte(addr + 7);
        
        if(valid) {
            logs[j].time.year = year;
            logs[j].time.month = month;
            logs[j].time.day = day;
            logs[j].time.hour = hour;
            logs[j].time.minute = minute;
            logs[j].time.second = second;
            logs[j].time.weekday = weekday;
            logs[j].valid = valid;
            j++;
        }
    }
    
    // 3. 剩余位置标记为无效
    for(; j < MAX_LOGS; j++) {
        logs[j].valid = 0;
    }
}

// 显示指定记录（display_index: 0=最新，4=最旧）
void TimeLog_DisplayRecord(uchar display_index) {
    TIME_LOG logs[MAX_LOGS];
    TIME_LOG log;
    
    // 读取所有记录（已按时间顺序排列）
    TimeLog_ReadRecords(logs);
    
    // 检查索引是否有效
    if(display_index >= MAX_LOGS || !logs[display_index].valid) {
        LCD_Clear();
        LCD_ShowString(1, 1, "Log ");
        LCD_ShowChar(1, 5, display_index + 1 + '0');
        LCD_ShowString(1, 6, ": No Record");
        LCD_ShowString(2, 1, " ");
        return;
    }
    
    log = logs[display_index];
    
    // 清屏并显示第一行：Log X: 24-05-25
    LCD_Clear();
    
    // 显示记录编号
    LCD_ShowString(1, 1, "Log");
    LCD_ShowChar(1, 4, display_index + 1 + '0');
    LCD_ShowChar(1, 5, ':');
    
    // 显示日期：年份（两位）
    LCD_ShowChar(1, 7, '2');
    LCD_ShowChar(1, 8, '0');
    LCD_ShowChar(1, 9, log.time.year / 10 + '0');
    LCD_ShowChar(1, 10, log.time.year % 10 + '0');
    LCD_ShowChar(1, 11, '-');
    
    // 月份
    LCD_ShowChar(1, 12, log.time.month / 10 + '0');
    LCD_ShowChar(1, 13, log.time.month % 10 + '0');
    LCD_ShowChar(1, 14, '-');
    
    // 日
    LCD_ShowChar(1, 15, log.time.day / 10 + '0');
    LCD_ShowChar(1, 16, log.time.day % 10 + '0');
    
    // 显示第二行：12:00:00 SAT
    // 小时
    LCD_ShowChar(2, 1, log.time.hour / 10 + '0');
    LCD_ShowChar(2, 2, log.time.hour % 10 + '0');
    LCD_ShowChar(2, 3, ':');
    
    // 分钟
    LCD_ShowChar(2, 4, log.time.minute / 10 + '0');
    LCD_ShowChar(2, 5, log.time.minute % 10 + '0');
    LCD_ShowChar(2, 6, ':');
    
    // 秒
    LCD_ShowChar(2, 7, log.time.second / 10 + '0');
    LCD_ShowChar(2, 8, log.time.second % 10 + '0');
    
    // 显示星期缩写
    LCD_ShowChar(2, 10, Weekday_Names[log.time.weekday][0]);
    LCD_ShowChar(2, 11, Weekday_Names[log.time.weekday][1]);
    LCD_ShowChar(2, 12, Weekday_Names[log.time.weekday][2]);
}

// 清除所有记录
void TimeLog_ClearAll(void) {
    uchar i;
    
    for(i = 0; i < MAX_LOGS; i++) {
        uchar addr = LOG_START_ADDR + i * 8 + 7;
        EEPROM_WriteByte(addr, 0);
    }
    
    // 重置写指针
    EEPROM_WriteByte(WRITE_POINTER_ADDR, 0);
}
