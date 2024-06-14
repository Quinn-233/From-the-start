#ifndef __LIGHT_SDATA_H
#define __LIGHT_SDATA_H

#include "disk.h"
#include "light_effects.h"

#define LIGEFF_USER_SECTOR	DISK_SECTOR_59

typedef struct tag_ligsdata
{
    int saddr;
    ligeff_user_cfg_s ligeff_user_cfg;
} ligsdata_s;

#define LIGHT_SDATA_ADDR_START          DISK_SECTOR_62
#define LIGHT_SDATA_USED_SECTOR_CNT     2

void ligeff_user_store(ligeff_user_cfg_s* ligeff_user_addr);
void ligeff_user_init(void);
void flash_manage_loop(void);
void ready_store_ligeff_user(void);

#endif // __LIGHT_SDATA_H
