#include "stm32f10x.h"                  // 设备头文件，包含STM32F10x系列芯片外设定义
#include <stdio.h>
#include <stdarg.h>

/**
  * @brief 串口1初始化函数
  * @details 配置USART1为9600波特率的发送模式，初始化TX引脚(GPIOA9)为复用推挽输出
  * @note 硬件连接：PA9(TX)接串口转换芯片，使用默认系统时钟(72MHz)计算波特率
  * @retval 无
  */
void Serial_Init(void)
{
    /* 使能外设时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);  // 使能USART1时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // 使能GPIOA时钟
    
    /* 配置TX引脚(PA9)为复用推挽输出 */
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;         // 复用推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;               // 选择PA9引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;       // 引脚速度50MHz
    GPIO_Init(GPIOA, &GPIO_InitStructure);                  // 初始化GPIOA
    
    /* 配置USART1参数 */
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = 9600;              // 波特率设为9600bps
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;  // 无硬件流控制
    USART_InitStructure.USART_Mode = USART_Mode_Tx;         // 仅发送模式
    USART_InitStructure.USART_Parity = USART_Parity_No;     // 无校验位
    USART_InitStructure.USART_StopBits = USART_StopBits_1;  // 1位停止位
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  // 8位数据位
    USART_Init(USART1, &USART_InitStructure);              // 初始化USART1
    
    /* 使能USART1 */
    USART_Cmd(USART1, ENABLE);                             // 启动USART1
}


/**
  * @brief 串口发送单字节数据
  * @param Byte 待发送的字节数据
  * @note 发送完成后等待TXE标志位为1（发送数据寄存器为空）
  * @retval 无
  */
void Serial_SendByte(uint8_t Byte)
{
    USART_SendData(USART1, Byte);                         // 向USART数据寄存器写入字节
    while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);  // 等待发送缓冲区为空
}


/**
  * @brief 串口发送字节数组
  * @param Array 待发送的字节数组首地址
  * @param Length 数组长度
  * @note 按顺序逐个字节发送，适用于二进制数据传输
  * @retval 无
  */
void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i++) {                        // 遍历数组所有元素
        Serial_SendByte(Array[i]);                        // 调用单字节发送函数
    }
}


/**
  * @brief 串口发送字符串
  * @param String 待发送的字符串指针（以'\0'结尾）
  * @note 自动识别字符串结束符'\0'，适用于ASCII文本传输
  * @retval 无
  */
void Serial_SendString(char *String)
{
    uint8_t i;
    for (i = 0; String[i] != '\0'; i++) {                 // 遍历字符串直到'\0'
        Serial_SendByte(String[i]);                        // 发送每个字符
    }
}


/**
  * @brief 计算幂函数（辅助函数）
  * @param X 底数
  * @param Y 指数
  * @return X的Y次幂结果
  * @note 例如：Serial_Pow(10, 3)返回1000
  */
uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
    uint32_t Result = 1;                                  // 初始化结果为1
    while (Y--) {                                         // 循环Y次
        Result *= X;                                      // 累乘计算幂
    }
    return Result;
}


/**
  * @brief 串口发送指定长度的数字
  * @param Number 待发送的无符号长整型数字(0~4294967295)
  * @param Length 发送的数字位数（不足前补0，超过则取高位）
  * @note 例如：Serial_SendNumber(123, 5)发送"00123"
  * @retval 无
  */
void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
    uint8_t i;
    for (i = 0; i < Length; i++) {                        // 按指定位数循环
        // 计算当前位的数值并转换为ASCII字符
        Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
    }
}


/**
  * @brief 重定向fputc函数到串口（支持printf）
  * @param ch 待发送的字符
  * @param f 文件指针（标准库参数，此处未使用）
  * @note 调用printf时会自动触发此函数
  * @retval 发送的字符
  */
int fputc(int ch, FILE *f)
{
    Serial_SendByte(ch);                                  // 调用串口发送单字节函数
    return ch;
}


/**
  * @brief 串口格式化输出函数（类似printf）
  * @param format 格式化字符串（支持%d, %s, %f等格式符）
  * @param ... 可变参数列表
  * @note 内部使用va_list处理可变参数，支持标准C的格式化输出
  * @retval 无
  */
void Serial_Printf(char *format, ...)
{
    char String[100];                                     // 临时存储格式化后的字符串
    va_list arg;                                          // 定义可变参数列表
    va_start(arg, format);                                // 初始化参数列表
    vsprintf(String, format, arg);                        // 按格式填充字符串
    va_end(arg);                                          // 结束参数列表
    Serial_SendString(String);                            // 发送格式化后的字符串
}