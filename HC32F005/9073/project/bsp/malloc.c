#include "malloc.h"

// 内存池(4字节对齐)
__align(4) uint8_t SRAMI_BASE[MEM1_MAX_SIZE];

// 内存管理表
uint16_t SRAMI_MAPBASE[MEM1_ALLOC_TABLE_SIZE];  // 内部SRAM内存池MAP

// 内存管理参数
const uint32_t memtblsize[SRAMBANK] = {MEM1_ALLOC_TABLE_SIZE};  // 内存表大小
const uint32_t memblksize[SRAMBANK] = {MEM1_BLOCK_SIZE};        // 内存分块大小
const uint32_t memsize[SRAMBANK] = {MEM1_MAX_SIZE};             // 内存总大小

// 内存管理控制器
malloc_dev_s mallco_dev =
{
    mem_init,       // 内存初始化
    mem_perused,    // 内存使用率
    SRAMI_BASE,     // 内存池
    SRAMI_MAPBASE,  // 内存管理状态表
    0,              // 内存管理未就绪
};

/**
  * @brief  复制内存
  * @param  *des: 目的地址
  * @param  *src: 源地址
  * @param  n: 需要复制的内存长度(字节为单位)
  * @retval NULL
  */
void mem_cpy(void *des, void *src, uint32_t n)
{
    uint8_t *xdes = des;
    uint8_t *xsrc = src;
    while (n--) {
        *xdes++ = *xsrc++;
    }
}

/**
  * @brief  设置内存
  * @param  *s: 内存首地址
  * @param   c: 要设置的值
  * @param  count: 需要设置的内存大小(字节为单位)
  * @retval NULL
  */
void mem_set(void *s, uint8_t c, uint32_t count)
{
    uint8_t *xs = s;
    while (count--) {
        *xs ++= c;
    }
}

/**
  * @brief  内存管理初始化
  * @param  memx: 所属内存块
  * @retval NULL
  */
void mem_init(uint8_t memx)
{
    mem_set(mallco_dev.memmap[memx], 0, memtblsize[memx] * sizeof(uint16_t)); // 内存状态表清零
    mem_set(mallco_dev.membase[memx], 0, memsize[memx]);                      // 内存池所有数据清零
    mallco_dev.memrdy[memx] = 1;                                              // 内存管理初始化OK
}

/**
  * @brief  获取内存使用率
  * @param  memx: 所属内存块
  * @retval 使用率(0~100)
  */
uint8_t mem_perused(uint8_t memx)
{
    uint32_t used = 0;
    uint32_t i;

    for (i = 0; i < memtblsize[memx]; i++) {
        if (mallco_dev.memmap[memx][i]) {
            used++;
        }
    }
    return (used * 100) / (memtblsize[memx]);
}

/**
  * @brief  内存分配(内部调用)
  * @param  memx: 所属内存块
  * @param  size: 要分配的内存大小(字节)
  * @retval 0XFFFFFFFF,代表错误; 其他,内存偏移地址
  */
uint32_t malloc(uint8_t memx, uint32_t size)
{
    signed long offset = 0;
    uint16_t nmemb;     // 需要的内存块数
    uint16_t cmemb = 0; // 连续空内存块数
    uint32_t i;

    if (!mallco_dev.memrdy[memx]) {
        mallco_dev.init(memx);
    }
    if (size == 0) {
        return 0XFFFFFFFF;
    }
    nmemb = size / memblksize[memx];  // 获取需要分配的连续内存块数
    if (size % memblksize[memx]) {
        nmemb++;
    }

    for (offset = memtblsize[memx] - 1; offset >= 0; offset--) {
        if (!mallco_dev.memmap[memx][offset]) {
            cmemb++;    // 连续空内存块数增加
        } else {
            cmemb = 0;  // 连续内存块清零
        }

        // 找到了连续nmemb个空内存块
        if (cmemb == nmemb) {
            for (i = 0; i < nmemb; i++) {
                // 标注内存块非空
                mallco_dev.memmap[memx][offset + i] = nmemb;
            }
            return (offset * memblksize[memx]);
        }
    }
    return 0XFFFFFFFF;  // 未找到符合分配条件的内存块
}

/**
  * @brief  释放内存(内部调用)
  * @param  memx: 所属内存块
  * @param  offset: 内存地址偏移
  * @retval 0,释放成功; 1,释放失败
  */
uint8_t free(uint8_t memx, uint32_t offset)
{
    int i;
    if (!mallco_dev.memrdy[memx]) {
        mallco_dev.init(memx);
        return 1; // 未初始化
    }

    // 偏移在内存池内
    if (offset < memsize[memx]) {
        int index = offset / memblksize[memx];      // 偏移所在内存块号码
        int nmemb = mallco_dev.memmap[memx][index]; // 内存块数量
        for (i = 0; i < nmemb; i++) {               // 内存块清零
            mallco_dev.memmap[memx][index + i] = 0;
        }
        return 0;
    } else {
        return 2; // 偏移超区了
    }
}

/**
  * @brief  分配内存
  * @param  memx: 所属内存块
  * @param  ptr: 内存首地址
  * @retval NULL
  */
void mem_free(uint8_t memx, void *ptr)
{
    uint32_t offset;
    if (ptr == NULL) {
        return; // 地址为0
    }

    offset = (uint32_t)ptr - (uint32_t)mallco_dev.membase[memx];
    free(memx, offset); // 释放内存
}

/**
  * @brief  分配内存
  * @param  memx: 所属内存块
  * @param  size: 要分配的内存大小(字节)
  * @retval 分配到的内存首地址
  */
void *mem_malloc(uint8_t memx, uint32_t size)
{
    uint32_t offset = malloc(memx, size);
    if (offset == 0XFFFFFFFF) {
        return NULL;
    } else {
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);
    }
}

/**
  * @brief  重新分配内存
  * @param  memx: 所属内存块
  * @param  *ptr: 旧内存首地址
  * @param  size: 要分配的内存大小(字节)
  * @retval 新分配到的内存首地址
  */
void *mem_realloc(uint8_t memx, void *ptr, uint32_t size)
{
    uint32_t offset = malloc(memx, size);
    if (offset == 0XFFFFFFFF) {
        return NULL;
    } else {
        mem_cpy((void *)((uint32_t)mallco_dev.membase[memx] + offset), ptr, size);  // 拷贝旧内存内容到新内存
        mem_free(memx, ptr);  // 释放旧内存
        return (void *)((uint32_t)mallco_dev.membase[memx] + offset);   // 返回新内存首地址
    }
}
