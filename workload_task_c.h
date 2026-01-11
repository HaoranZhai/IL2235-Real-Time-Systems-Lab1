#ifndef WORKLOAD_H
#define WORKLOAD_H

#include <stdint.h>

/* ===================== 基本时间参数 ===================== */
#define CYCLES_PER_MS  150000
#define CYCLES_PER_US  (CYCLES_PER_MS / 1000)

/* ===================== 各任务固定执行时间 ===================== */
#define EXECUTION_TIME_A ((1 * CYCLES_PER_MS) - (CYCLES_PER_US * 10))
#define EXECUTION_TIME_B ((1 * CYCLES_PER_MS) - (CYCLES_PER_US * 10))
#define EXECUTION_TIME_C ((2 * CYCLES_PER_MS) - (CYCLES_PER_US * 10))
#define EXECUTION_TIME_D ((2 * CYCLES_PER_MS) - (CYCLES_PER_US * 10))
#define EXECUTION_TIME_E ((4 * CYCLES_PER_MS) - (CYCLES_PER_US * 10))
#define EXECUTION_TIME_F ((2 * CYCLES_PER_MS) - (CYCLES_PER_US * 10))

/* ===================== 拨码开关控制参数 ===================== */
/**
 * 每一个开关代表的额外延迟比例。
 * 例如 SWITCH_DELAY_FACTOR = 0.1 表示每个开关增加 0.1ms 延时。
 * 在 job_C() 中会用到：
 *    extra_delay = switch_value * (CYCLES_PER_MS * SWITCH_DELAY_FACTOR)
 */
#define SWITCH_DELAY_FACTOR 0.1f

/* ===================== 任务返回结构体定义 ===================== */
typedef struct {
    uint64_t start;
    uint64_t stop;
} jobReturn_t;

/* ===================== 函数原型声明 ===================== */
void job_A(jobReturn_t* retval);
void job_B(jobReturn_t* retval);
void job_C(jobReturn_t* retval);
void job_D(jobReturn_t* retval);
void job_E(jobReturn_t* retval);
void job_F(jobReturn_t* retval);

#endif /* WORKLOAD_H */
