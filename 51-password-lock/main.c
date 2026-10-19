// main.c - 密码锁主程序
#include <reg52.h>

// 首先定义类型（在所有头文件之前）
#define uchar unsigned char
#define uint unsigned int

// 然后包含其他头文件（必须先包含定义了DS1302_TIME的头文件）
#include "ds1302.h"      // DS1302头文件，定义了DS1302_TIME结构体
#include "lcd1602.h"
#include "matrix_led.h"
#include "key.h"
#include "delay.h"
#include "eeprom.h"      // EEPROM头文件
#include "time_log.h"    // 时间记录头文件

// 使用idata扩展RAM（89C52有256字节内部RAM）
// 将大数组移到idata区域
idata uchar password[6];
idata uchar stored_password[6];
idata uchar temp_time_input[2];

// 时间设置相关变量（移到idata）
idata DS1302_TIME setting_time;  // 现在DS1302_TIME已经定义了

// 引脚定义
sbit BUZZER = P1^5;
sbit LED = P2^0;

// 全局变量（使用默认data区，尽量少）
uchar input_count = 0;
uchar current_state = 0;
uint state_timer = 0;
uchar last_key = 0;
uchar key_repeat = 0;
bit matrix_showing = 0;
bit key_scan_flag = 0;

// 时间查看相关变量
uchar view_log_index = 0;
bit view_log_mode = 0;

// 时间设置相关变量
uchar time_setting_step = 0;     // 时间设置步骤
uchar temp_time_count = 0;       // 临时输入计数

// 状态定义
#define STATE_IDLE       0
#define STATE_CONFIRM    1
#define STATE_SETTING    2
#define STATE_UNLOCKED   3
#define STATE_ERROR      4
#define STATE_SET_OK     5
#define STATE_VIEW_LOG   6
#define STATE_SET_TIME   7

// 函数声明
void System_Init(void);
void Buzzer_Beep(uchar ms);
void Show_Password(void);
void Process_State(void);
void Reset_System(void);
void Timer0_Init(void);
void Save_New_Password(void);
void Show_CurrentTime(void);
void Show_TimeSetting_Menu(void);
void Enter_TimeSetting_Mode(void);
void Process_TimeSetting(uchar key_val);

// 主函数
void main() {
    uchar key_val;
    uchar i;
    
    // 初始化idata数组
    for(i = 0; i < 6; i++) {
        password[i] = 0;
        stored_password[i] = i + 1;  // 默认密码123456
    }
    
    // 初始化时间设置变量
    setting_time.year = 24;
    setting_time.month = 1;
    setting_time.day = 1;
    setting_time.hour = 12;
    setting_time.minute = 0;
    setting_time.second = 0;
    setting_time.weekday = 1;
    
    System_Init();
    Timer0_Init();
    Reset_System();
    
    while(1) {
        Process_State();
        
        if(key_scan_flag) {
            key_scan_flag = 0;
            key_val = Key_Scan();
            
            if(key_val != 0) {
                if(key_val == last_key) {
                    key_repeat++;
                    if(key_repeat < 2) {
                        key_val = 0;
                    }
                } else {
                    last_key = key_val;
                    key_repeat = 1;
                }
                
                if(key_val != 0) {
                    Buzzer_Beep(10);
                    
                    // 如果在查看记录模式
                    if(view_log_mode) {
                        switch(key_val) {
                            case 'A':  // 上一条记录
                                if(view_log_index > 0) {
                                    view_log_index--;
                                } else {
                                    view_log_index = MAX_LOGS - 1;
                                }
                                TimeLog_DisplayRecord(view_log_index);
                                state_timer = 0;  // 重置超时计时器
                                break;
                                
                            case 'B':  // 下一条记录
                                view_log_index++;
                                if(view_log_index >= MAX_LOGS) {
                                    view_log_index = 0;
                                }
                                TimeLog_DisplayRecord(view_log_index);
                                state_timer = 0;
                                break;
                                
                            case 'C':  // 清除所有记录
                                TimeLog_ClearAll();
                                LCD_Clear();
                                LCD_ShowString(1, 1, "All Logs Cleared");
                                delay_ms(1000);
                                view_log_index = 0;
                                TimeLog_DisplayRecord(view_log_index);
                                state_timer = 0;
                                break;
                                
                            case 'D':  // 退出查看模式
                                view_log_mode = 0;
                                Reset_System();
                                break;
                                
                            case '1':
                            case '2':
                            case '3':
                            case '4':
                            case '5':
                                view_log_index = key_val - '1';
                                TimeLog_DisplayRecord(view_log_index);
                                state_timer = 0;
                                break;
                                
                            default:
                                // 其他按键忽略，不触发任何操作
                                break;
                        }
                        continue;  // 跳过后续的密码处理逻辑
                    }
                    
                    // 如果在时间设置模式
                    if(current_state == STATE_SET_TIME) {
                        Process_TimeSetting(key_val);
                        continue;  // 跳过后续的密码处理逻辑
                    }
                    
                    switch(current_state) {
                        case STATE_IDLE:
                            if(key_val >= '0' && key_val <= '9') {
                                if(input_count < 6) {
                                    password[input_count] = key_val - '0';
                                    input_count++;
                                    Show_Password();
                                    
                                    if(input_count == 6) {
                                        bit match = 1;
                                        for(i = 0; i < 6; i++) {
                                            if(password[i] != stored_password[i]) {
                                                match = 0;
                                                break;
                                            }
                                        }
                                        
                                        if(match) {
                                            // 立即更新显示
                                            LED = 0;  // 立即点亮LED
                                            LCD_Clear();
                                            LCD_ShowString(1, 1, "Unlock Success!");
                                            LCD_ShowString(2, 1, "Press any key..");
                                            
                                            // 设置状态变量
                                            current_state = STATE_UNLOCKED;
                                            state_timer = 0;
                                            matrix_showing = 1;
                                            
                                            // 添加时间记录
                                            TimeLog_AddRecord();
                                            
                                            // 立即执行一次状态处理
                                            Process_State();
                                        } else {
                                            current_state = STATE_ERROR;
                                            state_timer = 0;
                                        }
                                    }
                                }
                            } else {
                                switch(key_val) {
                                    case 'A':
                                        LCD_Clear();
                                        LCD_ShowString(1, 1, "Verify Old Pwd:");
                                        LCD_ShowString(2, 1, "Old:           ");
                                        current_state = STATE_CONFIRM;
                                        input_count = 0;
                                        for(i = 0; i < 6; i++) password[i] = 0;
                                        break;
                                    case 'C':
                                        input_count = 0;
                                        for(i = 0; i < 6; i++) password[i] = 0;
                                        Show_Password();
                                        break;
                                    case 'D':
                                        Reset_System();
                                        break;
                                    case 'B':
                                        matrix_showing = 1;
                                        current_state = STATE_UNLOCKED;
                                        state_timer = 0;
                                        break;
                                    case '#':  // 修改：进入时间设置
                                        if(input_count == 0) {
                                            // 空闲状态下按#进入时间设置
                                            Enter_TimeSetting_Mode();
                                        } else {
                                            // 输入密码时按#清除输入
                                            input_count = 0;
                                            for(i = 0; i < 6; i++) password[i] = 0;
                                            Show_Password();
                                        }
                                        break;
                                    // 在按键处理中添加提示信息
                                    case '*':  // 查看时间记录
                                        current_state = STATE_VIEW_LOG;
                                        state_timer = 0;
                                        view_log_mode = 1;
                                        view_log_index = 0;
                                        LCD_Clear();
                                        LCD_ShowString(1, 1, "Time Logs");
                                        LCD_ShowString(2, 1, "A:Prev B:Next");
                                        delay_ms(1500);  // 稍长一点时间看提示
                                        LCD_Clear();
                                        TimeLog_DisplayRecord(view_log_index);
                                        break;
                                }
                            }
                            break;
                            
                        case STATE_CONFIRM:
                            if(key_val >= '0' && key_val <= '9') {
                                if(input_count < 6) {
                                    password[input_count] = key_val - '0';
                                    input_count++;
                                    Show_Password();
                                    
                                    if(input_count == 6) {
                                        bit match = 1;
                                        for(i = 0; i < 6; i++) {
                                            if(password[i] != stored_password[i]) {
                                                match = 0;
                                                break;
                                            }
                                        }
                                        
                                        if(match) {
                                            current_state = STATE_SETTING;
                                            input_count = 0;
                                            for(i = 0; i < 6; i++) password[i] = 0;
                                            LCD_Clear();
                                            LCD_ShowString(1, 1, "Set New Password:");
                                            LCD_ShowString(2, 1, "New:           ");
                                        } else {
                                            current_state = STATE_ERROR;
                                            state_timer = 0;
                                        }
                                    }
                                }
                            } else if(key_val == 'C') {
                                input_count = 0;
                                for(i = 0; i < 6; i++) password[i] = 0;
                                Show_Password();
                            } else if(key_val == 'D') {
                                Reset_System();
                            }
                            break;
                            
                        case STATE_SETTING:
                            if(key_val >= '0' && key_val <= '9') {
                                if(input_count < 6) {
                                    password[input_count] = key_val - '0';
                                    input_count++;
                                    Show_Password();
                                    
                                    if(input_count == 6) {
                                        Save_New_Password();
                                        current_state = STATE_SET_OK;
                                        state_timer = 0;
                                        matrix_showing = 0;
                                    }
                                }
                            } else if(key_val == 'C') {
                                input_count = 0;
                                for(i = 0; i < 6; i++) password[i] = 0;
                                Show_Password();
                            } else if(key_val == 'D') {
                                Reset_System();
                            }
                            break;
                            
                        default:
                            matrix_showing = 0;
                            Clear_Matrix();
                            Reset_System();
                            break;
                    }
                }
            }
        }
    }
}

// 定时器0中断
void Timer0_ISR(void) interrupt 1 {
    static uchar timer_1ms = 0;
    static uchar timer_10ms = 0;
    
    TH0 = 0xFC;
    TL0 = 0x66;
    
    if(matrix_showing) Display_Smile();
    
    timer_1ms++;
    if(timer_1ms >= 10) {
        timer_1ms = 0;
        timer_10ms++;
        key_scan_flag = 1;
    }
    
    if(timer_10ms >= 10) {
        timer_10ms = 0;
        state_timer++;
    }
}

// 初始化定时器0
void Timer0_Init(void) {
    TMOD &= 0xF0;
    TMOD |= 0x01;
    TH0 = 0xFC;
    TL0 = 0x66;
    ET0 = 1;
    TR0 = 1;
    EA = 1;
}

// 显示当前时间
void Show_CurrentTime(void) {
    DS1302_TIME current_time;
    DS1302_GetTime(&current_time);
    
    LCD_Clear();
    LCD_ShowString(1, 1, "Current Time:");
    
    // 显示日期
    LCD_ShowChar(2, 1, '2');
    LCD_ShowChar(2, 2, '0');
    LCD_ShowChar(2, 3, current_time.year / 10 + '0');
    LCD_ShowChar(2, 4, current_time.year % 10 + '0');
    LCD_ShowChar(2, 5, '-');
    LCD_ShowChar(2, 6, current_time.month / 10 + '0');
    LCD_ShowChar(2, 7, current_time.month % 10 + '0');
    LCD_ShowChar(2, 8, '-');
    LCD_ShowChar(2, 9, current_time.day / 10 + '0');
    LCD_ShowChar(2, 10, current_time.day % 10 + '0');
    LCD_ShowChar(2, 11, ' ');
    
    // 显示时间
    LCD_ShowChar(2, 12, current_time.hour / 10 + '0');
    LCD_ShowChar(2, 13, current_time.hour % 10 + '0');
    LCD_ShowChar(2, 14, ':');
    LCD_ShowChar(2, 15, current_time.minute / 10 + '0');
    LCD_ShowChar(2, 16, current_time.minute % 10 + '0');
}

// 时间设置菜单显示（修改后的顺序）
void Show_TimeSetting_Menu(void) {
    uchar i;
    
    switch(time_setting_step) {
        case 0:  // 设置星期
            LCD_Clear();
            LCD_ShowString(1, 1, "Set Weekday (1-7):");
            LCD_ShowString(2, 1, "1=Sun,7=Sat");
            if(temp_time_count >= 1) {
                LCD_ShowChar(2, 13, temp_time_input[0] + '0');
            } else {
                LCD_ShowChar(2, 13, '_');
            }
            break;
            
        case 1:  // 设置年
            LCD_Clear();
            LCD_ShowString(1, 1, "Set Year (00-99):");
            LCD_ShowString(2, 1, "20");
            if(temp_time_count >= 1) LCD_ShowChar(2, 3, temp_time_input[0] + '0');
            if(temp_time_count >= 2) LCD_ShowChar(2, 4, temp_time_input[1] + '0');
            for(i = temp_time_count; i < 2; i++) {
                LCD_ShowChar(2, 3 + i, '_');
            }
            break;
            
        case 2:  // 设置月
            LCD_Clear();
            LCD_ShowString(1, 1, "Set Month (01-12):");
            if(temp_time_count >= 1) LCD_ShowChar(2, 1, temp_time_input[0] + '0');
            if(temp_time_count >= 2) LCD_ShowChar(2, 2, temp_time_input[1] + '0');
            for(i = temp_time_count; i < 2; i++) {
                LCD_ShowChar(2, 1 + i, '_');
            }
            break;
            
        case 3:  // 设置日
            LCD_Clear();
            LCD_ShowString(1, 1, "Set Day (01-31):");
            if(temp_time_count >= 1) LCD_ShowChar(2, 1, temp_time_input[0] + '0');
            if(temp_time_count >= 2) LCD_ShowChar(2, 2, temp_time_input[1] + '0');
            for(i = temp_time_count; i < 2; i++) {
                LCD_ShowChar(2, 1 + i, '_');
            }
            break;
            
        case 4:  // 设置时
            LCD_Clear();
            LCD_ShowString(1, 1, "Set Hour (00-23):");
            if(temp_time_count >= 1) LCD_ShowChar(2, 1, temp_time_input[0] + '0');
            if(temp_time_count >= 2) LCD_ShowChar(2, 2, temp_time_input[1] + '0');
            for(i = temp_time_count; i < 2; i++) {
                LCD_ShowChar(2, 1 + i, '_');
            }
            break;
            
        case 5:  // 设置分
            LCD_Clear();
            LCD_ShowString(1, 1, "Set Minute (00-59):");
            if(temp_time_count >= 1) LCD_ShowChar(2, 1, temp_time_input[0] + '0');
            if(temp_time_count >= 2) LCD_ShowChar(2, 2, temp_time_input[1] + '0');
            for(i = temp_time_count; i < 2; i++) {
                LCD_ShowChar(2, 1 + i, '_');
            }
            break;
            
        case 6:  // 确认时间（秒自动设为00）
            LCD_Clear();
            LCD_ShowString(1, 1, "Confirm Time:");
            LCD_ShowChar(2, 1, '2');
            LCD_ShowChar(2, 2, '0');
            LCD_ShowChar(2, 3, setting_time.year / 10 + '0');
            LCD_ShowChar(2, 4, setting_time.year % 10 + '0');
            LCD_ShowChar(2, 5, '-');
            LCD_ShowChar(2, 6, setting_time.month / 10 + '0');
            LCD_ShowChar(2, 7, setting_time.month % 10 + '0');
            LCD_ShowChar(2, 8, '-');
            LCD_ShowChar(2, 9, setting_time.day / 10 + '0');
            LCD_ShowChar(2, 10, setting_time.day % 10 + '0');
            LCD_ShowChar(2, 11, ' ');
            LCD_ShowChar(2, 12, setting_time.hour / 10 + '0');
            LCD_ShowChar(2, 13, setting_time.hour % 10 + '0');
            LCD_ShowChar(2, 14, ':');
            LCD_ShowChar(2, 15, setting_time.minute / 10 + '0');
            LCD_ShowChar(2, 16, setting_time.minute % 10 + '0');
            break;
    }
}

// 进入时间设置模式
void Enter_TimeSetting_Mode(void) {
    current_state = STATE_SET_TIME;
    time_setting_step = 0;  // 从星期开始
    temp_time_count = 0;
    
    // 获取当前时间作为初始值
    DS1302_GetTime(&setting_time);
    setting_time.second = 0;  // 秒自动设为00
    
    // 显示第一个设置项
    Show_TimeSetting_Menu();
}

// 处理时间设置（修改后的顺序）
void Process_TimeSetting(uchar key_val) {
    uchar temp_value;
    
    switch(key_val) {
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9':
            if(time_setting_step == 0) {  // 设置星期，只需要1位
                if(temp_time_count < 1) {
                    temp_time_input[temp_time_count] = key_val - '0';
                    temp_time_count++;
                    Show_TimeSetting_Menu();
                }
            } else {  // 其他设置需要2位
                if(temp_time_count < 2) {
                    temp_time_input[temp_time_count] = key_val - '0';
                    temp_time_count++;
                    Show_TimeSetting_Menu();
                    
                    // 如果输入完2位，自动进入下一步
                    if(time_setting_step != 0 && temp_time_count >= 2) {
                        // 保存当前项的值
                        temp_value = temp_time_input[0] * 10 + temp_time_input[1];
                        
                        switch(time_setting_step) {
                            case 1: setting_time.year = temp_value; break;
                            case 2: setting_time.month = temp_value; break;
                            case 3: setting_time.day = temp_value; break;
                            case 4: setting_time.hour = temp_value; break;
                            case 5: setting_time.minute = temp_value; break;
                        }
                        
                        // 清空输入，进入下一步
                        temp_time_count = 0;
                        time_setting_step++;
                        Show_TimeSetting_Menu();
                    }
                }
            }
            break;
            
        case 'A':  // 确认/下一步
            if(time_setting_step == 0) {  // 设置星期
                if(temp_time_count >= 1) {
                    setting_time.weekday = temp_time_input[0];
                    temp_time_count = 0;
                    time_setting_step++;  // 进入年份设置
                    Show_TimeSetting_Menu();
                }
            } else if(time_setting_step == 6) {  // 确认页面，保存时间
                // 秒已经自动设为00
                setting_time.second = 0;
                
                // 保存到DS1302
                DS1302_SetTime(&setting_time);
                
                LCD_Clear();
                LCD_ShowString(1, 1, "Time Saved!");
                LCD_ShowString(2, 1, "Auto return...");
                delay_ms(1500);
                
                // 返回空闲状态
                Reset_System();
            } else {
                // 其他步骤，手动进入下一步（如果用户不想输入完整2位）
                if(temp_time_count > 0) {
                    // 处理当前输入
                    if(time_setting_step == 0) {
                        setting_time.weekday = temp_time_input[0];
                    } else {
                        temp_value = 0;
                        if(temp_time_count == 1) {
                            temp_value = temp_time_input[0];
                        } else if(temp_time_count == 2) {
                            temp_value = temp_time_input[0] * 10 + temp_time_input[1];
                        }
                        
                        switch(time_setting_step) {
                            case 1: setting_time.year = temp_value; break;
                            case 2: setting_time.month = temp_value; break;
                            case 3: setting_time.day = temp_value; break;
                            case 4: setting_time.hour = temp_value; break;
                            case 5: setting_time.minute = temp_value; break;
                        }
                    }
                    
                    // 清空输入，进入下一步
                    temp_time_count = 0;
                    time_setting_step++;
                    Show_TimeSetting_Menu();
                }
            }
            break;
            
        case 'C':  // 清除当前输入
            temp_time_count = 0;
            Show_TimeSetting_Menu();
            break;
            
        case 'D':  // 取消/上一步
            if(time_setting_step > 0) {
                time_setting_step--;
                temp_time_count = 0;
                Show_TimeSetting_Menu();
            } else {
                // 退出时间设置
                Reset_System();
            }
            break;
            
        default:
            // 其他按键忽略
            break;
    }
}

// 状态处理
void Process_State(void) {
    static uchar last_state = 0xFF;
    
    if(current_state != last_state) {
        last_state = current_state;
        
        switch(current_state) {
            case STATE_IDLE:
                LED = 1;
                matrix_showing = 0;
                Clear_Matrix();
                LCD_Clear();
                LCD_ShowString(1, 1, "Input Password:");
                LCD_ShowString(2, 1, "Pwd:           ");
                break;
                
            case STATE_UNLOCKED:
                // 这里已经在上面的密码验证部分立即显示了
                // 所以这里可以留空或者只设置变量
                break;
                
            case STATE_ERROR:
                LED = 0;
                Buzzer_Beep(100);
                LCD_Clear();
                LCD_ShowString(1, 1, "Wrong Password!");
                LCD_ShowString(2, 1, "Press any key..");
                delay_ms(100);
                LED = 1;
                matrix_showing = 0;
                Clear_Matrix();
                break;
                
            case STATE_SET_OK:
                LED = 0;
                LCD_Clear();
                LCD_ShowString(1, 1, "Password Saved!");
                LCD_ShowString(2, 1, "Press any key..");
                delay_ms(100);
                LED = 1;
                matrix_showing = 0;
                Clear_Matrix();
                break;
                
            case STATE_VIEW_LOG:
                LED = 1;
                matrix_showing = 0;
                Clear_Matrix();
                break;
                
            case STATE_SET_TIME:
                LED = 1;
                matrix_showing = 0;
                Clear_Matrix();
                break;
                
            default:
                break;
        }
    }
    
    // 状态超时处理
    if(current_state == STATE_ERROR || current_state == STATE_SET_OK) {
        if(state_timer > 100) {
            Reset_System();
        }
    }
    
    // 查看记录状态超时返回
    if(view_log_mode && current_state == STATE_VIEW_LOG) {
        if(state_timer > 300) {  // 30秒无操作自动退出
            view_log_mode = 0;
            Reset_System();
        }
    }
    
    // 解锁状态超时处理
    if(current_state == STATE_UNLOCKED) {
        if(state_timer > 150) {  // 15秒后自动返回
            Reset_System();
        }
    }
    
    // 注意：时间设置状态没有超时处理，用户可以慢慢设置
    // 移除原来的时间设置超时处理代码
}

// 显示密码
void Show_Password(void) {
    uchar i;
    
    if(current_state == STATE_SETTING) {
        LCD_ShowString(2, 1, "New: ");
    } else if(current_state == STATE_CONFIRM) {
        LCD_ShowString(2, 1, "Old: ");
    } else {
        LCD_ShowString(2, 1, "Pwd: ");
    }
    
    for(i = 0; i < input_count; i++) {
        LCD_ShowChar(2, 6 + i, password[i] + '0');
    }
    
    for(i = input_count; i < 6; i++) {
        LCD_ShowChar(2, 6 + i, ' ');
    }
}

// 保存新密码
void Save_New_Password(void) {
    uchar i;
    EEPROM_SavePassword(password);
    for(i = 0; i < 6; i++) {
        stored_password[i] = password[i];
    }
}

// 重置系统
void Reset_System(void) {
    uchar i;
    
    current_state = STATE_IDLE;
    input_count = 0;
    state_timer = 0;
    matrix_showing = 0;
    view_log_mode = 0;
    view_log_index = 0;
    time_setting_step = 0;
    temp_time_count = 0;
    
    // 初始化idata数组
    for(i = 0; i < 6; i++) {
        password[i] = 0;
    }
    
    LED = 1;
    Clear_Matrix();
    
    LCD_Clear();
    LCD_ShowString(1, 1, "Input Password:");
    LCD_ShowString(2, 1, "Pwd:           ");
}

// 系统初始化
void System_Init(void) {
    uchar i;
    DS1302_TIME test_time;
    
    BUZZER = 1;
    LED = 1;
    
    LED = 0;
    Buzzer_Beep(50);
    LED = 1;
    
    LCD_Init();
    Matrix_LED_Init();
    Key_Init();
    I2C_Init();
    
    // 初始化时间记录模块
    TimeLog_Init();
    
    // 读取EEPROM密码
    EEPROM_ReadPassword(stored_password);
    
    // 显示系统信息
    LCD_Clear();
    LCD_ShowString(1, 1, "Password Lock");
    LCD_ShowString(2, 1, "Time Log System");
    delay_ms(1500);
    
    Reset_System();
}

// 蜂鸣器提示音
void Buzzer_Beep(uchar ms) {
    BUZZER = 0;
    delay_ms(ms);
    BUZZER = 1;
}
