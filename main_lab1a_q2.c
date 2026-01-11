#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/timer.h"
#include "bsp.h"
#include "workload.h"

#define FRAME_COUNT   20
#define FRAME_TIME_MS 5
#define HYPER_PERIOD_MS (FRAME_COUNT * FRAME_TIME_MS)

typedef void (*TaskFunc)(jobReturn_t*);

typedef struct {
    TaskFunc tasks[4];
    uint8_t  task_count;
} Frame;

// ------------------------------
// 全局变量
// ------------------------------
Frame schedule_table[FRAME_COUNT];
uint64_t frame_duration_us[FRAME_COUNT]; // 存储每帧耗时
bool frame_miss[FRAME_COUNT];             // 存储每帧是否超时

int current_frame = 0;
int cycle_id = 0;
repeating_timer_t timer;

// ------------------------------
// 构建调度表
// ------------------------------
void build_schedule(void) {
    for (int i = 0; i < FRAME_COUNT; i++)
        schedule_table[i].task_count = 0;

    schedule_table[0]  = (Frame){{job_B, job_C, job_F}, 3};
    schedule_table[1]  = (Frame){{job_B, job_A}, 2};
    schedule_table[2]  = (Frame){{job_B, job_D}, 2};
    schedule_table[3]  = (Frame){{job_B, job_A}, 2};
    schedule_table[4]  = (Frame){{job_B, job_F}, 2};
    schedule_table[5]  = (Frame){{job_B, job_A, job_C}, 3};
    schedule_table[6]  = (Frame){{job_B, job_E}, 2};
    schedule_table[7]  = (Frame){{job_B, job_A}, 2};
    schedule_table[8]  = (Frame){{job_B, job_F}, 2};
    schedule_table[9]  = (Frame){{job_B, job_A}, 2};
    schedule_table[10] = (Frame){{job_B, job_C}, 2};
    schedule_table[11] = (Frame){{job_B, job_A}, 2};
    schedule_table[12] = (Frame){{job_B, job_D, job_F}, 3};
    schedule_table[13] = (Frame){{job_B, job_A}, 2};
    schedule_table[14] = (Frame){{job_B, job_E}, 2};
    schedule_table[15] = (Frame){{job_B, job_A, job_C}, 3};
    schedule_table[16] = (Frame){{job_B, job_F}, 2};
    schedule_table[17] = (Frame){{job_B, job_A}, 2};
    schedule_table[18] = (Frame){{job_B}, 1};
    schedule_table[19] = (Frame){{job_B, job_A}, 2};
}

// ------------------------------
// 每帧回调：执行任务并记录结果
// ------------------------------
bool frame_callback(repeating_timer_t *tmr) {
    jobReturn_t retval;
    uint64_t frame_start = time_us_64();

    // 执行本帧任务集
    for (int i = 0; i < schedule_table[current_frame].task_count; i++) {
        schedule_table[current_frame].tasks[i](&retval);
    }

    // 计算帧耗时并记录
    uint64_t frame_stop = time_us_64();
    uint64_t frame_duration = frame_stop - frame_start;
    frame_duration_us[current_frame] = frame_duration;
    frame_miss[current_frame] = (frame_duration > FRAME_TIME_MS * 1000);

    current_frame++;

    // 如果完成一个超周期（20帧）
    if (current_frame >= FRAME_COUNT) {
        cancel_repeating_timer(tmr);
        return false;
    }
    return true;
}

// ------------------------------
// 打印周期结果（逐帧显示）
// ------------------------------
void print_cycle_results(void) {
    printf("\n================ CYCLE %d RESULT ================\n", cycle_id++);

    int missed = 0;
    printf("Frame |  Duration(ms)  |  Status\n");
    printf("------+----------------+----------------\n");

    for (int i = 0; i < FRAME_COUNT; i++) {
        double duration_ms = frame_duration_us[i] / 1000.0;
        if (frame_miss[i]) {
            printf("%5d |  %10.2f     |  Missed Deadline\n", i, duration_ms);
            missed++;
        } else {
            printf("%5d |  %10.2f     |  On Time\n", i, duration_ms);
        }
    }

    printf("----------------------------------\n");
    if (missed > 0)
        printf(" %d frame(s) missed the 5ms deadline.\n", missed);
    else
        printf("All frames finished within 5ms.\n");
    printf("=================================================\n\n");

    fflush(stdout);
}

// ------------------------------
// 主程序：循环运行多个超周期
// ------------------------------
int main() {
    stdio_init_all();
    BSP_Init();
    build_schedule();

    printf("Starting cyclic scheduler...\n");

    while (true) {
        current_frame = 0;

        // 启动一个超周期
        add_repeating_timer_ms(FRAME_TIME_MS, frame_callback, NULL, &timer);

        // 等待整个超周期执行完
        sleep_ms(HYPER_PERIOD_MS + 10);

        // 打印该周期结果（每帧一行）
        print_cycle_results();

        // 可选择延时再进入下一周期
        sleep_ms(500);
    }
}
