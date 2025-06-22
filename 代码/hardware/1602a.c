#include "1602a.h"
#include "stm32f10x_rcc.h"

/**
 * @brief 微秒级延时函数
 * @param us 延时的微秒数
 * @note 使用嵌套循环和nop指令实现延时，精度取决于系统时钟
 */
void delay_us(unsigned int us)
{
	unsigned int  i;
	
	do
	{
		i = 10;
		while(i--) __nop();  // 空操作，延长执行时间
	} 
	while (--us);
	
}

/**
 * @brief 初始化LCD相关GPIO引脚
 * @details 配置RS、RW、EN控制引脚和数据引脚为推挽输出模式
 */
void GPIO_INIT(void)
{		
	
	GPIO_InitTypeDef PB;
	GPIO_InitTypeDef PA;
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	// 禁用JTAG，释放PA15、PB3、PB4引脚
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOA, ENABLE );
	RCC_APB2PeriphClockCmd( RCC_APB2Periph_GPIOB, ENABLE );		// 使能GPIOA和GPIOB时钟
	
	
	
	PB.GPIO_Pin = EN|RW|RS; //PB12-RS   PB13-RW   PB14-EN									
	PB.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
	PB.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &PB);
	
	
	
	PA.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|//LCD数据引脚
								GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|
								GPIO_Pin_6|GPIO_Pin_7;
	PA.GPIO_Mode = GPIO_Mode_Out_PP;  // 推挽输出模式
	PA.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &PA);
	
}

/**
 * @brief 初始化LCD模块
 * @details 执行LCD初始化序列，配置显示模式、清屏等
 */
void LCD_INIT(void)	//LCD初始化
{
	
	GPIO_INIT();	//GPIO的初始化，在LCD_INIT中调用确保初始化顺序
	
	GPIO_Write( GPIOA, 0x0000 );		// 初始化GPIOA和GPIOB输出为0
	GPIO_Write( GPIOB, 0x0000 );
	
	LCD_WRITE_CMD( 0x38 );// 设置8位数据接口，5*7点阵，2行显示
	delay_us(5000);
	LCD_WRITE_CMD( 0x38 );// 重复发送确保设置成功
	delay_us(5000);
	
	LCD_WRITE_CMD( 0x01 );// 清屏	
	delay_us(5000);
	LCD_WRITE_CMD( 0x06 );// 光标右移，显示不移动
	delay_us(5000);	
	LCD_WRITE_CMD( 0x0c );// 显示开，光标关，闪烁关
}

/**
 * @brief 向LCD写入命令
 * @param CMD 要写入的命令字节
 * @details 等待LCD空闲后，通过控制引脚发送命令
 */
void LCD_WRITE_CMD( unsigned char CMD )	// 写命令操作
{	
	
	ReadBusy();  // 等待LCD空闲
	GPIO_ResetBits( GPIOB, RS );  // RS=0，选择命令寄存器
	GPIO_ResetBits( GPIOB, RW );  // RW=0，写操作
	GPIO_ResetBits( GPIOB, EN );  // EN=0
	GPIO_Write( GPIOA, CMD );		// 输出命令到数据引脚	
	GPIO_SetBits( GPIOB, EN );    // EN=1，产生上升沿
	GPIO_ResetBits( GPIOB, EN );  // EN=0，产生下降沿，完成命令发送
}

/**
 * @brief 向LCD写入数据
 * @param ByteData 要写入的数据字节
 * @details 等待LCD空闲后，通过控制引脚发送数据
 */
void LCD_WRITE_ByteDATA( unsigned char ByteData )// 写数据操作
{	
	
	ReadBusy();  // 等待LCD空闲
	GPIO_SetBits( GPIOB, RS );   // RS=1，选择数据寄存器
	GPIO_ResetBits( GPIOB, RW );  // RW=0，写操作
	GPIO_ResetBits( GPIOB, EN );  // EN=0
	GPIO_Write( GPIOA, ByteData );  // 输出数据到数据引脚
	GPIO_SetBits( GPIOB, EN );    // EN=1，产生上升沿
	GPIO_ResetBits( GPIOB, EN );  // EN=0，产生下降沿，完成数据发送
}

/**
 * @brief 设置LCD光标位置
 * @param Column 列位置（0-15）
 * @details 将光标设置到指定列位置，采用特殊的地址映射方式
 *          前8列映射到第一行，后8列映射到第二行
 */
//LCD设置光标，列位置从左到右
void LCD_SetCursor(unsigned char Column)
{
	if(Column <= 7)// 第一行左半部分
	{
		LCD_WRITE_CMD(0x80|(Column));  // 设置第一行地址
	}
	else// 第一行右半部分（实际映射到第二行）
	{
		LCD_WRITE_CMD(0x80|(Column-8)+0x40);  // 设置第二行地址
	}
}

/**
 * @brief 在指定位置显示字符串
 * @param StrData 要显示的字符串指针
 * @param col 起始列位置
 * @details 从指定列开始显示字符串，每个字符显示后光标自动右移
 */
void LCD_WRITE_StrDATA( unsigned char *StrData, unsigned char col )// 显示字符串
{
	unsigned char i;

	for (i=0;StrData[i]!='\0';i++)
	{
		LCD_SetCursor(col);// 注意：每次写入字符前都设置光标位置
		LCD_WRITE_ByteDATA( StrData[i] );
		col++;
	}
	
}

/**
 * @brief 读取LCD忙标志
 * @details 将数据引脚配置为输入模式，读取忙标志位，等待LCD空闲
 *          完成后恢复数据引脚为输出模式
 */
void ReadBusy(void)// 读取忙标志，判断LCD是否空闲
{		
	
	GPIO_Write( GPIOA, 0x00ff );	
	
	GPIO_InitTypeDef p;
	p.GPIO_Pin = GPIO_Pin_7;		// 配置GPIOA的第7位
	p.GPIO_Mode = GPIO_Mode_IN_FLOATING;	// 浮空输入模式，用于读取LCD的忙标志
	p.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOA, &p );
	
	GPIO_ResetBits( GPIOB, RS );//RS置低，选择命令寄存器
	GPIO_SetBits( GPIOB, RW );//RW置高，读操作
	
	GPIO_SetBits( GPIOB, EN );	// 使能信号置高
	while( GPIO_ReadInputDataBit( GPIOA, GPIO_Pin_7 ) );	// 读取第7位，等待忙标志为0
	GPIO_ResetBits( GPIOB, EN );// 使能信号置低
		
	p.GPIO_Pin = GPIO_Pin_0|GPIO_Pin_1|GPIO_Pin_2|
							 GPIO_Pin_3|GPIO_Pin_4|GPIO_Pin_5|
					 		 GPIO_Pin_6|GPIO_Pin_7;		// 恢复GPIOA为输出模式
	p.GPIO_Mode = GPIO_Mode_Out_PP;
	p.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init( GPIOA, &p  );
}

/**
 * @brief 自定义字符写入CGRAM
 * @param pos 自定义字符位置（0-7）
 * @param ImgInfo 字符点阵数据指针
 * @details 将自定义字符的点阵数据写入LCD的CGRAM
 */
void WUserImg(unsigned char pos,unsigned char *ImgInfo){ // 写入自定义字符
	unsigned char cgramAddr;			// CGRAM地址
	
	if( pos <= 1 ) cgramAddr = 0x40;		// 
	if( pos > 1 && pos <= 3 ) cgramAddr = 0x50;
	if( pos > 3 && pos <= 5 ) cgramAddr = 0x60;
	if( pos > 5 && pos <= 7 ) cgramAddr = 0x70;

	LCD_WRITE_CMD( (cgramAddr + (pos%2) * 8) );	// 设置CGRAM地址
	
	while( *ImgInfo != '\0' )// 循环写入点阵数据
	{		
		LCD_WRITE_ByteDATA( *ImgInfo );
		ImgInfo++;
	}
}