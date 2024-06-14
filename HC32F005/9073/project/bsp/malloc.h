#ifndef __MALLOC_H
#define __MALLOC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include <stdint.h>

#ifndef NULL
#define NULL    0
#endif

#define SRAM_IN     0   // 内部内存池
#define SRAMBANK    1   // 定义支持的内存管理块数

// mem1内存参数设定, mem1完全处于内部SRAM里面
#define MEM1_BLOCK_SIZE         32                                  // 内存块大小为32字节
#define MEM1_MAX_SIZE           (2 * 1024)                          // 最大管理内存 2K
#define MEM1_ALLOC_TABLE_SIZE   (MEM1_MAX_SIZE / MEM1_BLOCK_SIZE)   // 内存表大小

// 内存管理控制器
typedef struct tag_malloc_dev
{
    void (*init)(uint8_t);          // 初始化
    uint8_t (*perused)(uint8_t);    // 内存使用率
    uint8_t *membase[SRAMBANK];     // 内存池, 管理SRAMBANK个区域的内存
    uint16_t *memmap[SRAMBANK];     // 内存状态表
    uint8_t memrdy[SRAMBANK];       // 内存管理是否就绪
} malloc_dev_s;

/* Private functions ---------------------------------------------------------*/
void mem_set(void *s, uint8_t c, uint32_t count);   // 设置内存
void mem_cpy(void *des, void *src, uint32_t n);     // 复制内存
void mem_init(uint8_t memx);                        // 内存管理初始化函数(外/内部调用)
uint32_t malloc(uint8_t memx, uint32_t size);       // 内存分配(内部调用)
uint8_t free(uint8_t memx, uint32_t offset);        // 内存释放(内部调用)
uint8_t mem_perused(uint8_t memx);                  // 获得内存使用率(外/内部调用)

/* Exported functions ------------------------------------------------------- */
void mem_free(uint8_t memx, void *ptr);
void *mem_malloc(uint8_t memx, uint32_t size);
void *mem_realloc(uint8_t memx, void *ptr, uint32_t size);

#ifdef __cplusplus
}
#endif

#endif // __MALLOC_H
