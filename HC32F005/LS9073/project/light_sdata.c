#include "light_sdata.h"
#include "flash.h"
#include "disk.h"
#include "light_effects.h"
#include <string.h>
#include "systick.h"


uint32_t g_ligeff_user_cfg_addr = 0;//flash中最新存放的g_ligeff_user_cfg的地址
uint32_t ligeff_user_store_timer = 0;

// static ligsdata_s g_ligsdata;
void ligsdata_write(ligeff_user_cfg_s ligeff_user_cfg)
{
    disk_write(LIGHT_SDATA_ADDR_START, (const void *)(&ligeff_user_cfg), sizeof(ligeff_user_cfg_s));
}

void ligsdata_read(void *data, int len)
{
    disk_read(LIGHT_SDATA_ADDR_START, data, len);
}

void ligeff_user_retrieve(ligeff_user_cfg_s* ligeff_user_addr);
	
void ligeff_user_init(void)
{
	ligeff_user_cfg_s* ligeff_user = get_g_ligeff_user_cfg();
	//上电时，要从flash中取出上一次下电前最近的效果序号
	extern ligeff_idx_e Now_ligeff_idx;
	extern ltstr_solid_color_t g_ligeff_solid_color[];
	ligeff_user_cfg_s* ligeff_user_addr = get_g_ligeff_user_cfg();
	ligeff_user_retrieve(ligeff_user_addr);
	if(ligeff_user_addr->save_flag != 0xFA){
		Flash_SectorErase(LIGEFF_USER_SECTOR);
		ligeff_user_cfg_init();
		ligeff_user_store(ligeff_user_addr);
	}
	Now_ligeff_idx = ligeff_user_addr->idx;
	
	for (u8 i = 0; i < LIGEFF_USER_CFG_COLORS; i++) {
        ligeff_user->colors[i].value = 0;
    }
    //ligeff_user_set_color((pixel_u *)(&(g_ligeff_solid_color[ligeff_user->g_ligeff_solid_color_idx])), 1);
	ligeff_user_set_color((pixel_u *)g_ligeff_solid_color[ligeff_user->g_ligeff_solid_color_idx].color,g_ligeff_solid_color[ligeff_user->g_ligeff_solid_color_idx].color_len);
	STRLIGHTS_ON();
}

void ligeff_user_store(ligeff_user_cfg_s* ligeff_user_addr)
{
	ligeff_user_cfg_s ligeff_user_save;
	ligeff_user_cfg_s ligeff_user_read;
	memset(&ligeff_user_read, 0, sizeof(ligeff_user_cfg_s));
	if( (g_ligeff_user_cfg_addr >= (LIGEFF_USER_SECTOR + DISK_SECTOR_SIZE - sizeof(ligeff_user_cfg_s))) || (g_ligeff_user_cfg_addr < LIGEFF_USER_SECTOR) ){
		g_ligeff_user_cfg_addr = LIGEFF_USER_SECTOR;
		Flash_SectorErase(LIGEFF_USER_SECTOR);
	}
	memset(&ligeff_user_save, 0, sizeof(ligeff_user_cfg_s));
	memcpy(&ligeff_user_save,ligeff_user_addr,sizeof(ligeff_user_cfg_s));
	ligeff_user_save.save_flag = 0xFA;
	if(disk_write(g_ligeff_user_cfg_addr,(uint8_t *)&ligeff_user_save,sizeof(ligeff_user_cfg_s)) == (-1)){
		g_ligeff_user_cfg_addr = LIGEFF_USER_SECTOR;
		Flash_SectorErase(LIGEFF_USER_SECTOR);	
		ligeff_user_store(get_g_ligeff_user_cfg());
	}else{
		disk_read(g_ligeff_user_cfg_addr,(uint8_t *)&ligeff_user_read,sizeof(ligeff_user_cfg_s));
		g_ligeff_user_cfg_addr += sizeof(ligeff_user_cfg_s);
	}
	
	return;
}

void ligeff_user_retrieve(ligeff_user_cfg_s* ligeff_user_addr)
{
	ligeff_user_cfg_s ligeff_user_r;
	for(int i = 0; i < (DISK_SECTOR_SIZE - sizeof(ligeff_user_cfg_s)); i += sizeof(ligeff_user_cfg_s)){
		g_ligeff_user_cfg_addr = LIGEFF_USER_SECTOR + i;
		memset(&ligeff_user_r, 0, sizeof(ligeff_user_cfg_s));
		memcpy((uint8_t*)&ligeff_user_r,(uint8_t*)(g_ligeff_user_cfg_addr),sizeof(ligeff_user_cfg_s));
		if(ligeff_user_r.save_flag == 0xFA)
		{
			memcpy(ligeff_user_addr,&ligeff_user_r,sizeof(ligeff_user_cfg_s));
		}else{
			break;
		}
	}
}

void flash_manage_loop(void){
	if(ligeff_user_store_timer && sys_time_exceed_ms(ligeff_user_store_timer,3 * 1000)){
		ligeff_user_cfg_s* ligeff_user_addr = get_g_ligeff_user_cfg();
		ligeff_user_store_timer = 0;
		ligeff_user_store(ligeff_user_addr);
	}
}

void ready_store_ligeff_user(void){
	ligeff_user_store_timer = SYS_TIME_GET_MS();
}



