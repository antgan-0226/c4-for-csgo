#ifndef LCD1602_H
#define LCD1602_H

#include "stm32f10x_gpio.h"

/*********************************** 引脚定义 ********************************************/
#define BUSY 0x80        // LCD忙标志位（D7位为1时表示忙）
#define RS GPIO_Pin_12   // PB12引脚定义为RS（寄存器选择信号）
#define RW GPIO_Pin_13   // PB13引脚定义为RW（读写选择信号）
#define EN GPIO_Pin_14   // PB14引脚定义为EN（使能信号）
// PA0~PA7定义为数据总线D0~D7，用于传输命令和数据
/*********************************** 引脚定义 ********************************************/

/*********************************** 函数声明 ********************************************/
void ReadBusy(void);              // 读取LCD忙标志，等待LCD空闲
void LCD_WRITE_CMD(unsigned char CMD);  // 向LCD写入命令
void LCD_SetCursor(unsigned char Column);  // 设置LCD光标位置
void LCD_WRITE_StrDATA(unsigned char *StrData, unsigned char col);  // 显示字符串
void LCD_WRITE_ByteDATA(unsigned char ByteData);  // 向LCD写入数据字节
void LCD_INIT(void);                // 初始化LCD模块
void GPIO_INIT(void);               // 初始化LCD相关GPIO引脚
void WUserImg(unsigned char pos, unsigned char *ImgInfo);  // 写入自定义字符到CGRAM
/*********************************** 函数声明 ********************************************/

#endif