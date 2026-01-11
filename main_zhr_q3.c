#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>
#include <stdbool.h>
#include "pico/stdlib.h"
#include "bsp.h"
#include "../common/workload.h"

/******** Config ********/
#define F_MS 5u                 // minor frame = 5 ms
#define H_MS 100u               // hyperperiod = 100 ms
#define USE_DIP_FOR_TASK_C 1    // 启用拨码控制 Task_C 耗时

#define FRAMES_PER_HYPER (H_MS / F_MS)
#define MAX_TASKS_PER_FRAME 4

/******** Task meta ********/
typedef enum { TA, TB, TC, TD, TE, TF, TASK_CNT } task_id_t;

typedef struct {
    const char* name;
    uint32_t    period_ms;
} task_meta_t;

static const task_meta_t TASK[TASK_CNT] = {
    [TA] = { "Task_A", 10 },
    [TB] = { "Task_B",  5 },
    [TC] = { "Task_C", 25 },
    [TD] = { "Task_D", 50 },
    [TE] = { "Task_E", 50 },
    [TF] = { "Task_F", 20 },
};

/******** Frame table ********/
#define X 255u
static const uint8_t FRAME_PLAN[FRAMES_PER_HYPER][MAX_TASKS_PER_FRAME] = {
    /*0*/  { TB, TA, X,  X  },
    /*1*/  { TB, TF, X,  X  },
    /*2*/  { TB, TD, TA, X  },
    /*3*/  { TB, TC, X,  X  },
    /*4*/  { TB, TA, X,  X  },
    /*5*/  { TB, TE, X,  X  },
    /*6*/  { TB, TA, X,  X  },
    /*7*/  { TB, TF, X,  X  },
    /*8*/  { TB, TC, TA, X  },
    /*9*/  { TB, TF, X,  X  },
    /*10*/ { TB, TA, X,  X  },
    /*11*/ { TB, TF, X,  X  },
    /*12*/ { TB, TD, TA, X  },
    /*13*/ { TB, TC, X,  X  },
    /*14*/ { TB, TA, X,  X  },
    /*15*/ { TB, TE, X,  X  },
    /*16*/ { TB, TA, X,  X  },
    /*17*/ { TB, TF, X,  X  },
    /*18*/ { TB, TC, TA, X  },
    /*19*/ { TB, X,  X,  X  },
};

/******** 拨码控制 Task_C 耗时 ********/
#if USE_DIP_FOR_TASK_C
#ifndef SW0
#define SW0 SW_10
#define SW1 SW_11
#define SW2 SW_12
#define SW3 SW_13
#define SW4 SW_14
#define SW5 SW_15
#define SW6 SW_16
#define SW7 SW_17
#endif

// 读取 8 个拨码开关，生成 0–255 的值
static inline uint8_t read_switches8(void){
    uint8_t v = 0;
    v |= (BSP_GetInput(SW0) ? 1u : 0u) << 0;
    v |= (BSP_GetInput(SW1) ? 1u : 0u) << 1;
    v |= (BSP_GetInput(SW2) ? 1u : 0u) << 2;
    v |= (BSP_GetInput(SW3) ? 1u : 0u) << 3;
    v |= (BSP_GetInput(SW4) ? 1u : 0u) << 4;
    v |= (BSP_GetInput(SW5) ? 1u : 0u) << 5;
    v |= (BSP_GetInput(SW6) ? 1u : 0u) << 6;
    v |= (BSP_GetInput(SW7) ? 1u : 0u) << 7;
    return v;
}

// 根据拨码值控制 Task_C 额外等待时间（每档 200us）
static inline void job_C_switch(jobReturn_t* r, uint8_t sw){
    r->start = time_us_64();
    const uint32_t extra_cycles = (uint32_t)sw * (CYCLES_PER_US * 200u); 
    BSP_WaitClkCycles(EXECUTION_TIME_C + extra_cycles);
    r->stop  = time_us_64();
}
#endif

/******** 执行任务 ********/
static void run_task(task_id_t id, jobReturn_t* r){
#if USE_DIP_FOR_TASK_C
    if (id == TC) { job_C_switch(r, read_switches8()); return; }
#endif
    switch (id){
        case TA: job_A(r); break;
        case TB: job_B(r); break;
        case TC: job_C(r); break;
        case TD: job_D(r); break;
        case TE: job_E(r); break;
        case TF: job_F(r); break;
        default: r->start = r->stop = time_us_64(); break;
    }
}

/******** 记录结构 ********/
typedef struct {
    uint8_t  id;
    uint64_t start_abs;
    uint64_t end_abs;
    uint64_t deadline_abs;
    bool     miss;
} rec_t;

// 每帧最多记录 4 个任务
static rec_t frame_records[FRAMES_PER_HYPER][MAX_TASKS_PER_FRAME];
static int frame_task_count[FRAMES_PER_HYPER];
static bool frame_has_miss[FRAMES_PER_HYPER];

int main(void){
    BSP_Init();
    stdio_init_all();

    const uint64_t f_us = (uint64_t)F_MS * 1000ull;

    // 对齐到下一个 frame 边界
    uint64_t now0 = time_us_64();
    uint64_t t0   = ((now0 + f_us - 1) / f_us) * f_us;
    while (time_us_64() < t0) { tight_loop_contents(); }

    uint64_t T_us[TASK_CNT];
    for (int i=0;i<TASK_CNT;i++) T_us[i] = (uint64_t)TASK[i].period_ms * 1000ull;

    uint64_t frame_start = t0;
    uint32_t frame_idx   = 0;

    printf("EPOCH t0=%llu us, frame=%u ms\n", (unsigned long long)t0, F_MS);

    while (true){
        // 对齐帧起点
        uint64_t now = time_us_64();
        if (now < frame_start) sleep_us((uint32_t)(frame_start - now));

        int  rec_n    = 0;
        bool any_miss = false;

        // 帧内执行任务并记录
        for (int k=0; k<MAX_TASKS_PER_FRAME; ++k){
            uint8_t id = FRAME_PLAN[frame_idx][k];
            if (id == X) break;

            jobReturn_t r;
            run_task((task_id_t)id, &r);

            uint64_t phase    = (r.start >= t0) ? (r.start - t0) : 0;
            uint64_t k_rel    = phase / T_us[id];
            uint64_t rel_abs  = t0 + k_rel * T_us[id];
            uint64_t ddl_abs  = rel_abs + T_us[id];
            bool miss = (r.stop > ddl_abs);

            frame_records[frame_idx][rec_n++] = (rec_t){
                .id = id,
                .start_abs = r.start - t0,
                .end_abs   = r.stop  - t0,
                .deadline_abs = ddl_abs - t0,
                .miss = miss
            };
            any_miss |= miss;
        }

        frame_task_count[frame_idx] = rec_n;
        frame_has_miss[frame_idx]   = any_miss;

        // 下一帧
        BSP_ToggleLED(LED_GREEN);
        frame_idx = (frame_idx + 1) % FRAMES_PER_HYPER;
        frame_start += f_us;

        // 超周期结束后打印并重新对齐时间
        if (frame_idx == 0) {
            printf("\n================ HYPERPERIOD RESULT ================\n");
            for (int f = 0; f < FRAMES_PER_HYPER; f++) {
                printf("Frame %02d:\n", f);
                for (int i = 0; i < frame_task_count[f]; i++) {
                    rec_t *r = &frame_records[f][i];
                    uint64_t dur = r->end_abs - r->start_abs;
                    printf("   %s: start=%llu end=%llu dur=%llu%s\n",
                           TASK[r->id].name,
                           (unsigned long long)r->start_abs,
                           (unsigned long long)r->end_abs,
                           (unsigned long long)dur,
                           r->miss ? " [MISS]" : "");
                }
            }

            int total_miss = 0;
            for (int f = 0; f < FRAMES_PER_HYPER; f++)
                if (frame_has_miss[f]) total_miss++;
            printf("---------------------------------------------------\n");
            printf("Total missed frames: %d / %d\n", total_miss, FRAMES_PER_HYPER);
            printf("===================================================\n\n");

            // 时间重新对齐，防止漂移
            uint64_t now_align = time_us_64();
            t0 = ((now_align + f_us - 1) / f_us) * f_us;
            frame_start = t0;
        }
    }
}
