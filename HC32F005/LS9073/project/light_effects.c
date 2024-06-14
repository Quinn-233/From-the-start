#include "hc32f005.h"
#include "systick.h"
#include "malloc.h"
#include "light_effects.h"
#include "light_sdata.h"

static u8 g_ligeff_start = 0;
ligeff_user_cfg_s g_ligeff_user_cfg;
//ligeff_solid_color_e g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_YELLOW;

#define LIGEFF_CFG_SAVE_TIME        3000

#define LIGEFF_TWINKLE_APART        2       // 轮闪间隔
#define LIGEFF_CRASH_POINT          50

#define foreach(i, n)           for (unsigned int (i) = 0; (i) < (n); ++(i))
#define foreach_range(i, s, e)  for (unsigned int (i) = (s); (i) < (e); ++(i))
#define foreach_arr(i, arr)     for (unsigned int (i) = 0; (i) < ARRAY_SIZE(arr); ++(i))

static const uint8_t g_gamma_table[] = {
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
    1, 1, 1, 1, 2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 3, 3,
    3, 3, 3, 3, 3, 4, 4, 4, 4, 4, 5, 5, 5, 5, 5, 6,
    6, 6, 6, 7, 7, 7, 7, 8, 8, 8, 8, 9, 9, 9, 10, 10,
    10, 11, 11, 12, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16, 17,
    17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25, 25,
    26, 27, 27, 28, 29, 29, 30, 31, 31, 32, 33, 34, 34, 35, 36, 37,
    38, 38, 39, 40, 41, 42, 43, 43, 44, 45, 46, 47, 48, 49, 50, 51,
    52, 53, 54, 55, 56, 57, 58, 59, 60, 62, 63, 64, 65, 66, 67, 68,
    70, 71, 72, 73, 75, 76, 77, 78, 80, 81, 82, 84, 85, 87, 88, 89,
    91, 92, 94, 95, 97, 98, 100, 101, 103, 104, 106, 108, 109, 111, 112, 114,
    116, 117, 119, 121, 123, 124, 126, 128, 130, 131, 133, 135, 137, 139, 141, 143,
    145, 147, 149, 151, 153, 155, 157, 159, 161, 163, 165, 167, 169, 171, 173, 176,
    178, 180, 182, 185, 187, 189, 192, 194, 196, 199, 201, 203, 206, 208, 211, 213,
    216, 218, 221, 223, 226, 228, 231, 234, 236, 239, 242, 244, 247, 250, 253, 255
};

// 默认颜色定义
static const pixel_u g_ligeff_default_color[LIGEFF_DEF_COLOR_BUTT] = {
    { .value = 0x0000ff00 }, // r: 255 g: 0 b: 0
    { .value = 0x00be0000 },
    { .value = 0xbe000000 },
    { .value = 0x4baaff00 },
    { .value = 0x00beff00 },
    { .value = 0x5abe0000 },
    { .value = 0x5a00ff00 },
    { .value = 0x0064ff00 },
    { .value = 0x15be0000 },

    { .value = 0x00000000 },
};

const ltstr_solid_color_t g_ligeff_solid_color[LIGEFF_SOLID_COLOR_BUTT] = {
#ifdef LIGEFF_APP_NORMAL
    { .value = 0x0000ff00 }, // r: 255 g: 0 b: 0
    { .value = 0x00be0000 },
    { .value = 0xbe000000 },
    { .value = 0x4baaff00 },
    { .value = 0x00beff00 },
    { .value = 0x5abe0000 },
    { .value = 0x5a00ff00 },
    { .value = 0x0064ff00 },
    { .value = 0x15be0000 },

    { .value = 0x00000000 },
#endif
#ifdef LIGEFF_APP_ALEXA
    { .color_len = 1, .color[0] = {.value = 0x000000ff} }, // w: 255 r: 0 g: 0 b: 0
    { .color_len = 1, .color[0] = {.value = 0x0000ff00} }, // r: 255 g: 0 b: 0
    { .color_len = 1, .color[0] = {.value = 0x3B13DA00} },
    { .color_len = 1, .color[0] = {.value = 0x7A9FFF00} },
    { .color_len = 1, .color[0] = {.value = 0x00A5FF00} },
    { .color_len = 1, .color[0] = {.value = 0x00D4FF00} },
    { .color_len = 1, .color[0] = {.value = 0x00FFFF00} },
    { .color_len = 1, .color[0] = {.value = 0x00FF0000} },
    { .color_len = 1, .color[0] = {.value = 0xCFDF3E00} },
    { .color_len = 1, .color[0] = {.value = 0xFFFF0000} },
    { .color_len = 1, .color[0] = {.value = 0xE9CD8600} },
    { .color_len = 1, .color[0] = {.value = 0xFF000000} },
    { .color_len = 1, .color[0] = {.value = 0xEF21A000} },
    { .color_len = 1, .color[0] = {.value = 0xFF00FF00} },
    { .color_len = 1, .color[0] = {.value = 0xCBBEFF00} },
    { .color_len = 1, .color[0] = {.value = 0xFF7F9F00} },
	
    { .color_len = 2, .color[0] = {.value = 0x0000ff00}, .color[1] = {.value = 0xffffff00} },
    { .color_len = 2, .color[0] = {.value = 0x0000ff00}, .color[1] = {.value = 0x00be0000} },
    { .color_len = 3, .color[0] = {.value = 0xff000000}, .color[1] = {.value = 0x80800000}, .color[2] = {.value = 0xEF21A000} },
    { .color_len = 3, .color[0] = {.value = 0x0000ff00}, .color[1] = {.value = 0x000000ff}, .color[2] = {.value = 0x00be0000} },
    { .color_len = 5, .color[0] = {.value = 0x0000ff00}, .color[1] = {.value = 0x00be0000}, .color[2] = {.value = 0xff000000}, .color[3] = {.value = 0x00ffff00}, .color[4] = {.value = 0x00a5ff00} },
    { .color_len = 2, .color[0] = {.value = 0x000000ff}, .color[1] = {.value = 0xffffff00} },
    { .color_len = 3, .color[0] = {.value = 0x0000ff00}, .color[1] = {.value = 0xEF21A000}, .color[2] = {.value = 0x00a5ff00} },
    { .color_len = 3, .color[0] = {.value = 0x0000ff00}, .color[1] = {.value = 0xffffff00}, .color[2] = {.value = 0xff000000} },	
#endif
};

/*
const pixel_u g_ligeff_solid_color[LIGEFF_SOLID_COLOR_BUTT] = {
#ifdef LIGEFF_APP_NORMAL
    { .value = 0x0000ff00 }, // r: 255 g: 0 b: 0
    { .value = 0x00be0000 },
    { .value = 0xbe000000 },
    { .value = 0x4baaff00 },
    { .value = 0x00beff00 },
    { .value = 0x5abe0000 },
    { .value = 0x5a00ff00 },
    { .value = 0x0064ff00 },
    { .value = 0x15be0000 },

    { .value = 0x00000000 },
#endif
#ifdef LIGEFF_APP_ALEXA
    { .value = 0x000000ff }, // w: 255 r: 0 g: 0 b: 0
    { .value = 0x0000ff00 }, // r: 255 g: 0 b: 0
    { .value = 0x3B13DA00 },
    { .value = 0x7A9FFF00 },
    { .value = 0x00A5FF00 },
    { .value = 0x00D4FF00 },
    { .value = 0x00FFFF00 },
    { .value = 0x00FF0000 },
    { .value = 0xCFDF3E00 },
    { .value = 0xFFFF0000 },
    { .value = 0xE9CD8600 },
    { .value = 0xFF000000 },
    { .value = 0xEF21A000 },
    { .value = 0xFF00FF00 },
    { .value = 0xCBBEFF00 },
    { .value = 0xFF7F9F00 },
#endif
};
*/


/*
static const pixel_u g_ligeff_fallranbow_color[LIGEFF_FALLRANBOW_COLOR_BUTT] = {
    { .value = 0x0000ad00 }, // r: 173 g: 0 b: 0
    { .value = 0x00670000 }, // r: 0 g: 103 b: 0
    { .value = 0x29000000 }, // r: 0 g: 0 b: 41
};
*/
typedef enum {
    VM_display_ready = 0,
    VM_display_busy = 1,
    VM_display_over = 2,
    VM_display_idle = 0xAA
} video_display_status;

typedef enum {
    VM_update_ready = 0,
    VM_update_start = 1,
    VM_update_busy = 2,
    VM_update_over = 3,
    VM_update_coherent = 4,
    VM_update_idle = 0xAA
} video_update_status;

typedef struct {
    video_display_status display_status;
    video_update_status update_status;
} video_memory_handler_t;

video_memory_handler_t video_memory_handler;    //显存状态
u32 max_changed_number = 0;

typedef struct {
    int i;
    int i2;
    int addr;
    int temp;
    u16 count;
    u16 ltstr_length;
    u16 length;
    u8 intensity;
    u8 flag;
    u8 step;
    u8 offset;
    u8 v[STRLIGHTS_LENGTH];
    u8 u[STRLIGHTS_LENGTH];
} scene_variable_t;
scene_variable_t a;

static void ligeff_show_solid(void);
static void ligeff_show_twinkle(void);
static void ligeff_show_glisten(void);
static void ligeff_show_shimmer(void);
static void ligeff_show_step_breath(void);
static void ligeff_show_firecracker(void);
static void ligeff_show_flash(void);
static void ligeff_show_sparkle(void);

// static void ligeff_show_blink(void);
// static void ligeff_show_fallranbow(void);
// static void ligeff_show_fallsnake(void);
// static void ligeff_show_collide(void);
// static void ligeff_show_anakonda(void);
// static void ligeff_show_glow(void);
// static void ligeff_show_theYOYO(void);
// static void ligeff_show_wave_inv(void);
// static void ligeff_show_random_color(void);

// static u8 g_ligeff_frame_delay = 0;
static ligeff_s g_ligeffs[LIGEFF_IDX_BUTT] = {
    {
        .idx = LIGEFF_SOLID,
        .handle = ligeff_show_solid,
    },
    // {
    //     .idx = LIGEFF_BLINK,
    //     .handle = ligeff_show_blink,
    // },
    {
        .idx = LIGEFF_TWINKLE,
        .handle = ligeff_show_twinkle,
    },
    {
        .idx = LIGEFF_GLISTEN,
        .handle = ligeff_show_glisten,
    },
    {
        .idx = LIGEFF_SHIMMER,
        .handle = ligeff_show_shimmer,
    },
    {
        .idx = LIGEFF_STEPBREATH,
        .handle = ligeff_show_step_breath,
    },
    {
        .idx = LIGEFF_FIRECRACKER,
        .handle = ligeff_show_firecracker,
    },
    {
        .idx = LIGEFF_FLASH,
        .handle = ligeff_show_flash,
    },
    {
        .idx = LIGEFF_SPARKLE,
        .handle = ligeff_show_sparkle,
    }
    // {
    //     .idx = LIGEFF_FALLRAINBOW,
    //     .handle = ligeff_show_fallranbow,
    // },
    // {
    //     .idx = LIGEFF_FALLSNAKE,
    //     .handle = ligeff_show_fallsnake,
    // },
    // {
    //     .idx = LIGEFF_COLLIDE,
    //     .handle = ligeff_show_collide,
    // },
    // {
    //     .idx = LIGEFF_ANAKONDA,
    //     .handle = ligeff_show_anakonda,
    // },
    // {
    //     .idx = LIGEFF_GLOW,
    //     .handle = ligeff_show_glow,
    // },
    // {
    //     .idx = LIGEFF_THEYOYO,
    //     .handle = ligeff_show_theYOYO,
    // },
    // {
    //     .idx = LIGEFF_WAVE_INV,
    //     .handle = ligeff_show_wave_inv,
    // },
    // // {
    // //     .idx = LIGEFF_RANDOM_C,
    // //     .handle = ligeff_show_random_color,
    // // }
};



static unsigned int rnd_m_w = 12431;
static unsigned int rnd_m_z = 52462;
static __inline unsigned int rand(void)
{
    rnd_m_w = 18000 * (rnd_m_w & 0xffff) + (rnd_m_w >> 16);
    rnd_m_z = 36969 * (rnd_m_z & 0xffff) + (rnd_m_z >> 16);
    unsigned int result = (rnd_m_z << 16) + rnd_m_w;

    return (unsigned int)( result ^ SYS_TICK_GET_VAL() );
}
/*
static const u16 g_random_table[] = {
    4779,  33334, 57825, 58249, 53019, 26948, 12007, 40018, 20520, 27606,
    11001, 16256, 30736, 27732, 23359, 29842, 14955, 38899, 46533, 27497,
    14355, 54955, 38930, 63617, 17725, 39049, 42519, 45782, 42788, 20670,
    23592, 29287, 49468, 29785, 4684,  38061, 39248, 10542, 50822, 12776,
    45217, 29508, 57098, 57169, 3276,  59136, 32341, 30658, 48170, 52286,
    62729, 33624, 2005,  14067, 54798, 9489,  64582, 42147, 37386, 24542,
    47556, 1432,  28090, 30381, 35047, 52618, 11027, 9444,  46125, 43658,
    49818, 64110, 14581, 20945, 27156, 23044, 31718, 2328,  50426, 20751,
    17493, 44383, 41340, 47185, 63353, 1197,  19087, 60274, 31598, 18646,
    49793, 10165, 18530, 19691, 63335, 46243, 17781, 44541, 10023, 60778,
    15490, 39874, 53210, 22274, 21739, 20764, 14086, 14530, 39349, 34491,
    16815, 15340, 43888, 34925, 59378, 9263,  33055, 35145, 62229, 3313,
    39359, 16747, 18034, 8497,  42194, 65402, 28942, 39039
};
static u16 my_rand(int seed)
{
    static u8 index = 0;
    u16 val;
    if (seed != -1) {
        index = seed;
    }
    val = g_random_table[index];
    index = (index + 1) & 0x7F;
    return val;
}
*/
// // 碰撞效果阶段定义
// typedef enum tag_ligeff_crash_stage
// {
//     LIGEFF_CRASH_STAGE_COLLECT = 0, // 相聚
//     LIGEFF_CRASH_STAGE_SPLIT,       // 分开
//     LIGEFF_CRASH_STAGE_DISAPPEAR,   // 消失
//     LIGEFF_CRASH_STAGE_BUTT
// } ligeff_crash_stage_e;
// static ligeff_crash_stage_e g_crash_stage = LIGEFF_CRASH_STAGE_COLLECT;

// static __inline u8 ligeff_frame_delay(ligeff_s *ligeff)
// {
//     if (g_ligeff_frame_delay == 0) {
//         ligeff->frame_id = 0;   // 该效果的第一帧, 则复位当前帧
//     }

//     if ((g_ligeff_frame_delay != 0) && (g_ligeff_frame_delay <= ligeff->frame_rate)) {
//         g_ligeff_frame_delay++;
//         return 1;
//     }

//     g_ligeff_frame_delay = 1;
//     return 0;
// }

ligeff_user_cfg_s* get_g_ligeff_user_cfg(void){
	return &g_ligeff_user_cfg;
}

void ligeff_clear(void)
{
    ligvm_buf_reset();
    for (u16 i = 0; i < STRLIGHTS_LENGTH; i++) {
        ligvm_buf_display(i);
    }
}

void ligeff_show_frames(void)
{
    for (u16 i = 0; i < STRLIGHTS_LENGTH; i++) {
        ligvm_buf_display(i);
    }
}

void ligeff_fast_show_frames(void)
{
    for (u16 i = 0; i < STRLIGHTS_LENGTH; i++) {
        if (LIGVM_IF_CHANGED(i)) {
            LIGVM_NCHANGE(i);
            ligvm_buf_display(i);
        }
    }
}

void ligeff_light_all(pixel_u *rgbpix)
{
    for (u16 i = 0; i < STRLIGHTS_LENGTH; i++) {
        strlights_set_color(STRLIGHTS_START_ADDR + i, (u8 *)rgbpix);
    }
}

void ligeff_user_set_speed(ligeff_speed_e speed)
{
    if (speed >= LIGEFF_SPEED_BUTT) {
        return;
    }

    g_ligeff_user_cfg.speed = speed;
}

void ligeff_user_set_luminance(u8 luminance)
{
    g_ligeff_user_cfg.luminance = luminance;
}

void ligeff_user_set_color(pixel_u *color, u8 len)
{
    if (len > LIGEFF_USER_CFG_COLORS) {
        return;
    }
    for (u8 i = 0; i < len; i++) {
#ifdef STRLIGHTS_COLOR_RGBW
        g_ligeff_user_cfg.colors[i].rgb.w = g_gamma_table[color[i].rgb.w];
#endif
        g_ligeff_user_cfg.colors[i].rgb.r = g_gamma_table[color[i].rgb.r];
        g_ligeff_user_cfg.colors[i].rgb.g = g_gamma_table[color[i].rgb.g];
        g_ligeff_user_cfg.colors[i].rgb.b = g_gamma_table[color[i].rgb.b];
    }

    g_ligeff_user_cfg.color_len = len;
}


void ligeff_user_cfg_init(void)
{
    g_ligeff_user_cfg.idx = LIGEFF_SOLID;
    g_ligeff_user_cfg.speed = LIGEFF_SPEED_NORMAL;

    for (u8 i = 0; i < LIGEFF_USER_CFG_COLORS; i++) {
        g_ligeff_user_cfg.colors[i].value = 0;
    }
    //ligeff_user_set_color((pixel_u *)(&(g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx])), 1);
	ligeff_user_set_color((pixel_u *)g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx].color,g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx].color_len);
	
    g_ligeff_user_cfg.luminance = STRLIGHTS_LUMINANCE_MAX_VAL;
	g_ligeff_user_cfg.last_user_idx = LIGEFF_TWINKLE;
	g_ligeff_user_cfg.g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_WARM_WHITE;
	
	g_ligeff_user_cfg.save_flag = 0;
    // ligeff_user_cfg_s ligeff_user_cfg_backup = {0};
    // ligsdata_read((void *)&ligeff_user_cfg_backup, sizeof(ligeff_user_cfg_s));
    // if (ligeff_user_cfg_backup.idx != 0xff) {
    //     mem_cpy((void *)&g_ligeff_user_cfg, (void *)&ligeff_user_cfg_backup, sizeof(ligeff_user_cfg_s));
    // }
}

void ligeff_init(void)
{
    ligvm_buf_init();
    ligeff_clear();
	ligeff_user_init();
    ligeff_user_cfg_init();
}

ligeff_idx_e ligeff_switch_effect(ligeff_idx_e ligeff_idx)
{
	//若切换，则当前动态效果序号为最近一次保存的序号，否则当前动态效果加1
	
	if(ligeff_idx == LIGEFF_IDX_BUTT){//切换
		g_ligeff_user_cfg.idx = g_ligeff_user_cfg.last_user_idx;
	}else{
		g_ligeff_user_cfg.idx++;
	}
	
	ligvm_buf_reset();
    if (g_ligeff_user_cfg.idx >= LIGEFF_IDX_BUTT) {
        g_ligeff_user_cfg.idx = LIGEFF_TWINKLE;
    }	
	
	g_ligeff_user_cfg.last_user_idx = g_ligeff_user_cfg.idx;
	max_changed_number = STRLIGHTS_LENGTH;
	
	return g_ligeff_user_cfg.idx;
	
	/*
	static ligeff_idx_e last_idx = LIGEFF_TWINKLE;
    ligvm_buf_reset();
	if(last_idx == g_ligeff_user_cfg.idx){
		g_ligeff_user_cfg.idx++;
	}else{
		g_ligeff_user_cfg.idx = last_idx;
	}
    if (g_ligeff_user_cfg.idx >= LIGEFF_IDX_BUTT) {
        g_ligeff_user_cfg.idx = LIGEFF_TWINKLE;
    }
    max_changed_number = STRLIGHTS_LENGTH;
	last_idx = g_ligeff_user_cfg.idx;
    return g_ligeff_user_cfg.idx;
	*/
	/*
    ligvm_buf_reset();
    g_ligeff_user_cfg.idx++;
    if (g_ligeff_user_cfg.idx >= LIGEFF_IDX_BUTT) {
        g_ligeff_user_cfg.idx = LIGEFF_TWINKLE;
        // g_ligeff_user_cfg.idx = LIGEFF_BLINK;
    }
    max_changed_number = STRLIGHTS_LENGTH;
    return g_ligeff_user_cfg.idx;
	*/
	
}

ligeff_idx_e ligeff_set_effect(ligeff_idx_e effect)
{
    ligvm_buf_reset();

    if (effect >= LIGEFF_IDX_BUTT) {
        return LIGEFF_IDX_BUTT;
    }
    g_ligeff_user_cfg.idx = effect;

    max_changed_number = STRLIGHTS_LENGTH;

    return g_ligeff_user_cfg.idx;
}

ligeff_idx_e ligeff_switch_normal(void)
{
    ligeff_clear();

    g_ligeff_user_cfg.idx = LIGEFF_SOLID;

    max_changed_number = STRLIGHTS_LENGTH;

    g_ligeffs[LIGEFF_SOLID].frame_id = 0;

    return g_ligeff_user_cfg.idx;
}

ligeff_idx_e ligeff_switch_butt(void)
{
    g_ligeff_user_cfg.g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_BUTT;
    g_ligeff_user_cfg.idx = LIGEFF_IDX_BUTT;

    ligeff_clear();

    return g_ligeff_user_cfg.idx;
}

// ligeff_s ligeff_get_current_effect(void)
// {
//     ligeff_s ligeff_invalid = {
//         .idx = LIGEFF_IDX_BUTT,
//         .frame_rate = 0,
//         .frame_cnt = 0,
//         .frame_id = 0,
//     };
//     if (g_ligeff_user_cfg.idx >= LIGEFF_IDX_BUTT) {
//         return ligeff_invalid;
//     }
//     return g_ligeffs[g_ligeff_user_cfg.idx];
// }

void ligeff_solid_change_color(ligeff_idx_e ligeff_idx)
{
	//若上一次效果是常量，则颜色序号直接加1，否则颜色序号为最近一次的颜色
	if(ligeff_idx == LIGEFF_SOLID){//上一次效果是常量
		g_ligeff_user_cfg.g_ligeff_solid_color_idx++;
	}
	//否侧颜色仍为最近一次
	
	if (g_ligeff_user_cfg.g_ligeff_solid_color_idx >= LIGEFF_SOLID_COLOR_BUTT) {
        #ifdef LIGEFF_APP_ALEXA
        g_ligeff_user_cfg.g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_WARM_WHITE;
        #else
        g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_RED;
        #endif	
	}
	//ligeff_user_set_color((pixel_u *)(&(g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx])), 1);	
	ligeff_user_set_color((pixel_u *)g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx].color,g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx].color_len);
	
	/*
	static ligeff_solid_color_e last_color_idx = LIGEFF_SOLID_COLOR_WARM_WHITE;
	if(last_color_idx == g_ligeff_user_cfg.g_ligeff_solid_color_idx){
		g_ligeff_user_cfg.g_ligeff_solid_color_idx++;
	}else{
		g_ligeff_user_cfg.g_ligeff_solid_color_idx = last_color_idx;
	}
    if (g_ligeff_user_cfg.g_ligeff_solid_color_idx >= LIGEFF_SOLID_COLOR_BUTT) {
        #ifdef LIGEFF_APP_ALEXA
        g_ligeff_user_cfg.g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_WARM_WHITE;
        #else
        g_ligeff_solid_color_idx = LIGEFF_SOLID_COLOR_RED;
        #endif
    }
    ligeff_user_set_color((pixel_u *)(&(g_ligeff_solid_color[g_ligeff_user_cfg.g_ligeff_solid_color_idx])), 1);
	last_color_idx = g_ligeff_user_cfg.g_ligeff_solid_color_idx;
	*/
}

#define LIGEFF_SOLID_SECTION            (STRLIGHTS_LENGTH >> 2)
static void ligeff_show_solid(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;

    static u8 flag = 0;

    if (g_ligeff_start) {
        g_ligeff_start = 0;
		flag = 0;
    }
	if(flag == 0){
		flag = 1;
		for (int j = 0; j < STRLIGHTS_LENGTH; j++) {
			ligvm_buf_set_color(j, color[j % color_len], g_ligeff_user_cfg.luminance);
		}
	}

	/*
    // ligeff_s ligeff = g_ligeffs[LIGEFF_SOLID];
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;

    static u16 i = 0;
    static u8 delay = 0;

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        i = 0;
        delay = 0;
    }

    if (i >= STRLIGHTS_LENGTH) {
        return;
    }

    if (delay == 0) {
        for (int j = 0; (j < LIGEFF_SOLID_SECTION) && (i < STRLIGHTS_LENGTH); j++, i++) {
            ligvm_buf_set_color(i, color[i % color_len], g_ligeff_user_cfg.luminance);
        }
    }

    delay++;
    if (delay == 2) {
        delay = 0;
    }
	*/
}

// static void ligeff_show_blink(void)
// {
//     pixel_u color = g_ligeff_user_cfg.colors[0];
//     pixel_u black = {0};

//     static u16 i = 0;
//     static u8 speed = LIGEFF_SPEED_NORMAL;
//     static u8 delay = 0;

//     if (speed != g_ligeff_user_cfg.speed) {
//         speed = g_ligeff_user_cfg.speed;
//         g_ligeff_start = 1;
//     }

//     u8 delay_cnt = 0;
//     if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//         delay_cnt = 80;
//     } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//         delay_cnt = 40;
//     } else { // LIGEFF_SPEED_FAST
//         delay_cnt = 20;
//     }

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         i = 0;
//         delay = 0;
//     }

//     if (i & 1) {
//         for (int j = 0; (j < STRLIGHTS_LENGTH); j++) {
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }
//     } else {
//         for (int j = 0; (j < STRLIGHTS_LENGTH); j++) {
//             ligvm_buf_set_color(j, black, g_ligeff_user_cfg.luminance);
//         }
//     }
//     if (delay == 0) {
//         i++;
//     }
//     delay++;
//     if (delay == delay_cnt) {
//         delay = 0;
//     }
// }

static void ligeff_show_twinkle(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;
    pixel_u black = {0};

    static u16 i = 0;
    static u8 speed = LIGEFF_SPEED_NORMAL;
    static u8 delay = 0;

    if (speed != g_ligeff_user_cfg.speed) {
        speed = g_ligeff_user_cfg.speed;
        g_ligeff_start = 1;
    }

    u8 delay_cnt = 0;
    if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
        delay_cnt = 100;
    } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
        delay_cnt = 60;
    } else { // LIGEFF_SPEED_FAST
        delay_cnt = 20;
    }

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        i = 0;
        delay = 0;
    }

    for (int j = 0; j < STRLIGHTS_LENGTH; j++) {
        if (j % LIGEFF_TWINKLE_APART == i) {
            ligvm_buf_set_color(j, color[j % color_len], g_ligeff_user_cfg.luminance);
        } else {
            ligvm_buf_set_color(j, black, g_ligeff_user_cfg.luminance);
        }
    }
    if (delay == 0) {
        i++;
        i %= LIGEFF_TWINKLE_APART;
    }
    delay++;
    if (delay == delay_cnt) {
        delay = 0;
    }
}

#if (STRLIGHTS_LENGTH >= 250)
static const u8 g_glisten_luminances_slow[] = {200, 160, 130, 100, 80, 60, 45, 30, 20, 10, 5, 2, 1};
static const u8 g_glisten_luminances_normal[] = {200, 130, 90, 60, 30, 10, 5, 1};
static const u8 g_glisten_luminances_fast[] = {200, 110, 50, 10, 1};
#else
u8 g_glisten_luminances[12] = {200, 150, 100, 80, 60, 45, 30, 20, 10, 5, 2, 1};
#endif
static void ligeff_show_glisten(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;

    static u8 speed = LIGEFF_SPEED_NORMAL;
    static u8 direction = 0;
    static u8 index = 0;

    const u8 *p_luminances = NULL;
    u8 luminances_len = 0;

    if (speed != g_ligeff_user_cfg.speed) {
        speed = g_ligeff_user_cfg.speed;
        g_ligeff_start = 1;
    }

//    if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//        p_luminances = g_glisten_luminances_slow;
//        luminances_len = sizeof(g_glisten_luminances_slow);
//    } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//        p_luminances = g_glisten_luminances_normal;
//        luminances_len = sizeof(g_glisten_luminances_normal);
//    } else { // LIGEFF_SPEED_FAST
//        p_luminances = g_glisten_luminances_fast;
//        luminances_len = sizeof(g_glisten_luminances_fast);
//    }

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        direction = 0;
        index = 0;
    }

    u8 luminance_0 = (u8)(((u16)p_luminances[index] * g_ligeff_user_cfg.luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);
    u8 luminance_1 = (u8)(((u16)p_luminances[luminances_len - 1 - index] * g_ligeff_user_cfg.luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);

    for (int j = 0; j < STRLIGHTS_LENGTH; j += 2) {
        ligvm_buf_set_color(j, color[j % color_len], luminance_0);
        ligvm_buf_set_color(j + 1, color[(j + 1) % color_len], luminance_1);
    }

    if (direction == 0) {
        index++;
        if (index == luminances_len - 1) {
            direction = 1;
        }
    } else {
        index--;
        if (index == 0) {
            direction = 0;
        }
    }
}

static void ligeff_show_shimmer(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;

    static u8 speed = LIGEFF_SPEED_NORMAL;
    static u8 direction = 0;
    static u8 index = 0;

    const u8 *p_luminances = NULL;
    u8 luminances_len = 0;

    if (speed != g_ligeff_user_cfg.speed) {
        speed = g_ligeff_user_cfg.speed;
        g_ligeff_start = 1;
    }

//    if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//        p_luminances = g_glisten_luminances_slow;
//        luminances_len = sizeof(g_glisten_luminances_slow);
//    } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//        p_luminances = g_glisten_luminances_normal;
//        luminances_len = sizeof(g_glisten_luminances_normal);
//    } else { // LIGEFF_SPEED_FAST
//        p_luminances = g_glisten_luminances_fast;
//        luminances_len = sizeof(g_glisten_luminances_fast);
//    }

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        direction = 0;
        index = 0;
    }

    u8 luminance_0 = (u8)(((u16)p_luminances[index] * g_ligeff_user_cfg.luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);
    for (int j = 0; j < STRLIGHTS_LENGTH; j++) {
        ligvm_buf_set_color(j, color[j % color_len], luminance_0);
    }

    if (direction == 0) {
        index++;
        if (index == luminances_len - 1) {
            direction = 1;
        }
    } else {
        index--;
        if (index == 0) {
            direction = 0;
        }
    }
}

static void ligeff_show_step_breath(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;

    static u8 speed = LIGEFF_SPEED_NORMAL;
    static u8 direction = 0;
    static u8 index = 0;
    static u8 step = 0;

    const u8 *p_luminances = NULL;
    u8 luminances_len = 0;

    if (speed != g_ligeff_user_cfg.speed) {
        speed = g_ligeff_user_cfg.speed;
        g_ligeff_start = 1;
    }

//    if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//        p_luminances = g_glisten_luminances_slow;
//        luminances_len = sizeof(g_glisten_luminances_slow);
//    } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//        p_luminances = g_glisten_luminances_normal;
//        luminances_len = sizeof(g_glisten_luminances_normal);
//    } else { // LIGEFF_SPEED_FAST
//        p_luminances = g_glisten_luminances_fast;
//        luminances_len = sizeof(g_glisten_luminances_fast);
//    }

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        direction = 0;
        index = 0;
        step = 0;
    }

    u8 luminance_0 = (u8)(((u16)p_luminances[luminances_len - 1 - index] * g_ligeff_user_cfg.luminance) / STRLIGHTS_LUMINANCE_MAX_VAL);
    for (int j = 0; j < STRLIGHTS_LENGTH; j += 2) {
        if (step == 0) {
            ligvm_buf_set_color(j, color[j % color_len], luminance_0);
            ligvm_buf_set_color(j + 1, color[(j + 1) % color_len], 0);
        } else {
            ligvm_buf_set_color(j, color[j % color_len], 0);
            ligvm_buf_set_color(j + 1, color[(j + 1) % color_len], luminance_0);
        }

    }

    if (direction == 0) {
        index++;
        if (index == luminances_len - 1) {
            direction = 1;
        }
    } else {
        index--;
        if (index == 0) {
            direction = 0;
            step++;
            if (step == 2) {
                step = 0;
            }
        }
    }
}

static void ligeff_show_firecracker(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;
    pixel_u black = {0};

    static u16 i = 0;
    static u8 speed = LIGEFF_SPEED_NORMAL;
    static u8 delay = 0;
    static u8 fire_cnt = 0;

    if (speed != g_ligeff_user_cfg.speed) {
        speed = g_ligeff_user_cfg.speed;
        g_ligeff_start = 1;
    }

    u8 delay_cnt = 0;
    if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
        delay_cnt = 40;
    } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
        delay_cnt = 20;
    } else { // LIGEFF_SPEED_FAST
        delay_cnt = 1;
    }

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        i = 0;
        delay = 0;
    }

    if (fire_cnt > 0) {
        for (int j = 0; j < STRLIGHTS_LENGTH; j++) {
            if ((j % LIGEFF_TWINKLE_APART == i) && (fire_cnt % 2 == 0)) {
                ligvm_buf_set_color(j, color[j % color_len], g_ligeff_user_cfg.luminance);
            } else {
                ligvm_buf_set_color(j, black, g_ligeff_user_cfg.luminance);
            }
        }
        fire_cnt--;
        return;
    }

    for (int j = 0; j < STRLIGHTS_LENGTH; j++) {
        if (j % LIGEFF_TWINKLE_APART == i) {
            ligvm_buf_set_color(j, color[j % color_len], g_ligeff_user_cfg.luminance);
        } else {
            ligvm_buf_set_color(j, black, g_ligeff_user_cfg.luminance);
        }
    }
    if (delay == 0) {
        i++;
        i %= LIGEFF_TWINKLE_APART;
        fire_cnt = 4;
    }
    delay++;
    if (delay == delay_cnt) {
        delay = 0;
    }
}

static void ligeff_show_flash(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;
    pixel_u black = {0};

    static u16 i = 0;
    static u8 speed = LIGEFF_SPEED_NORMAL;
    static u8 delay = 0;

    if (speed != g_ligeff_user_cfg.speed) {
        speed = g_ligeff_user_cfg.speed;
        g_ligeff_start = 1;
    }

    u8 delay_cnt = 0;
    if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
        delay_cnt = 80;
    } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
        delay_cnt = 40;
    } else { // LIGEFF_SPEED_FAST
        delay_cnt = 20;
    }

    if (g_ligeff_start) {
        g_ligeff_start = 0;
        i = 0;
        delay = 0;
    }

    if (i & 1) {
        for (int j = 0; (j < STRLIGHTS_LENGTH); j++) {
            ligvm_buf_set_color(j, color[j % color_len], g_ligeff_user_cfg.luminance);
        }
    } else {
        for (int j = 0; (j < STRLIGHTS_LENGTH); j++) {
            ligvm_buf_set_color(j, black, g_ligeff_user_cfg.luminance);
        }
    }
    if (delay == 0) {
        i++;
    }
    delay++;
    if (delay == delay_cnt) {
        delay = 0;
    }
}

static u16 rand_tmp[100] = {0};
static void ligeff_show_sparkle(void)
{
    pixel_u *color = g_ligeff_user_cfg.colors;
    u8 color_len = g_ligeff_user_cfg.color_len;
    pixel_u flashing = g_ligeff_default_color[LIGEFF_DEF_COLOR_WHITE];

    //static u16 i = 0;
    if (g_ligeff_start) {
        g_ligeff_start = 0;
        //i = 0;
    }

    static int j = 0;
    if (j == 0) {
        for (int i = 0; i < 100; i++) {
            rand_tmp[i] = rand() % STRLIGHTS_LENGTH;
        }
    }
    for (; j < STRLIGHTS_LENGTH;) {
        u8 background_flag = 1;
        pixel_u background = color[j % color_len];
        if (background.value == g_ligeff_default_color[LIGEFF_DEF_COLOR_WHITE].value) {
            flashing.value = 0x000000ff;
        }
        for (int i = 0; i < 100; i++) {
            if (rand_tmp[i] == j) {
                ligvm_buf_set_color(j, flashing, g_ligeff_user_cfg.luminance);
                background_flag = 0;
                break;
            }
        }
        if (background_flag) {
            ligvm_buf_set_color(j, background, g_ligeff_user_cfg.luminance);
        }
        // if (rand() % 3 == 0) {
        //     ligvm_buf_set_color(j, flashing, g_ligeff_user_cfg.luminance);
        // } else {
        //     ligvm_buf_set_color(j, background, g_ligeff_user_cfg.luminance);
        // }
        j++;
        if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
            if ((j % 10 == 0) && (j < STRLIGHTS_LENGTH)) {
                return;
            }
        } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
            if ((j % 40 == 0) && (j < STRLIGHTS_LENGTH)) {
                return;
            }
        } else { // LIGEFF_SPEED_FAST
            if ((j % 80 == 0) && (j < STRLIGHTS_LENGTH)) {
                return;
            }
        }

    }

    j = 0;
}

// static void ligeff_show_fallranbow(void)
// {
//     pixel_u color;
//     pixel_u color2;
//     u8 color_length = (u8)LIGEFF_FALLRANBOW_COLOR_BUTT;

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;

//         a.ltstr_length = STRLIGHTS_LENGTH;
//         a.addr = a.ltstr_length - 1;
//         mem_set(a.u, 0, a.ltstr_length); // color
//         a.step = (a.ltstr_length / 3) < 0x7F ? (a.ltstr_length / 3) : 0x7F;
//         mem_set(a.v, 0, a.ltstr_length); // lum
//     }

//     static int j = 0;
//     for (; j < a.ltstr_length;) {
//         if (a.v[j] & 0x80) {
//             int i = (a.u[j] + 1) % color_length;
//             u32 lum = a.v[j] & 0x7F;

//             color = g_ligeff_fallranbow_color[a.u[j]];
//             color2 = g_ligeff_fallranbow_color[i];
//             color.rgb.r = ((u32)color.rgb.r * (a.step - lum) + color2.rgb.r * (lum)) / a.step;
//             color.rgb.g = ((u32)color.rgb.g * (a.step - lum) + color2.rgb.g * (lum)) / a.step;
//             color.rgb.b = ((u32)color.rgb.b * (a.step - lum) + color2.rgb.b * (lum)) / a.step;

//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         } else {
//             color.value = 0;
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }

//         if (a.v[j] & (1<<7)) {
//             a.v[j]++;
//             if ((a.v[j] & 0x7F) == a.step) {
//                 a.u[j] = (a.u[j] + 1) % color_length;
//                 a.v[j] = (1<<7);
//             }
//         }

//         j++;
//         if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//             if (j % 10 == 0) {
//                 video_memory_handler.update_status = VM_update_busy;
//                 return;
//             }
//         } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//             if (j % 50 == 0) {
//                 video_memory_handler.update_status = VM_update_busy;
//                 return;
//             }
//         } else { // LIGEFF_SPEED_FAST
//             if (j % 100 == 0) {
//                 video_memory_handler.update_status = VM_update_busy;
//                 return;
//             }
//         }

//     }
//     if (a.addr >= 0) {
//         a.v[a.addr] |= (1 << 7);
//         a.addr--;
//     }
//     j = 0;
// }

// static void ligeff_show_theme_rainbow(void)
// {
//     // pixel_u color;
//     // pixel_u color2;
//     // u8 color_length = (u8)LIGEFF_DEF_COLOR_BLACK;

//     // if (g_ligeff_start) {
//     //     g_ligeff_start = 0;
//     //     a.ltstr_length = STRLIGHTS_LENGTH;
//     //     a.addr = a.ltstr_length - 1;
//     //     mem_set(a.u, 0, a.ltstr_length);    // color
//     //     a.step = (a.ltstr_length / 3) < 0x7F ? (a.ltstr_length / 3) : 0x7F;
//     //     mem_set(a.v, 0, a.ltstr_length);    // lum
//     // }
//     // for(int j = 0;j < a.ltstr_length;j++){
//     //     if(a.v[j] & 0x80){
//     //         int i = (a.u[j] + 1) % color_length;
//     //         u32 lum = a.v[j] & 0x7F;
//     //         color = g_ligeff_default_color[a.u[j]];
//     //         color2 = g_ligeff_default_color[i];
//     //         color.rgb.r = ((u32)color.rgb.r * (a.step - lum) + color2.rgb.r * (lum)) / a.step;
//     //         color.rgb.g = ((u32)color.rgb.g * (a.step - lum) + color2.rgb.g * (lum)) / a.step;
//     //         color.rgb.b = ((u32)color.rgb.b * (a.step - lum) + color2.rgb.b * (lum)) / a.step;
//     //         ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//     //     }else{
//     //         color.value = 0;
//     //         ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//     //     }

//     //     if(a.v[j] & (1<<7)){
//     //         a.v[j] ++ ;
//     //         if((a.v[j] & 0x7F) == a.step){
//     //             a.u[j] = (a.u[j] + 1) % color_length;
//     //             a.v[j] = (1<<7);
//     //         }
//     //     }
//     // }
//     // if(a.addr >= 0){
//     //     a.v[a.addr] |= (1 << 7);
//     //     a.addr--;
//     // }
// }

// static void ligeff_show_fallsnake(void)
// {
//     static u8 speed = LIGEFF_SPEED_NORMAL;
//     static u8 delay = 0;

//     if (speed != g_ligeff_user_cfg.speed) {
//         speed = g_ligeff_user_cfg.speed;
//         g_ligeff_start = 1;
//     }

//     u8 delay_cnt = 0;
//     if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//         delay_cnt = 6;
//     } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//         delay_cnt = 3;
//     } else { // LIGEFF_SPEED_FAST
//         delay_cnt = 1;
//     }

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         a.ltstr_length = STRLIGHTS_LENGTH;
//         a.addr = a.ltstr_length - 1;
//         a.i = 0;
//         a.flag = 0;

//         delay = 0;
//     }

//     if ((delay % delay_cnt) == 0) {
//         delay = 0;
//     }
//     delay++;
//     if (delay != 1) {
//         return;
//     }

//     if (a.flag == 0) {
//         for (int j = 0;j < a.ltstr_length;j++) {
//             pixel_u color = g_ligeff_default_color[a.i];
//             if (j >= a.addr) {
//                 ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//             } else {
//                 color.value = 0;
//                 ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//             }
//         }
//         a.addr--;
//         if(a.addr < 0){
//             a.addr = a.ltstr_length - 1;
//             a.i = (a.i + 1) % LIGEFF_DEF_COLOR_BLACK; // LIGEFF_DEF_COLOR_BUTT // cb->color_length;
//             a.flag = 1;
//             a.temp = max_changed_number;
//             max_changed_number = STRLIGHTS_LENGTH;
//         }
//         video_memory_handler.update_status = VM_update_busy;
//     } else {
//         pixel_u color = {0};
//         foreach(j,a.ltstr_length) {
//             color.rgb.r = g_ligeff_default_color[LIGEFF_DEF_COLOR_WHITE].rgb.r * (5 - a.flag) / 4;
//             color.rgb.g = g_ligeff_default_color[LIGEFF_DEF_COLOR_WHITE].rgb.g * (5 - a.flag) / 4;
//             color.rgb.b = g_ligeff_default_color[LIGEFF_DEF_COLOR_WHITE].rgb.b * (5 - a.flag) / 4;

//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }
//         a.flag ++;
//         if (a.flag == 6) {
//             a.flag = 0;
//             max_changed_number = a.temp;
//         }
//         video_memory_handler.update_status = VM_update_coherent;
//     }
// }

// static const float g_speed_cali_map[30] = {
//     9.274, 8.377, 7.5709, 6.8085, 6.097,
//     5.396, 4.74, 4.0507, 3.4934, 3.27865,
//     3.0889, 2.9, 2.725, 2.54778, 2.4024,
//     2.27, 2.153, 2.0465, 1.92, 1.785,
//     1.667, 1.56, 1.468, 1.387, 1.315,
//     1.248, 1.19, 1.135, 1.0855, 1.041
// };
// #define SPPED_CAILB     ((LIGEFF_CRASH_POINT >= 20)?((LIGEFF_CRASH_POINT<50)?g_speed_cali_map[LIGEFF_CRASH_POINT-20]: 1):1)
// #define CAILB_VAULE(x)  ((int)((a.ltstr_length-(x))*SPPED_CAILB/a.ltstr_length)+1)

// extern void ligvm_buf_set_color2(u16 addr, pixel_u rgb, u8 luminance);
// static void ligeff_show_collide(void)
// {
//     u32 r,g,b;
//     #ifdef STRLIGHTS_COLOR_RGBW
//     u32 w;
//     #endif
//     pixel_u color = g_ligeff_user_cfg.colors[0];
//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         a.ltstr_length = STRLIGHTS_LENGTH;
//         mem_set(a.v, 0, (a.ltstr_length));
//         a.i = a.ltstr_length -1;
//         a.i2 = 0;
//         a.flag = 0;
//         a.step = (a.ltstr_length>>2) < 0x7f ? (a.ltstr_length>>2) : 0x7f;
//     }
//     for(int j=0;j<a.ltstr_length;j++){
//         if ((a.v[j] & 0x7F)) {
//             u32 lum;
//             lum = ((((u32)a.v[j] & 0x7F) << 8) / a.step) - 1;
//             #ifdef STRLIGHTS_COLOR_RGBW
//             w = lum * (u32)color.rgb.w;
//             #endif
//             r = lum * (u32)color.rgb.r;
//             g = lum * (u32)color.rgb.g;
//             b = lum * (u32)color.rgb.b;
//             #ifdef STRLIGHTS_COLOR_RGBW
//             w = (w >> 8) + ((w & 0xFF) ? 1 : 0);
//             #endif
//             r = (r >> 8) + ((r & 0xFF) ? 1 : 0);
//             g = (g >> 8) + ((g & 0xFF) ? 1 : 0);
//             b = (b >> 8) + ((b & 0xFF) ? 1 : 0);

//             #ifdef STRLIGHTS_COLOR_RGBW
//             if(XOR(w, color.rgb.w) || XOR(r, color.rgb.r) || XOR(g, color.rgb.g) || XOR(b, color.rgb.b)){
//                 w = r = g = b = 0;
//             }
//             #else
//             if(XOR(r, color.rgb.r) || XOR(g, color.rgb.g) || XOR(b, color.rgb.b)){
//                 r = g = b = 0;
//             }
//             #endif
//             pixel_u color_now;

//             #ifdef STRLIGHTS_COLOR_RGBW
//             color_now.rgb.w = w;
//             #endif
//             color_now.rgb.r = r;
//             color_now.rgb.g = g;
//             color_now.rgb.b = b;
//             ligvm_buf_set_color(j, color_now, g_ligeff_user_cfg.luminance);
//         }else{
//             pixel_u color_now;
//             color_now.value = 0;
//             ligvm_buf_set_color(j, color_now, g_ligeff_user_cfg.luminance);
//         }
//         if(!(a.v[j] & (1<<7))){
//             if((a.v[j] & 0x7F)>0){
//                 a.v[j] -- ;
//             }
//         }
//     }
//     if(a.flag == 0){
//         for(int i = 0;i < CAILB_VAULE(a.i);i++){
//             a.v[a.i] = a.step;
//             a.i --;
//         }
//         for(int i = 0;i < CAILB_VAULE(a.i2+1);i++){
//             a.v[a.i2] = a.step;
//             a.i2 ++;
//         }
//         if(a.i <= a.i2){
//             a.i2 = a.i = ((a.ltstr_length+1)*(100-LIGEFF_CRASH_POINT)/100);
//             a.flag = 1;
//         }
//     }else if(a.flag == 1){
//         for(int i = 0;i < CAILB_VAULE(a.i+1);i++){
//             if(a.i > a.ltstr_length-1){
//                 break;
//             }
//             a.v[a.i] = (1<<7) | a.step;
//             a.i++;
//         }
//         for(int i = 0;i < CAILB_VAULE(a.i2);i++){
//             if(a.i2 < 0){
//                 break;
//             }
//             a.v[a.i2] = (1<<7) | a.step;
//             a.i2--;
//         }
//         if(a.i2 < 0 && a.i > a.ltstr_length-1){
//             a.i2 = a.i = ((a.ltstr_length+1)*(100-LIGEFF_CRASH_POINT)/100);
//             a.flag = 2;
//         }
//     }else if(a.flag == 2){
//         for(int i = 0;i < CAILB_VAULE(a.i+1);i++){
//             if(a.i > a.ltstr_length-1){
//                 break;
//             }
//             a.v[a.i+1] &= ~(1<<7);
//             a.i++;
//         }
//         for(int i = 0;i < CAILB_VAULE(a.i2);i++){
//             if(a.i2 < 0){
//                 break;
//             }
//             a.v[a.i2] &= ~(1<<7);
//             a.i2--;
//         }
//         if(a.i2 < 0 && a.i > a.ltstr_length-1){
//             a.i2 = a.step;
//             a.flag = 3;
//         }
//     }else if(a.flag == 3){
//         a.i2--;
//         if(a.i2 < 0){
//             a.i2 = 0;
//             a.i = a.ltstr_length -1;
//             a.flag = 0;
//         }
//     }
//     video_memory_handler.update_status = VM_update_busy;
//     return;
// }

// static void ligeff_show_anakonda(void)
// {
//     static u8 speed = LIGEFF_SPEED_NORMAL;
//     static u8 delay = 0;

//     if (speed != g_ligeff_user_cfg.speed) {
//         speed = g_ligeff_user_cfg.speed;
//         g_ligeff_start = 1;
//     }

//     u8 delay_cnt = 0;
//     if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//         delay_cnt = 80;
//     } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//         delay_cnt = 40;
//     } else { // LIGEFF_SPEED_FAST
//         delay_cnt = 20;
//     }

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         a.ltstr_length = STRLIGHTS_LENGTH;
//         a.temp = 1;
//         a.i = 4;
//         a.flag = 0;
//         a.length = 4;
//         a.i2 = ((u16)my_rand(0) % a.ltstr_length);
//         while(1){
//             a.i2 = ((u16)my_rand(-1) % a.ltstr_length);
//             if(a.i2 > a.length + 1){
//                 break;
//             }
//         }

//         delay = 0;
//     }

//     if ((delay % delay_cnt) == 0) {
//         delay = 0;
//     }
//     delay++;
//     if (delay != 1) {
//         return;
//     }

//     if(a.flag == 0){
//         pixel_u color;
//         for(int j = 0;j < a.ltstr_length;j++){
//             if(j == a.i){
//                 color = g_ligeff_default_color[0];
//                 ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//             }else if(j == a.i2){
//                 color = g_ligeff_default_color[1];
//                 ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//             }else if((a.temp == 1 && j < a.i && j >= a.i - a.length) || (a.temp == 0 && j > a.i && j <= a.i + a.length)){
//                 color = g_ligeff_default_color[0];

//                 pixel_u color1;
//                 color1.rgb.r = color.rgb.r * 4 / 5;
//                 color1.rgb.g = color.rgb.g * 4 / 5;
//                 color1.rgb.b = color.rgb.b * 4 / 5;

//                 ligvm_buf_set_color(j, color1, g_ligeff_user_cfg.luminance);
//             }else{
//                 color.value = 0;
//                 ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);

//             }
//         }
//         if(a.temp == 1){
//             a.i++;
//         }else{
//             a.i--;
//         }
//         if(a.i == a.i2){
//             a.length ++;
//             while(1){
//                 a.i2 = ((u16)my_rand(-1) % a.ltstr_length);
//                 if((a.temp == 1 && a.i2 <= a.i && a.i2 >= a.i - a.length) || (a.temp == 0 && a.i2 >= a.i && a.i2 <= a.i + a.length)){
//                     continue;
//                 }else{
//                     break;
//                 }
//             }
//             if(a.temp == 1 && a.i2 < a.i){
//                 a.temp = 0;
//                 a.i = a.i - a.length;
//             }else if(a.temp == 0 && a.i2 > a.i){
//                 a.i = a.i + a.length;
//                 a.temp = 1;
//             }
//             if(a.length > a.ltstr_length / 2){
//                 a.flag = 1;
//                 a.temp = 1;
//                 a.i = 4;
//                 a.length = 4;
//                 while(1){
//                     a.i2 = ((u16)my_rand(-1) % a.ltstr_length);
//                     if(a.i2 > a.length + 1){
//                         break;
//                     }
//                 }
//             }
//         }
//     } else {
//         pixel_u color;
//         for(int j=0;j<a.ltstr_length;j++){
//             color.rgb.r = 300-60*a.flag;
//             color.rgb.g = 300-60*a.flag;
//             color.rgb.b = 300-60*a.flag;
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }
//         a.flag++;
//         if(a.flag == 6){
//             a.flag = 0;
//         }
//     }
// }

// static void ligeff_show_glow(void)
// {
//     u32 r, g, b;
//     u8 color_length = LIGEFF_DEF_COLOR_BLACK;

//     static u8 speed = LIGEFF_SPEED_NORMAL;
//     static u8 delay = 0;

//     if (speed != g_ligeff_user_cfg.speed) {
//         speed = g_ligeff_user_cfg.speed;
//         g_ligeff_start = 1;
//     }

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         a.ltstr_length = STRLIGHTS_LENGTH;
//         mem_set(a.v, 0, a.ltstr_length);
//         mem_set(a.u, 0, a.ltstr_length);

//         delay = 0;
//     }

//     u8 delay_cnt = 0;
//     if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//         delay_cnt = 80;
//     } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//         delay_cnt = 35;
//     } else { // LIGEFF_SPEED_FAST
//         delay_cnt = 2;
//     }

//     if ((delay % delay_cnt) == 0) {
//         delay = 0;
//     }
//     delay++;
//     if (delay != 1) {
//         return;
//     }

//     for(int j=0;j<a.ltstr_length;j++){
//         pixel_u color = g_ligeff_default_color[a.u[j]];
//         if((a.v[j] & 0x1F)){
//             r = (((u32)a.v[j] & 0x1F) << 3)*(u32)color.rgb.r;
//             g = (((u32)a.v[j] & 0x1F) << 3)*(u32)color.rgb.g;
//             b = (((u32)a.v[j] & 0x1F) << 3)*(u32)color.rgb.b;
//             r = (r >> 8) + ((r & 0xFF) ? 1 : 0);
//             g = (g >> 8) + ((g & 0xFF) ? 1 : 0);
//             b = (b >> 8) + ((b & 0xFF) ? 1 : 0);
//             if(XOR(r , color.rgb.r) || XOR(g , color.rgb.g) || XOR(b , color.rgb.b)){
//                 r = g = b = 0;
//             }
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }else{
//             color.value = 0;
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }
//         if(a.v[j] & (1<<7)){
//             if((a.v[j] & 0x1F) < 0x1F){
//                 a.v[j] ++ ;
//             }else{
//                 a.v[j] &= ~(1<<7);
//             }
//         }else{
//             if((a.v[j] & 0x1F) > 0x00){
//                 a.v[j] -- ;
//             }
//         }
//         if(!((u16)rand() % 50)){
//             if((a.v[j] & 0x1F) == 0){
//                 a.v[j] |= (1<<7);
//                 a.u[j] = rand() % color_length;
//             }
//         }
//     }
// }

// static void ligeff_show_theYOYO(void)
// {
//     pixel_u color;
//     u8 color_length = LIGEFF_DEF_COLOR_BLACK;

//     static u8 speed = LIGEFF_SPEED_NORMAL;
//     static u8 delay = 0;

//     if (speed != g_ligeff_user_cfg.speed) {
//         speed = g_ligeff_user_cfg.speed;
//         g_ligeff_start = 1;
//     }

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         a.addr = 0;
//         a.ltstr_length = STRLIGHTS_LENGTH;
//         a.flag = 0;
//         a.i = 0;

//         delay = 0;
//     }

//     u8 delay_cnt = 0;
//     if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//         delay_cnt = 10;
//     } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//         delay_cnt = 5;
//     } else { // LIGEFF_SPEED_FAST
//         delay_cnt = 1;
//     }

//     if ((delay % delay_cnt) == 0) {
//         delay = 0;
//     }
//     delay++;
//     if (delay != 1) {
//         return;
//     }

//     if (a.flag == 0) {
//         for (int j = 0;(j < (a.ltstr_length >> 3)) && (a.addr < a.ltstr_length);j++,a.addr++) {
//             color = g_ligeff_default_color[a.i];
//             ligvm_buf_set_color(a.addr, color, g_ligeff_user_cfg.luminance);
//         }
//         if (a.addr >= a.ltstr_length) {
//             a.i = (a.i + 1) % color_length;
//             a.flag = 1;
//             a.addr--;
//         }
//     } else {
//         for (int j = 0;(j < (a.ltstr_length >> 3)) && (a.addr >= 0);j++,a.addr--) {
//             color = g_ligeff_default_color[a.i];
//             ligvm_buf_set_color(a.addr, color, g_ligeff_user_cfg.luminance);
//         }
//         if (a.addr < 0) {
//             a.i = (a.i + 1) % color_length;
//             a.flag = 0;
//             a.addr++;
//         }
//     }
// }

// #define LIGEFF_WAVE_INV_EACHLENGTH  20
// static void ligeff_show_wave_inv(void)
// {
//     u8 color_length = LIGEFF_DEF_COLOR_BLACK;
//     u16 BufLength = color_length * LIGEFF_WAVE_INV_EACHLENGTH;

//     static u8 speed = LIGEFF_SPEED_NORMAL;
//     static u8 delay = 0;

//     if (speed != g_ligeff_user_cfg.speed) {
//         speed = g_ligeff_user_cfg.speed;
//         g_ligeff_start = 1;
//     }

//     if (g_ligeff_start) {
//         g_ligeff_start = 0;
//         u8 index = 0;
//         a.ltstr_length = STRLIGHTS_LENGTH;
//         a.i = 0;
//         mem_set(a.v, 0, a.ltstr_length);
//         for (int j = 0; j < BufLength;) {
//             for (int k = 0; k < LIGEFF_WAVE_INV_EACHLENGTH; k++, j++){
//                 a.v[j] = index;
//             }
//             index = (index+1) % color_length;
//         }

//         delay = 0;
//     }

//     u8 delay_cnt = 0;
//     if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
//         delay_cnt = 30;
//     } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
//         delay_cnt = 15;
//     } else { // LIGEFF_SPEED_FAST
//         delay_cnt = 1;
//     }

//     if ((delay % delay_cnt) == 0) {
//         delay = 0;
//     }
//     delay++;
//     if (delay != 1) {
//         return;
//     }

//     for (int j = 0; j < a.ltstr_length; j++) {
//         u8 index;
//         index = (a.i-j)>=0 ? (a.i-j)%BufLength : BufLength;

//         pixel_u color;
//         if (index < BufLength) {
//             color = g_ligeff_default_color[a.v[index]];
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         } else {
//             color.value = 0;
//             ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//         }
//     }

//     a.i++;
//     if (a.i < 0) {
//         a.i = 0;
//     }
// }

// static void ligeff_show_random_color(void)
// {
//     // pixel_u color;
//     // u8 color_length = LIGEFF_DEF_COLOR_BLACK;
//     // static u8 delay = 0;
//     // if (g_ligeff_start) {
//     //     g_ligeff_start = 0;
//     //     a.ltstr_length = STRLIGHTS_LENGTH;
//     //     if (color_length) {
//     //         a.intensity = g_ligeff_default_color[0].rgb.r;
//     //     }else{
//     //         a.intensity = 0xFF;
//     //     }
//     // }

//     // if (delay == 0) {
//     //     for(int j = 0;j<a.ltstr_length;j++){
//     //         color.rgb.r = (u8)(rand()%a.intensity);
//     //         color.rgb.g = (u8)(rand()%a.intensity);
//     //         color.rgb.b = (u8)(rand()%a.intensity);
//     //         ligvm_buf_set_color(j, color, g_ligeff_user_cfg.luminance);
//     //     }
//     // }
//     // delay++;
//     // if (delay == 2) {
//     //     delay = 0;
//     // }
// }

void ligeff_refresh(ligeff_idx_e ligeff_idx)
{
    static u8 cfg_save_flag = 0;
    static u32 cfg_save_time = 0;
    static u32 refresh_time = 0;
    static u32 interval = 10000;
    if (ligeff_idx >= LIGEFF_IDX_BUTT) {
        return;
    }
    if (ligeff_idx != g_ligeff_user_cfg.idx) {
        // 更新效果
        g_ligeff_user_cfg.idx = ligeff_idx;
    }
    if (ligvm_buf_get_status() == LIGVM_STATUS_IDLE) {
        g_ligeff_start = 1;
        cfg_save_flag = 1;
        cfg_save_time = SYS_TIME_GET_MS();
        refresh_time = SYS_TICK_GET_VAL();

        ligvm_buf_set_status(LIGVM_STATUS_WRITE);

    //     if ((g_ligeff_user_cfg.idx == LIGEFF_SOLID) ||
    //         (g_ligeff_user_cfg.idx == LIGEFF_GLISTEN) ||
    //         (g_ligeff_user_cfg.idx == LIGEFF_FALLRAINBOW)) {
    //         interval = 0;
    //     } else if (g_ligeff_user_cfg.idx == LIGEFF_TWINKLE) {
    //         interval = 25000;
    //     } else {
    //         if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_SLOW) {
    //             interval = 35000;
    //         } else if (g_ligeff_user_cfg.speed == LIGEFF_SPEED_NORMAL) {
    //             interval = 25000;
    //         } else { // LIGEFF_SPEED_FAST
    //             interval = 15000;
    //         }
    //     }
    }

    if (g_ligeffs[g_ligeff_user_cfg.idx].handle != NULL) {
        if (sys_time_exceed_us(refresh_time, interval)) {
            refresh_time = SYS_TICK_GET_VAL();
            g_ligeffs[g_ligeff_user_cfg.idx].handle();

            ligvm_buf_set_status(LIGVM_STATUS_WRITE_DONE);
            video_memory_handler.update_status = VM_update_over;
            video_memory_handler.display_status = VM_display_ready;
        }
    }

    if (cfg_save_flag) {
        if (sys_time_exceed_ms(cfg_save_time, LIGEFF_CFG_SAVE_TIME)) {
            // save cfg
            cfg_save_time = SYS_TIME_GET_MS();
            cfg_save_flag = 0;

            // ligsdata_write(g_ligeff_user_cfg);
        }
    }
}

void ligeff_load(void)
{
    u32 time1 = SYS_TICK_GET_VAL();
    // ligvm_buf_set_status(LIGVM_STATUS_READ);
    if ((g_ligeff_user_cfg.idx == LIGEFF_SOLID) ||
        (g_ligeff_user_cfg.idx == LIGEFF_STEPBREATH)) {
        ligeff_show_frames();
    } else {
        ligeff_fast_show_frames();
    }
    u32 time2 = SYS_TICK_GET_VAL();

    // ligvm_buf_set_status(LIGVM_STATUS_IDLE);
    video_memory_handler.display_status = VM_display_over;
}

void ligeff_handler(ligeff_idx_e ligeff_idx)
{
    if ((video_memory_handler.display_status == VM_display_over) || (g_ligeff_start == 1)) {
        ligeff_refresh(ligeff_idx);
    }
    if (video_memory_handler.display_status == VM_display_ready) {
        ligeff_load();
    }
}
