#include "stm32f10x.h"                  // 设备头文件

/**
  * @brief 初始化定时器2，配置为定时中断模式
  * @details 配置TIM2为系统时钟源，设置预分频和自动重装载值，使能更新中断
  *          定时器中断周期计算公式：T = (PSC+1)*(ARR+1)/Tclk
  *          本配置中：T = (360)*(10000)/72MHz = 50ms
  * @retval 无
  */
void Timer_Init(void)
{
    /* 使能定时器时钟 */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    // 使能TIM2外设时钟
    
    /* 选择时钟源 */
    TIM_InternalClockConfig(TIM2);        // 选择TIM2为内部时钟源（默认也是内部时钟）
    
    /* 定时器时基单元初始化 */
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure;        // 定义时基初始化结构体
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;  // 时钟分频因子，设置为不分频
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;  // 计数模式，设置为向上计数
    TIM_TimeBaseInitStructure.TIM_Period = 10000 - 1;          // 自动重装载值ARR，决定定时器周期
    TIM_TimeBaseInitStructure.TIM_Prescaler = 360 - 1;         // 预分频值PSC，对系统时钟进行分频
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 0;       // 重复计数器值，高级定时器特有
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStructure);        // 初始化TIM2时基单元
    
    /* 清除更新标志位 */
    TIM_ClearFlag(TIM2, TIM_FLAG_Update);                       // 清除定时器更新标志
    // TIM_TimeBaseInit函数调用后会产生更新事件，可能会设置更新标志
    // 因此需要手动清除标志位，避免刚初始化就触发中断
    
    /* 使能定时器中断 */
    TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);                  // 使能TIM2的更新中断
    
    /* NVIC中断优先级配置 */
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);             // 配置NVIC优先级分组为组2
    // 分组2对应：抢占优先级0~3，子优先级0~3
    // 中断优先级配置只需在主程序开始时调用一次
    
    /* NVIC具体中断配置 */
    NVIC_InitTypeDef NVIC_InitStructure;                        // 定义NVIC初始化结构体
    NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;             // 选择TIM2中断通道
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;             // 使能该中断通道
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;   // 设置抢占优先级为2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;          // 设置子优先级为1
    NVIC_Init(&NVIC_InitStructure);                              // 初始化NVIC中断配置
    
    /* 使能定时器 */
    TIM_Cmd(TIM2, ENABLE);                     // 使能TIM2，定时器开始计数
}

/**
  * @brief TIM2中断服务函数
  * @details 定时器计数溢出时触发此函数
  *          用户需要在此函数中添加定时执行的代码
  * @retval 无
  */
/* 定时器中断服务函数，用户需要实现具体功能
void TIM2_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)
    {
        // 这里添加定时器中断需要执行的代码
        
        TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志位
    }
}
*/