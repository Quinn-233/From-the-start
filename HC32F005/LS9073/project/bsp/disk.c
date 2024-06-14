#include "hc32f005.h"
#include "disk.h"

void flash_int(void)
 {
    if (TRUE == Flash_GetIntFlag(flash_int0)) {
        Flash_ClearIntFlag(flash_int0);
        Flash_DisableIrq(flash_int0);
    }
    if (TRUE == Flash_GetIntFlag(flash_int1)) {
        Flash_ClearIntFlag(flash_int1);
        Flash_DisableIrq(flash_int1);
    }
 }

void disk_init(void)
{
    Flash_Init(flash_int, 0);
}

void disk_erase(int offset, size_t size)
{
    u8 erase_sector_cnt = 0;
    if (((offset + size) > (DISK_END_ADDR + 1)) || (size % DISK_SECTOR_SIZE != 0)) {
        return;
    }

    erase_sector_cnt = size / DISK_SECTOR_SIZE;
    for (u8 i = 0; i < erase_sector_cnt; i++) {
        Flash_SectorErase(offset + i * DISK_SECTOR_SIZE);
    }
}

int disk_write(int offset, const void *data, int len)
{
    en_result_t enResult = Error;
    u32 addr = offset;
    u8 wdata = 0;

    if (((offset + len) > DISK_END_ADDR) || (data == NULL)) {
        return -1;
    }

    for (int i = 0; i < len; i++) {
        addr = offset + i;
        wdata = ((u8 *)data)[i];
        enResult = Flash_WriteByte(addr, wdata);
        if (Ok == enResult) {
            if (*((volatile u8 *)addr) == wdata) {
                enResult = Ok;
            } else {
                return -1;
            }
        } else {
            return -1;
        }
    }

    return 0;
}

int disk_read(int offset, void *data, int len)
{
    u32 addr = offset;
    if (((offset + len) > DISK_END_ADDR) || (data == NULL)) {
        return -1;
    }
    for (int i = 0; i < len; i++) {
        addr = offset + i;
        ((u8 *)data)[i] = *((volatile u8 *)addr);
    }
    return 0;
}
