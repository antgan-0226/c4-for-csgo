#include "stm32f10x.h"                  // Device header
#include "Delay.h"

/**
 * @brief 初始化矩阵键盘相关GPIO引脚
 * @details 配置3个输出引脚（行）和4个输入引脚（列）
 */
void MatrixKey_Init(void)
{
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);    // 使能GPIOB时钟
    
    GPIO_InitTypeDef GPIO_InitStructure;
    
    // 配置行引脚（PB7、PB5、PB15）为推挽输出模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7 | GPIO_Pin_5 | GPIO_Pin_15;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
    
    // 配置列引脚（PB6、PB4、PB9、PB8）为上拉输入模式
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;     // 上拉输入
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6 | GPIO_Pin_4 | GPIO_Pin_9 | GPIO_Pin_8;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

/**
 * @brief 确定当前按下的按键所在列
 * @return 返回列号(1-4)，无按键按下时返回0
 * @details 检测并返回当前按下的按键所在的列，包含消抖处理
 */
uint8_t MatrixKey_determine(void)
{
    uint8_t Key = 0;
    
    // 检测第1列（PB4）
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0)
    {
        Delay_ms(10);  // 消抖延时
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == 0) {Key = 1;}
    }
    
    // 检测第2列（PB9）
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == 0)
    {
        Delay_ms(10);  // 消抖延时
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_9) == 0) {Key = 2;}
    }    
    
    // 检测第3列（PB8）
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == 0)
    {
        Delay_ms(10);  // 消抖延时
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_8) == 0) {Key = 3;}
    }    
    
    // 检测第4列（PB6）
    if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0)
    {
        Delay_ms(10);  // 消抖延时
        if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_6) == 0) {Key = 4;}
    }
    
    return Key;
}

/*

                                 ①      ②      ③      ④      ⑤      ⑥      ⑦
			|	②		1      2      3    pin15   pin4   pin5   pin6    pin7   pin8   pin9
			|	⑦		4      5      6
			|	⑥		7      8      9
			V	④		*      0      #
            ③      ①     ⑤
           --------------->

*/


/**
 * @brief 获取矩阵键盘按下的键值
 * @return 返回按键对应的字符值（'1'-'9','0','*','#'），无按键按下时返回空格' '
 * @details 通过逐行扫描确定具体按下的按键，并转换为对应的字符值
 */
uint8_t MatrixKey_GetValue(void)
{
    uint8_t Key = 0;
    
    // 扫描第1行（PB5输出低电平）
    GPIO_ResetBits(GPIOB, GPIO_Pin_5);  // 选中第1行
    GPIO_SetBits(GPIOB, GPIO_Pin_7);   
    GPIO_SetBits(GPIOB, GPIO_Pin_15);  

    if(MatrixKey_determine())  // 检测是否有按键按下
    {
        Key = MatrixKey_determine();  // 获取列号(1-4)
    }
    
    // 扫描第2行（PB15输出低电平）
    GPIO_ResetBits(GPIOB, GPIO_Pin_15);  // 选中第2行
    GPIO_SetBits(GPIOB, GPIO_Pin_5);   
    GPIO_SetBits(GPIOB, GPIO_Pin_7);   
    
    if(MatrixKey_determine())
    {
        Key = MatrixKey_determine() + 4;  // 第2行按键值为列号+4
    }
    
    // 扫描第3行（PB7输出低电平）
    GPIO_ResetBits(GPIOB, GPIO_Pin_7);  // 选中第3行
    GPIO_SetBits(GPIOB, GPIO_Pin_5);   
    GPIO_SetBits(GPIOB, GPIO_Pin_15);  
    
    if(MatrixKey_determine())
    {
        Key = MatrixKey_determine() + 8;  // 第3行按键值为列号+8
    }
    
    // 将数字键值转换为对应的字符
    switch(Key)
    {
        case 1: Key = '1'; break;
        case 2: Key = '4'; break;
        case 3: Key = '7'; break;
        case 4: Key = '*'; break;
        case 5: Key = '2'; break;
        case 6: Key = '5'; break;
        case 7: Key = '8'; break;
        case 8: Key = '0'; break;
        case 9: Key = '3'; break;
        case 10: Key = '6'; break;
        case 11: Key = '9'; break;
        case 12: Key = '#'; break;
        default: Key = ' ';  // 无按键按下
    }
    
    return Key;
}