#include "stm32f10x.h"
#include "Serial.h"
#include "delay.h"

/*
音乐序号  音乐名
			0	  虚空
			1	  军团
			2	  新黑色电影
			3	  困兽
			4	  神枪手
			5   追溯起源
			6   竞技场
			7		冲破藩篱
			8		咄咄逼人
			9		哥特浮华
			10	大胆尝试
			11	GLA
			12  漂泊者
			13  战火星空
			14	黄色魔法
			15	巨龙之眼
			16	咖啡拉花
			17	巴克拉姆
			18	电锯追命
			19	决心
			20	燥起来
			21	尘归尘
			22	A.D.8
			23	疤王
			24	万夫莫敌包
			25	终极
			26	大局入侵
			27	同型节奏
			28	地球末夜
			29	莫洛托夫烈火
			30	无人之境
			31	理由
			32	精彩时刻
			33	超爆话筒
			34	叛乱
			35	雄狮之口
			36	沙漠之焰
			37	好好干，好好活
			38	深红突击
			39	骷髅爆破
			40	绝对统御
			41	金属
			42	如日中天
			43	永恒之钻
			44	圣诞之歌
			45	你急了
			46	蜂鸟
			47	海绵手指
			48	8位音乐盒
			49	迈阿密热线
			50	令人发指
			51	躺平青年
			52	爆头
			53	闪光舞
			54	尖峰时刻
			55	塔罗斯的法则
			56	有害物质环境
			57	就是我
			58	EZ4ENCE
			59	爪哇哈瓦那放克乐
			60	触摸能量
			61	通宵达旦
			62	非人类
			63	花脸
			64	征服
			65	万众瞩目
			66	冲击星
			67	人生何处不青山
			68	枪炮卷饼卡车
			69	有为青年
			70	光环：士官长合集
			71	黑帝斯音乐盒
			72	反叛者
			73	Ben Bromfield,Rabbit Hole
			74	Daniel Sadowski,Dead Shot
			75	Sam Marshall,Clutch
			76	Tree Adams,Seventh Moon
			77	Tim Huling,Devil's Paintbrush
			78	Dren McDonald,Coffee!Kofe Kahveh!
			79	Matt Levine,Agency
			80	Austin Wintory,The Devil Went Clubbing in Georgia
			81	诶嘿
			
			100 c4上电下包
			101 c4下包完成
			102 c4倒计时结束爆炸_完整
*/

#include "stm32f10x.h"
#include "Serial.h"

uint8_t Send_buf[10];
void MP3CMD(uint8_t CMD , uint16_t data)
{

    Send_buf[0] = 0x7e;    //头
    Send_buf[1] = 0xff;    //保留字节 
    Send_buf[2] = 0x06;    //长度
    Send_buf[3] = CMD;     //控制指令
    Send_buf[4] = 0x00;    //是否需要反馈
	Send_buf[5] = (uint8_t)(data >> 8);    //data
    Send_buf[6] = (uint8_t)(data);   //data
	Send_buf[7] = ((~(0xff+0x06+CMD+data))+1) >> 8;    //校验和
	Send_buf[8] = ((~(0xff+0x06+CMD+data))+1)&0x00ff;//校验和
    Send_buf[9] = 0xef;    //尾
	
	for (u8 i = 0; i < 10; i ++)		//遍历数组
	{
		Serial_SendByte(Send_buf[i]);	  //依次调用Serial_SendByte发送每个字节数据
	}
}


void mp3_Init(void)
{
	Serial_Init();
}

void mp3_start(void)//上电提示音
{
	MP3CMD(0x12,100);
}

void mp3_over(void)//下包结束音
{
	MP3CMD(0x12,101);
}

void mp3_boom(void)//c4爆炸音
{
	MP3CMD(0x12,102);
}

void mp3_boom_music(void)//默认播放02文件夹内000xxx.MP3歌曲
{
	
//	MP3CMD(0x0f,0x0251);
	
		Send_buf[0] = 0x7e;    //头
    Send_buf[1] = 0xff;    //保留字节 
    Send_buf[2] = 0x06;    //长度
    Send_buf[3] = 0x0f;     //控制指令
    Send_buf[4] = 0x00;    //是否需要反馈
		Send_buf[5] = 0x02;    //data
    Send_buf[6] = 0x00;
		Send_buf[7] = 0xfe;
		Send_buf[8] = 0xea;
    Send_buf[9] = 0xef;    //尾
	
		for (u8 i = 0; i < 10; i ++)		//遍历数组
		{
			Serial_SendByte(Send_buf[i]);	  //依次调用Serial_SendByte发送每个字节数据
		}

}

/**
 * 播放01文件夹中的指定序号音乐
 * @param musicIndex 音乐序号(0-81)
 */
void mp3_playMusic(uint8_t musicIndex)
{
    // 确保音乐序号在有效范围内
    if (musicIndex > 81) {
        musicIndex = 0; // 超出范围则默认播放第一首
    }
    
    // 构造指令帧
    Send_buf[0] = 0x7e;    // 帧头
    Send_buf[1] = 0xff;    // 保留字节
    Send_buf[2] = 0x06;    // 数据长度
    Send_buf[3] = 0x0F;    // 播放指定文件夹中的音乐命令
    Send_buf[4] = 0x00;    // 不需要反馈
    
    // 设置文件夹和文件编号
    Send_buf[5] = 0x01;    // 高字节：固定为01文件夹
    Send_buf[6] = musicIndex; // 低字节：文件编号
    
    // 计算校验和
    uint16_t checksum = ~(0xff + 0x06 + 0x0F + 0x01 + musicIndex) + 1;
    Send_buf[7] = (uint8_t)(checksum >> 8);    // 校验和高字节
    Send_buf[8] = (uint8_t)(checksum & 0xFF);  // 校验和低字节
    
    Send_buf[9] = 0xef;    // 帧尾
    
    // 发送完整命令
    for (uint8_t i = 0; i < 10; i++) {
        Serial_SendByte(Send_buf[i]);
    }
}