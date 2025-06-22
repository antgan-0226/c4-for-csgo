#include "stm32f10x.h"                  // 设备头文件

/**
  * @brief 初始化LED和蜂鸣器控制引脚
  * @details 配置GPIOA的Pin8和Pin10为推挽输出模式
  *          Pin8 - 控制LED1和蜂鸣器（低电平触发）
  *          Pin10 - 控制LED2和继电器（低电平触发）
  * @retval 无
  */
void LED_Init(void)
{
    // 使能GPIOA时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // 定义GPIO初始化结构体
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;    // 推挽输出模式
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_10;  // 配置Pin8和Pin10
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;   // IO口速度为50MHz
    
    // 初始化GPIOA
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // 初始化为低电平（LED熄灭，蜂鸣器静音，继电器断开）
    GPIO_ResetBits(GPIOA, GPIO_Pin_8 | GPIO_Pin_10);
}

/**
  * @brief 打开LED1和蜂鸣器
  * @details 如果当前引脚为低电平，则切换为高电平
  *          注意：硬件设计为低电平触发，调用此函数会使LED亮起，蜂鸣器发声
  * @retval 无
  */
void LED1_ON(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
    }
}

/**
  * @brief 关闭LED1和蜂鸣器
  * @details 如果当前引脚为高电平，则切换为低电平
  *          注意：硬件设计为低电平触发，调用此函数会使LED熄灭，蜂鸣器静音
  * @retval 无
  */
void LED1_OFF(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8) != 0)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    }
}

/**
  * @brief 翻转LED1和蜂鸣器状态
  * @details 如果当前为高电平则切换为低电平，反之亦然
  *          用于实现LED闪烁和蜂鸣器鸣叫效果
  * @retval 无
  */
void LED1_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_8) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_8);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_8);
    }
}

/**
  * @brief 打开LED2和继电器
  * @details 如果当前引脚为低电平，则切换为高电平
  *          注意：硬件设计为低电平触发，调用此函数会使LED亮起，继电器吸合
  * @retval 无
  */
void LED2_ON(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_10) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_10);
    }
}

/**
  * @brief 关闭LED2和继电器
  * @details 如果当前引脚为高电平，则切换为低电平
  *          注意：硬件设计为低电平触发，调用此函数会使LED熄灭，继电器断开
  * @retval 无
  */
void LED2_OFF(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_10) != 0)
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_10);
    }
}

/**
  * @brief 翻转LED2和继电器状态
  * @details 如果当前为高电平则切换为低电平，反之亦然
  *          用于实现LED闪烁效果
  * @retval 无
  */
void LED2_Turn(void)
{
    if (GPIO_ReadOutputDataBit(GPIOA, GPIO_Pin_10) == 0)
    {
        GPIO_SetBits(GPIOA, GPIO_Pin_10);
    }
    else
    {
        GPIO_ResetBits(GPIOA, GPIO_Pin_10);
    }
}