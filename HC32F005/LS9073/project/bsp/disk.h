#ifndef __DISK_H
#define __DISK_H

#include "base_types.h"

#define DISK_END_ADDR       (0x00007FFFu)
#define DISK_SECTOR_SIZE    512
#define DISK_SECTOR_59      0x7600
#define DISK_SECTOR_60      0x7800
#define DISK_SECTOR_61      0x7A00
#define DISK_SECTOR_62      0x7C00
#define DISK_SECTOR_63      0x7E00

void disk_init(void);
void disk_erase(int offset, size_t size);
int disk_write(int offset, const void *data, int len);
int disk_read(int offset, void *data, int len);

#endif // __DISK_H
