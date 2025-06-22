/* FlashData.h - STM32F103C8内部FLASH数据存储模块 */
#ifndef __FLASH_DATA_H__
#define __FLASH_DATA_H__

#include "stm32f10x.h"
#include <stdbool.h>

/* FLASH配置 */
#define FLASH_USER_START_ADDR   0x0800F000    // 数据存储起始地址(最后4KB)
#define FLASH_PAGE_SIZE         1024          // F103C8每页1KB
#define FLASH_LAST_PAGE         63            // 最后一页(0-63)

/* 数据块定义(根据需要扩展) */
typedef struct {
    uint32_t magic;           // 魔术字，用于验证数据有效性
    uint16_t version;         // 数据版本
    uint16_t reserved;        // 保留字段
    
    /* 用户数据字段 */
    int score;                // 分数示例
    uint8_t volume;           // 音量设置
    bool music_on;            // 音乐开关
    uint32_t play_count;      // 播放次数
    // 可以添加更多变量...
} FlashData;

/* API函数声明 */
bool FlashData_Init(void);
bool FlashData_Read(FlashData* data);
bool FlashData_Write(const FlashData* data);
bool FlashData_Erase(void);

#endif