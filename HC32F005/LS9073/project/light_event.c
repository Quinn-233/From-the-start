#include "hc32f005.h"
#include "systick.h"
#include "key.h"
#include "com.h"
#include "string_lights.h"
#include "light_effects.h"
#include "light_event.h"
#include "light_sdata.h"

ligeff_idx_e Now_ligeff_idx = LIGEFF_SOLID;

void ligevt_process(void)
{
    
    ligevt_com_cmd_s *p_com_cmd = NULL;
    key_status_e key1_status = key_status_get(KEY1);
    key_status_e key2_status = key_status_get(KEY2);

	//每次短按下任意一个按键，都需要保存下当前的状态
	
	//短按下常量键，最新的常量颜色序号保存在g_ligeff_user_cfg.g_ligeff_solid_color_idx
    if (key1_status == KEY_STATUS_SON) {//常量键
        if ((Now_ligeff_idx != LIGEFF_SOLID) && (Now_ligeff_idx != LIGEFF_IDX_BUTT)) {
			//从 效果键 转到 常量键：
			//1、关灯,清空显存
			//2、Now_ligeff_idx = LIGEFF_IDX_BUTT表明从效果键切换到了常量键
            STRLIGHTS_OFF();
			ligeff_clear();
			Now_ligeff_idx = LIGEFF_IDX_BUTT;
            //Now_ligeff_idx = ligeff_switch_butt();
        } else {		
            STRLIGHTS_ON();//开灯
            ligeff_solid_change_color(Now_ligeff_idx);//预备最近一次常量颜色
            Now_ligeff_idx = ligeff_switch_normal();
        }
		ready_store_ligeff_user();
    }
	
	//短按下效果键，记录当前效果序号
    if (key2_status == KEY_STATUS_SON) {//效果键
        if (Now_ligeff_idx == LIGEFF_SOLID) {
			//从 常量键 转到 效果键 :
			//1、关灯,清空显存
			//2、将最近一次所保存的效果序号g_ligeff_user_cfg.last_user_idx赋值给g_ligeff_user_cfg.idx
			//3、Now_ligeff_idx = LIGEFF_IDX_BUTT表明从常量键切换到了效果键
            STRLIGHTS_OFF();
			ligeff_clear();
			get_g_ligeff_user_cfg()->idx = get_g_ligeff_user_cfg()->last_user_idx;
			Now_ligeff_idx = LIGEFF_IDX_BUTT;
            //Now_ligeff_idx = ligeff_switch_butt();
        } else {
            STRLIGHTS_ON();//开灯
            Now_ligeff_idx = ligeff_switch_effect(Now_ligeff_idx);//确定动态效果序号
        }
		ready_store_ligeff_user();
    }

    if (key2_status == KEY_STATUS_LON) {
        if (Now_ligeff_idx == LIGEFF_SOLID) {
            u8 factory_reset = 1;
            u32 blink_time = 0;
            u8 blink_cnt = 0;
            pixel_u color = { .value = 0 };
            STRLIGHTS_ON();

            u8 cmd = LIGEVT_COM_CMD_FAC_REST;
            com_cmd_send(&cmd, 1);

            while (factory_reset) {
                if (sys_time_exceed_us(blink_time, 500 * 1000)) {
                    blink_time = SYS_TICK_GET_VAL();
                    if ((blink_cnt % 2) == 0) {
                        color.rgb.r = 0;
                        color.rgb.g = 0;
                        color.rgb.b = 0xff;
                    } else {
                        color.value = 0;
                    }

                    ligeff_light_all(&color);
                    blink_cnt++;
                    if (blink_cnt == 6) {
						Flash_SectorErase(LIGEFF_USER_SECTOR);
						ligeff_user_cfg_init();
						ligeff_init();
						//ligeff_user_store(get_g_ligeff_user_cfg());
						//STRLIGHTS_OFF();
						factory_reset = 0;
                    }
                }
            }
        }
    }

    p_com_cmd = (ligevt_com_cmd_s *)com_cmd_get();
    if (p_com_cmd != NULL) {
		static u8 firstflag = 1;
		if(firstflag){
			firstflag = 0;
			return;
		}
        // // 长度为一，特殊命令字
        // if ((p_com_cmd->speed == 0) && (p_com_cmd->effect == LIGEVT_COM_CMD_FAC_REST)) {
        //     u8 factory_reset = 1;
        //     u32 blink_time = 0;
        //     u8 blink_cnt = 0;
        //     pixel_u color = { .value = 0 };
        //     STRLIGHTS_ON();

        //     while (factory_reset) {
        //         if (sys_time_exceed_us(blink_time, 500 * 1000)) {
        //             blink_time = SYS_TICK_GET_VAL();
        //             if ((blink_cnt % 2) == 0) {
        //                 color.rgb.r = 0;
        //                 color.rgb.g = 0;
        //                 color.rgb.b = 0xff;
        //             } else {
        //                 color.value = 0;
        //             }

        //             ligeff_light_all(&color);
        //             blink_cnt++;
        //             if (blink_cnt == 6) {
        //                 factory_reset = 0;
        //             }
        //         }
        //     }
        //     return;
        // }

        if (p_com_cmd->luminance == 0) {
            STRLIGHTS_OFF();
            Now_ligeff_idx = ligeff_switch_butt();
        } else {
            ligeff_idx_e ligeff = (ligeff_idx_e)(p_com_cmd->effect - 1); // App中从1开始，灯串为0开始

            ligeff_user_set_speed((ligeff_speed_e)p_com_cmd->speed);
            ligeff_user_set_luminance(p_com_cmd->luminance);
            ligeff_user_set_color(p_com_cmd->color, p_com_cmd->color_len);
            STRLIGHTS_ON();

            if (ligeff == LIGEFF_SOLID) {
                Now_ligeff_idx = ligeff_switch_normal();
            } else {
                if (ligeff < LIGEFF_IDX_BUTT) {
                    Now_ligeff_idx = ligeff_set_effect(ligeff);
                }
            }
        }
    }

    ligeff_handler(Now_ligeff_idx);
}
