// time_log.h
#ifndef __TIME_LOG_H__
#define __TIME_LOG_H__

// 必须先包含ds1302.h以获取DS1302_TIME定义
#include "ds1302.h"

// 时间记录结构体
typedef struct {
    DS1302_TIME time;  // 这里需要DS1302_TIME已经定义
    uchar valid;
} TIME_LOG;

// 最大记录数
#define MAX_LOGS 5

// EEPROM存储地址
#define LOG_START_ADDR 0x20
#define WRITE_POINTER_ADDR 0x48  // 写指针地址

// 函数声明
void TimeLog_Init(void);
void TimeLog_AddRecord(void);
void TimeLog_ReadRecords(TIME_LOG logs[]);
void TimeLog_DisplayRecord(uchar display_index);
void TimeLog_ClearAll(void);

#endif
