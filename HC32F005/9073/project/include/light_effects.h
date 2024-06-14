#ifndef LIGHT_EFFECTS_H
#define LIGHT_EFFECTS_H

#include "light_vm.h"

// #define LIGEFF_APP_NORMAL
#define LIGEFF_APP_ALEXA

#define LIGEFF_USER_CFG_COLORS          16      // 用户自定义颜色个数

typedef enum tag_ligeff_def_color
{
    LIGEFF_DEF_COLOR_RED = 0,   // 红
    LIGEFF_DEF_COLOR_GREEN,     // 绿
    LIGEFF_DEF_COLOR_BLUE,      // 蓝
    LIGEFF_DEF_COLOR_WHITE,     // 白
    LIGEFF_DEF_COLOR_YELLOW,    // 黄
    LIGEFF_DEF_COLOR_CYAN,      // 青
    LIGEFF_DEF_COLOR_PURPLE,    // 紫
    LIGEFF_DEF_COLOR_ORANGE,    // 橘
    LIGEFF_DEF_COLOR_EMERALD,   // 翠绿
    LIGEFF_DEF_COLOR_BLACK,     // 黑

    LIGEFF_DEF_COLOR_BUTT
} ligeff_def_color_e;

// 静态效果颜色定义，当前应用与
typedef enum tag_ligeff_solid_color
{
#ifdef LIGEFF_APP_NORMAL
    LIGEFF_SOLID_COLOR_RED = 0,
    LIGEFF_SOLID_COLOR_GREEN,
    LIGEFF_SOLID_COLOR_BLUE,
    LIGEFF_SOLID_COLOR_WHITE,
    LIGEFF_SOLID_COLOR_YELLOW,
    LIGEFF_SOLID_COLOR_CYAN,
    LIGEFF_SOLID_COLOR_PURPLE,
    LIGEFF_SOLID_COLOR_ORANGE,
    LIGEFF_SOLID_COLOR_EMERALD,
    LIGEFF_SOLID_COLOR_BLACK,
#endif
#ifdef LIGEFF_APP_ALEXA
    LIGEFF_SOLID_COLOR_WARM_WHITE = 0,
    LIGEFF_SOLID_COLOR_RED,
    LIGEFF_SOLID_COLOR_CRIMSON,
    LIGEFF_SOLID_COLOR_SALMON,
    LIGEFF_SOLID_COLOR_ORANGE,
    LIGEFF_SOLID_COLOR_GOLD,
    LIGEFF_SOLID_COLOR_YELLOW,
    LIGEFF_SOLID_COLOR_GREEN,
    LIGEFF_SOLID_COLOR_TURQUOISE,
    LIGEFF_SOLID_COLOR_CYAN,
    LIGEFF_SOLID_COLOR_SKY_BLUE,
    LIGEFF_SOLID_COLOR_BLUE,
    LIGEFF_SOLID_COLOR_PURPLE,
    LIGEFF_SOLID_COLOR_MAGENTA,
    LIGEFF_SOLID_COLOR_PINK,
    LIGEFF_SOLID_COLOR_LAVENDER,
	
	COLORMIX_MODE_CANDY_CANE,
	COLORMIX_MODE_CHRISTMASMIX,
	COLORMIX_MODE_FROZENMIX,
	COLORMIX_MODE_HOLIDAYMIX,
	COLORMIX_MODE_CHRISTMASMULTI,
	COLORMIX_MODE_ELEQANCE,
	COLORMIX_MODE_HARVEST,
	COLORMIX_MODE_PATRIOTIC,
	
#endif
    LIGEFF_SOLID_COLOR_BUTT
} ligeff_solid_color_e;

typedef enum tag_ligeff_fallranbow_color
{
    LIGEFF_FALLRANBOW_COLOR_RED = 0,   // 红
    LIGEFF_FALLRANBOW_COLOR_GREEN,     // 绿
    LIGEFF_FALLRANBOW_COLOR_BLUE,      // 蓝
    LIGEFF_FALLRANBOW_COLOR_BUTT
} ligeff_fallranbow_color_e;

typedef enum tag_ligeff_speed
{
    LIGEFF_SPEED_SLOW = 1,
    LIGEFF_SPEED_NORMAL,
    LIGEFF_SPEED_FAST,
    LIGEFF_SPEED_BUTT
} ligeff_speed_e;

typedef enum tag_ligeff_idx
{
    LIGEFF_SOLID = 0,       // 单色常亮
#ifdef LIGEFF_APP_NORMAL
    // LIGEFF_BLINK,           // 闪烁 (节奏)
    // LIGEFF_FALLRAINBOW,     // 彩虹瀑布
    // LIGEFF_FALLSNAKE,       // 瀑布蛇
    // LIGEFF_COLLIDE,         // 碰撞
    // LIGEFF_ANAKONDA,        // 贪吃蛇
    // LIGEFF_GLOW,            // 星火
    // LIGEFF_THEYOYO,         // 溜溜球
    // LIGEFF_WAVE_INV,        // 反向波浪(波浪上)

    // LIGEFF_THEME_RAINBOW,   // 主题彩虹(串联)
    // LIGEFF_RANDOM_C,        // 随机颜色(小行星)
#endif
#ifdef LIGEFF_APP_ALEXA
    LIGEFF_TWINKLE,         // 轮闪
    LIGEFF_GLISTEN,         // 闪亮 (交替渐变)
    LIGEFF_SHIMMER,         // 渐变 (BREATH)
    LIGEFF_STEPBREATH,
    LIGEFF_FIRECRACKER,
    LIGEFF_FLASH,           // 闪烁 (节奏, 同BLINK)
    LIGEFF_SPARKLE,
#endif
    LIGEFF_IDX_BUTT
} ligeff_idx_e;

typedef void (* ligeff_handle) (void);

typedef struct tag_ligeff
{
    ligeff_idx_e idx;
    ligeff_handle handle;   // 效果函数指针
    u16 frame_id;   // 当前帧

    u16 frame_cnt;  // 帧数
    u8 frame_rate;  // 每更新多少颗, 刷新一次
    u8 frame_delay; // 每延迟多少帧, 更新一次
    u8 frame_skip;  // 跳帧
    u8 luminance;   // 亮度
} ligeff_s;

typedef struct tag_ligeff_user_cfg
{
	u8 save_flag;
	ligeff_solid_color_e g_ligeff_solid_color_idx;//最近一次的常量效果颜色序号
	ligeff_idx_e last_user_idx;//最近一次的动态效果序号
    ligeff_idx_e idx;//当前效果序号(包括常量)
    ligeff_speed_e speed;
    u8 luminance;
    u8 color_len;
    pixel_u colors[LIGEFF_USER_CFG_COLORS];
} ligeff_user_cfg_s;

typedef struct ltstr_color_t{
    uint8_t color_len;
    pixel_u color[6];
} ltstr_solid_color_t;


ligeff_user_cfg_s* get_g_ligeff_user_cfg(void);
void ligeff_user_cfg_init(void);
void ligeff_clear(void);
void ligeff_show_frames(void);
void ligeff_fast_show_frames(void);
void ligeff_light_all(pixel_u *rgbpix);

void ligeff_init(void);
void ligeff_user_set_speed(ligeff_speed_e speed);
void ligeff_user_set_luminance(u8 luminance);
void ligeff_user_set_color(pixel_u *color, u8 len);

ligeff_idx_e ligeff_switch_effect(ligeff_idx_e ligeff_idx);
ligeff_idx_e ligeff_set_effect(ligeff_idx_e effect);
ligeff_idx_e ligeff_switch_normal(void);

ligeff_idx_e ligeff_switch_butt(void);
ligeff_s ligeff_get_current_effect(void);
void ligeff_solid_change_color(ligeff_idx_e ligeff_idx);

void ligeff_handler(ligeff_idx_e ligeff_idx);

#endif // LIGHT_EFFECTS_H
