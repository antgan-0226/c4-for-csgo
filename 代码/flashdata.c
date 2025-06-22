/* FlashData.c - STM32F103C8内部FLASH数据存储模块实现 */
#include "stm32f10x_flash.h"
#include "flashdata.h"

#define FLASH_MAGIC           0x5A5A5A5A    // 魔术字，用于验证数据有效性
#define FLASH_DATA_VERSION    0x0001        // 数据版本

/* 检查数据是否有效 */
static bool isDataValid(const FlashData* data) {
    return (data->magic == FLASH_MAGIC && data->version == FLASH_DATA_VERSION);
}

/* 解锁FLASH */
static void unlockFlash(void) {
    FLASH_Unlock();
    FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPRTERR);
}

/* 锁定FLASH */
static void lockFlash(void) {
    FLASH_Lock();
}

/* 擦除FLASH页 */
static bool eraseFlashPage(uint32_t address) {
    FLASH_Status status = FLASH_ErasePage(address);
    return (status == FLASH_COMPLETE);
}

/* 写入数据到FLASH */
static bool writeFlashData(uint32_t address, const uint8_t* data, uint32_t length) {
    FLASH_Status status = FLASH_COMPLETE;
    
    // 按字(32位)写入
    for (uint32_t i = 0; i < length; i += 4) {
        uint32_t word = 0;
        
        // 构建32位字
        for (int j = 0; j < 4 && i + j < length; j++) {
            word |= ((uint32_t)data[i + j]) << (j * 8);
        }
        
        // 写入字
        status = FLASH_ProgramWord(address + i, word);
        if (status != FLASH_COMPLETE) {
            return false;
        }
    }
    
    return true;
}

/* 初始化FLASH数据模块 */
bool FlashData_Init(void) {
    unlockFlash();
    return true;
}

/* 从FLASH读取数据 */
bool FlashData_Read(FlashData* data) {
    if (data == NULL) {
        return false;
    }
    
    // 从FLASH读取数据
    const FlashData* flashData = (const FlashData*)FLASH_USER_START_ADDR;
    
    // 检查数据有效性
    if (isDataValid(flashData)) {
        // 有效数据，复制到输出结构
        *data = *flashData;
        return true;
    }
    
    // 数据无效，返回默认值
    data->magic = FLASH_MAGIC;
    data->version = FLASH_DATA_VERSION;
    data->score = 0;
    data->volume = 50;
    data->music_on = true;
    data->play_count = 0;
    
    return false;
}

/* 写入数据到FLASH */
bool FlashData_Write(const FlashData* data) {
    if (data == NULL) {
        return false;
    }
    
    // 擦除页
    if (!eraseFlashPage(FLASH_USER_START_ADDR)) {
        return false;
    }
    
    // 写入数据
    return writeFlashData(FLASH_USER_START_ADDR, (const uint8_t*)data, sizeof(FlashData));
}

/* 擦除FLASH数据 */
bool FlashData_Erase(void) {
    return eraseFlashPage(FLASH_USER_START_ADDR);
}