#include "stm32f10x.h"
#include "stm32f10x_flash.h"
#include "matrixkey.h"
#include "1602a.h"
#include "delay.h"
#include "LED.H"
#include "math.h"
#include "TIMER.h"
#include "Serial.h"
#include "mp3.h"
#include "string.h"
#include <stdbool.h>
#include <stdio.h> 


//函数声明
void showDefaultScreen(void);//展示默认屏幕
char arraysEqual(unsigned char arr1[], unsigned char arr2[], int size); //比较数组
void rightShiftArray(unsigned char arr[], int size);
void leftShiftArray(unsigned char arr[], int size); 
void updateKeyPressState(void);//更新按键标识
void handleKeyPressFeedback(void);//按下键盘响应
void clearPasswordArray(unsigned char pass[], int size);//清除密码
void updatePasswordDisplay(unsigned char pass[]);//更新密码展示
void showStarAnimation(void);//展示***动画
void unlockPasswordScan(unsigned char pass[]);//解锁密码扫描
void deployPasswordScan(unsigned char pass[]);//部署密码扫描 

// 状态处理函数声明
void handleInitState(void);//初始化状态
void handlePasswordInputState(void);//处理输入状态
void handlePasswordVerifyState(void);//验证密码状态
void handleUnlockSuccessState(void);//解锁成功，CT胜利
void handleUnlockFailureState(void);//解锁失败，T胜利，播放音乐盒
void handleAlarmClockModeState(void); //闹钟模式
void checkHashPress(void); //检查是否按下#按键

// 定义系统状态
typedef enum {
    STATE_PASSWORD_INPUT, // 密码输入状态
    STATE_PASSWORD_VERIFY,// 密码验证状态
    STATE_UNLOCK_SUCCESS, // 解锁成功状态
    STATE_UNLOCK_FAILURE, // 解锁失败状态
		STATE_ALARM_CLOCK_MODE    // 设置闹钟状态
} SystemState;

// 变量定义
SystemState currentState = STATE_PASSWORD_INPUT;  // 当前系统状态
uint8_t volumeLevel = 20; // 音量级别

unsigned char defaultPassword[8]={'7','3','5','5','6','0','8'};	//默认密码
unsigned char password[8]={'*','*','*','*','*','*','*'};	//用户输入的密码
unsigned char unlockPassword[8]={'*','*','*','*','*','*','*'};	//解锁输入的密码

char KeyNum, sign; //按键变量
int unlockArrayIndex=0; //解密时密码下标
int spaceCount=5; //居中显示前面空格数
uint8_t isPressed=0; //是否键盘被按下标记
uint8_t hashPressed = 0;               // #键按下标志
uint16_t hashTimer = 0;             // 记录按#计数器
uint16_t countdown = 100;              // 倒计时值
uint16_t Num = 0,Num_sign = 0;			//定义在定时器中断里自增的变量

int main()
{
	//初始化
	mp3_Init();//mp3初始化		
	Timer_Init();//定时器初始化
	LED_Init();//led 蜂鸣器 继电器io口初始化
	LCD_INIT();		//LCD1601初始化
	MatrixKey_Init();//矩阵键盘初始化
	
	Delay_ms(1000);
	
	//开机默认显示文案
	showDefaultScreen();
	
	
	Delay_ms(500);
	mp3_playMusic(81);
	
	//	mp3_start();//由于市面上mini mp3模块质量参差不齐，导致上电后无法立刻检测到指令，需等段时间才能检测到
					//如果你想启用上电音效，可以尝试取消此注释，尝试是否有上电音效

	//主循环
	while(1)
	{
		KeyNum = MatrixKey_GetValue();
		// 检测*和#是否同时按下
    checkHashPress();
		
		// 根据当前状态调用相应的处理函数
    switch(currentState) {
				case STATE_PASSWORD_INPUT:
						handlePasswordInputState();
						break;
				case STATE_PASSWORD_VERIFY:
						handlePasswordVerifyState();
            break;
				case STATE_UNLOCK_SUCCESS:
            handleUnlockSuccessState();
            break;
				case STATE_UNLOCK_FAILURE:
						handleUnlockFailureState();
            break;
        case STATE_ALARM_CLOCK_MODE:
            handleAlarmClockModeState();
						break;
				default:
						currentState = STATE_PASSWORD_INPUT;
						break;
		}
		Delay_ms(10);
  }
}

// 检测#是否按下
void checkHashPress(void) {
    if (KeyNum == '#') {
        hashPressed = 1;
				hashTimer++;
    } else {
        hashPressed = 0;
    }
    
    // #超过5次判定为进入设置模式
    if (hashPressed) {
        if (hashTimer >= 5) {
            currentState = STATE_ALARM_CLOCK_MODE;
            hashPressed = 0;
        }
    } else {
        hashPressed = 0;
    }
}

// 密码输入状态处理
void handlePasswordInputState(void) {
    deployPasswordScan(password);
    
    // 检测密码是否输入完成
    if(password[0] != '*') {
        // 延时防抖
        for(long i = 0; i < 50000; i++) {
            deployPasswordScan(password);
        }
				
				//输入完毕
				//检查密码是否输入完成并与默认密码对比
				if (password[0] != '*' && password[6] != '*') {  // 假设7位密码已输入完成
						Delay_ms(200);  // 防抖延时
						if (password[0] != '*' && password[6] != '*') {  // 二次确认
								if (arraysEqual(password, defaultPassword, 7)) {
										// 密码正确，部署成功
										currentState = STATE_PASSWORD_VERIFY;
								} else {
										// 密码错误, 提示
										LCD_WRITE_StrDATA("Password Error!", 0);
										for(int i = 0; i < 5; i++) {
												LED1_Turn();
												Delay_ms(50);
										}
										LED1_OFF();
										Delay_ms(2000);
										// 清空密码数组，恢复默认文案
										clearPasswordArray(password, 7);
										currentState = STATE_PASSWORD_INPUT;
									  showDefaultScreen();
								}
						}
				}
    }
}

// 默认文案
void showDefaultScreen(void) {
		LCD_WRITE_StrDATA("                ", 0);
		LCD_WRITE_StrDATA("*******", spaceCount);									
}

// 密码验证状态处理
void handlePasswordVerifyState(void) {
    LED2_ON();
    mp3_over();
	
		for(int j=0; j<countdown; j++)//倒计时提示
		{
			LED1_ON();
		
			// 蜂鸣器和LED亮起时间
			for(int i = 0; i < 2; i++) {
				KeyNum = MatrixKey_GetValue();
				if(KeyNum!=' '){sign=1;}
			
				if(sign == 0) {
					Delay_ms(20);                    
				} else {
					unlockPasswordScan(unlockPassword);                            
				}
		}
		
		LED1_OFF();
		for(int i=0; i<((int)((pow(((double)0.978),((double)j)))*50)-2); i++)//j增大，此for循环随时间增大而执行次数变少，所占用时间变短，即LED1_ON()被执行间隔的越来越短			
		{
				KeyNum = MatrixKey_GetValue();
				if(KeyNum!=' '){sign=1;}
			
				if(sign==0)
				{
						showStarAnimation();							
				}
				else
				{
						unlockPasswordScan(unlockPassword);							
				}
		}
		
		// 检查解锁密码是否输入完成
		if(unlockPassword[6] != '*') {
				if(arraysEqual(password, unlockPassword, 7)) {
					currentState = STATE_UNLOCK_SUCCESS;
					return;
				} else {
					for (int i = 0; i < 7; i++) {
						unlockPassword[i] = '*';
					}
					unlockArrayIndex = 0;
					sign=0;
					LCD_WRITE_StrDATA(unlockPassword, spaceCount);
				}
			}
		}
		
		//全部倒计时结束，拆弹失败
    currentState = STATE_UNLOCK_FAILURE;
}


// 解锁成功状态处理
void handleUnlockSuccessState(void) {
    LCD_WRITE_StrDATA("                ", 0);
    Delay_ms(50);
    
    // 成功提示
    for(int i = 0; i < 5; i++) {
        LED1_Turn();
        Delay_ms(50);
    }
    LCD_WRITE_StrDATA(unlockPassword, spaceCount);
    
    for(int i = 0; i < 2; i++) {
        Delay_ms(300);
        LCD_WRITE_StrDATA("                ", 0);
        Delay_ms(300);
        LCD_WRITE_StrDATA(unlockPassword, spaceCount);
    }
		LCD_WRITE_StrDATA("                ", 0);
		LCD_WRITE_StrDATA("CT win!", 5);
		
    while(1) {
        LED2_ON(); // 保持继电器开启
    }
}

// 解锁失败状态处理
void handleUnlockFailureState(void) {
    LED1_ON();
    LCD_WRITE_StrDATA("                ", 0);
    mp3_boom_music();
    Delay_ms(1300);
    LED1_OFF();
    
		LCD_WRITE_StrDATA("T win!", 5);
    while(1) {
        LED2_OFF(); // 保持继电器关闭
    }
}

void mp3_stop(void)
{
    // 发送停止播放命令 (0x16)
    MP3CMD(0x16, 0);
}

// 设置模式状态处理
void handleAlarmClockModeState(void) {
		LCD_WRITE_StrDATA("                ", 0);
    LCD_WRITE_StrDATA("Alarm Clock Mode!", 0);
}

void TIM2_IRQHandler(void) // TIM2定时器中断服务函数
{
    // 检查是否发生了更新中断（计数器溢出或下溢）
    if (TIM_GetITStatus(TIM2, TIM_IT_Update) == SET)    
    {
        // 根据Num_sign的值决定计数方向
        if(Num_sign == 0)
        {
            Num ++;  // 向上计数
            TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志位
            if(Num >= 13) {
                Num_sign = 1;  // 达到上限，切换为向下计数
            }
        }
        else
        {
            Num --;  // 向下计数
            TIM_ClearITPendingBit(TIM2, TIM_IT_Update);  // 清除中断标志位
            if(Num <= 0) {
                Num_sign = 0;  // 达到下限，切换为向上计数
            }
        }
    }
}

char arraysEqual(unsigned char arr1[], unsigned char arr2[], int size) 
{
    for (int i = 0; i < size; i++)
	{
        if (arr1[i] != arr2[i])
				{
            return 0; // 如果发现有不相等的元素，返回false
        }
    }
    return 1; // 所有元素都相等，返回true
}

void rightShiftArray(unsigned char arr[], int size)
{
    if (size > 0)
		{
        for (int i = size - 1; i > 0; i--)
			  {
            arr[i] = arr[i - 1]; // 将每个元素向右移动
        }
        arr[0] = '*'; // 将第一个元素设置为*
    }
}

void leftShiftArray(unsigned char arr[], int size)
{
    if (size > 0) 
		{
        for (int i = 0; i < size - 1; i++) 
			  {
            arr[i] = arr[i + 1]; // 将每个元素向左移动
        }
        arr[7] = 0; // 将最后一个元素设置为0
    }
}


/**
  * @brief 处理按键按下的LED反馈
  * @note 按键按下时LED闪烁两次
  */
void handleKeyPressFeedback() {
    LED1_Turn();
    Delay_ms(50);
    LED1_Turn();
}

/**
  * @brief 清空密码数组
  * @param pass 密码数组指针
  * @param size 数组大小
  */
void clearPasswordArray(unsigned char pass[], int size) {
    for (int i = 0; i < size; i++) {
        pass[i] = '*';
    }
}

/**
  * @brief 更新按键按下状态
  * @note 按键按下时isPressed置1，松开时置0
  */
void updateKeyPressState() {
    if (KeyNum != ' ') {
        isPressed = 1; // 按键按下
    } else {
        isPressed = 0; // 按键松开
    }
}

/**
  * @brief 更新密码显示
  * @param pass 密码数组指针
  * @note 先清空LCD显示区域，再显示更新后的密码
  */
void updatePasswordDisplay(unsigned char pass[]) {
    LCD_WRITE_StrDATA("                ", 0);
    LCD_WRITE_StrDATA(pass, spaceCount);
}


//*左右晃动动画
void showStarAnimation(void)
{
	LCD_WRITE_StrDATA("                ",0);
	LCD_WRITE_StrDATA("***", Num);		
	Delay_ms(20);
}

//扫描键盘，更新解锁密码数组并更新显示
void unlockPasswordScan(unsigned char pass[])
{
	
	// 定义局部变量存储是否需要更新显示
		KeyNum = MatrixKey_GetValue();
		if (isPressed == 0) {
			// 处理数字键输入（0-9）
			if ((KeyNum!=' ') &&(KeyNum!='*') && (KeyNum!='#')&& (isPressed == 0)) {
					// 按键防抖处理
					Delay_ms(50);
					// LED反馈
					handleKeyPressFeedback();
					pass[unlockArrayIndex] = KeyNum;//第一位为按下的键
					unlockArrayIndex++;
					if(unlockArrayIndex>=7)unlockArrayIndex=0;
					updatePasswordDisplay(pass);
			}
			// 处理退格键(*)输入
			else if (KeyNum == '*' && (isPressed == 0)) {
					// 按键防抖处理
					Delay_ms(50);
					// LED反馈
					handleKeyPressFeedback();
					unlockArrayIndex--;
					pass[unlockArrayIndex]='*';
					updatePasswordDisplay(pass);
			}
			// 处理清除键(#)输入
			else if (KeyNum == '#' && (isPressed == 0)) {
					// 按键防抖处理
					Delay_ms(50);
					// LED反馈
					handleKeyPressFeedback();
					// 清空密码数组
					clearPasswordArray(pass, 7);
					// 标记需要更新显示
					unlockArrayIndex=0;
					updatePasswordDisplay(pass);
			}
		}
    
	  // 更新按键状态
    if(KeyNum!=' ')
		{
			isPressed=1;//按下
			Delay_ms(20);
		}
		else
		{
			isPressed=0;
			Delay_ms(20);
		}
}

void deployPasswordScan(unsigned char pass[]) {
    // 获取当前按键值
    KeyNum = MatrixKey_GetValue();
    
    // 定义局部变量存储是否需要更新显示
    bool needDisplayUpdate = false;
    
    // 处理数字键输入（0-9）
    if ((KeyNum >= '0' && KeyNum <= '9') && (isPressed == 0)) {
        // 按键防抖处理
        Delay_ms(50);
        // LED反馈
        handleKeyPressFeedback();
        // 左移数组并添加新输入的数字
        leftShiftArray(pass, 8);
        pass[6] = KeyNum;
        // 标记需要更新显示
        needDisplayUpdate = true;
    }
    // 处理退格键(*)输入
    else if (KeyNum == '*' && (isPressed == 0)) {
        // 按键防抖处理
        Delay_ms(50);
        // LED反馈
        handleKeyPressFeedback();
        // 右移数组实现退格
        rightShiftArray(pass, 7);
        // 标记需要更新显示
        needDisplayUpdate = true;
    }
    // 处理清除键(#)输入
    else if (KeyNum == '#' && (isPressed == 0)) {
        // 按键防抖处理
        Delay_ms(50);
        // LED反馈
        handleKeyPressFeedback();
        // 清空密码数组
        clearPasswordArray(pass, 7);
        // 标记需要更新显示
        needDisplayUpdate = true;
    }
    
    // 更新按键状态
    updateKeyPressState();
    
    // 根据标记更新LCD显示
    if (needDisplayUpdate) {
        updatePasswordDisplay(pass);
    }

}